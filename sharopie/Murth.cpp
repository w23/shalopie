#include <libkern/OSAtomic.h>
#include <cstring>
#include <cassert>
#include "Murth.h"

////////////////////////////////////////////////////////////////////////////////

Queue::carriage_t::carriage_t(uint32_t type, uint32_t param, const void *data, uint32_t size)
  : type_(type)
  , param_(param)
  , buffer_(size > 0 ? new uint8_t[size] : nullptr)
  , buffer_size_(size)
{
  memcpy(buffer_, data, size);
}

Queue::carriage_t::~carriage_t() {
  delete [] buffer_;
}

Queue::Queue() : inbound_(nullptr), outbound_(nullptr) {
}

Queue::~Queue() {
  carriage_t *c = outbound_;
  while (c) { carriage_t *tmp = c; c = c->next_; delete tmp; }
  c = inbound_;
  while (c) { carriage_t *tmp = c; c = c->next_; delete tmp; }
}

Queue::carriage_t *Queue::carriage_get() {
  carriage_t *c;
  for (;;) {
    c = inbound_;
    if (c == nullptr)
      return nullptr;
    if (OSAtomicCompareAndSwapPtrBarrier(c, c->next_, reinterpret_cast<void *volatile*>(&inbound_)))
      return c;
  }
}

void Queue::carriage_return(carriage_t *carriage) {
  delete carriage;
}
  
void Queue::send(uint32_t type, uint32_t param, const void *data, uint32_t size) {
  carriage_t *c = new carriage_t(type, param, data, size);
  for (;;) {
    c->next_ = inbound_;
    if (OSAtomicCompareAndSwapPtrBarrier(c->next_, c, reinterpret_cast<void *volatile*>(&inbound_)))
      return;
  }
}

enum {
  CarriageTypeProgMixer,
  CarriageTypeProgNote,
  CarriageTypeProgCtl,
  CarriageTypeProgInstr,
  CarriageTypeProgSeq,
  CarriageTypeNote,
  CarriageTypeCtl
};

Murth::Murth()
  : mixer_program_(nullptr)
  , note_program_(nullptr)
  , ctl_program_(nullptr)
{
  memset(instrument_programs_, 0, sizeof(instrument_programs_));
  memset(sequencer_programs_, 0, sizeof(sequencer_programs_));
}

Murth::~Murth() {
 // \todo
}

void Murth::init(int samplerate) {
  shmurth_init(&sh_, samplerate, murth_emit_event, this);
}

void Murth::queue_mixer_reprogram(shmach_op_t *section, uint32_t size) {
  in_queue_.send(CarriageTypeProgMixer, 0, section, size);
}

void Murth::queue_note_reprogram(shmach_op_t *section, uint32_t size) {
  in_queue_.send(CarriageTypeProgNote, 0, section, size);
}

void Murth::queue_ctl_reprogram(shmach_op_t *section, uint32_t size) {
  in_queue_.send(CarriageTypeProgCtl, 0, section, size);
}

void Murth::queue_instrument_reprogram(uint32_t index, shmach_op_t *section, uint32_t size)
{
  assert(index < SHMURTH_MAX_INSTRUMENTS);
  in_queue_.send(CarriageTypeProgInstr, index, section, size);
}

void Murth::queue_sequencer_reprogram(uint32_t index, shmach_op_t *section, uint32_t size)
{
  assert(index < SHMURTH_MAX_SEQUENCERS);
  in_queue_.send(CarriageTypeProgSeq, index, section, size);
}

void Murth::process_raw_midi(const void *data, uint32_t size) {
  const uint8_t *p = static_cast<const uint8_t*>(data);
  for (;size > 2;) {
    switch (p[0]&0xf0) {
      case 0x80: // note off
        in_queue_.send(CarriageTypeNote, p[1], nullptr, 0);
        p += 3;
        size -= 3;
        break;

      case 0x90: // note on
        in_queue_.send(CarriageTypeNote,
          ((p[0] & 0x0f) << 16) | (p[2] << 8) | p[1], nullptr, 0);
        p += 3;
        size -= 3;
        break;
        
      case 0xb0: // ctl
        in_queue_.send(CarriageTypeCtl,
          ((p[0] & 0x0f) << 16) | (p[1] << 8) | p[2], nullptr, 0);
        p += 3;
        size -= 3;
      break;

      default:
        size = 0; //abort
    }
  }
}

void Murth::synthesize(float *lr_interleave, uint32_t frames) {
  for (;;) {
    Queue::carriage_t *c = in_queue_.carriage_get();
    if (c == nullptr)
      break;
    
    switch (c->type()) {
    case CarriageTypeProgMixer:
      in_queue_.carriage_return(mixer_program_);
      mixer_program_ = c;
      shmurth_set_mixer_section(&sh_, static_cast<const shmach_section_t>(c->data()));
      break;
    case CarriageTypeProgNote:
    in_queue_.carriage_return(note_program_);
      note_program_ = c;
      shmurth_set_note_section(&sh_, static_cast<const shmach_section_t>(c->data()));
      break;
    case CarriageTypeProgCtl:
      in_queue_.carriage_return(ctl_program_);
      ctl_program_ = c;
      shmurth_set_ctl_section(&sh_, static_cast<const shmach_section_t>(c->data()));
      break;
    case CarriageTypeProgInstr:
      in_queue_.carriage_return(instrument_programs_[c->param()]);
      instrument_programs_[c->param()] = c;
      shmurth_set_instrument_section(&sh_, c->param(), static_cast<const shmach_section_t>(c->data()));
      break;
    case CarriageTypeProgSeq:
      in_queue_.carriage_return(sequencer_programs_[c->param()]);
      sequencer_programs_[c->param()] = c;
      shmurth_set_sequencer_section(&sh_, c->param(), static_cast<const shmach_section_t>(c->data()));
      break;
    case CarriageTypeNote:
      shmurth_note(&sh_, c->param() >> 16, c->param() & 0xff,
        (0xff & (c->param() >> 8)) / 127.f);
      in_queue_.carriage_return(c);
      break;
    case CarriageTypeCtl: {
        shmach_value_t v;
        v.v.f = (c->param() & 0xff) / 127.f;
        shmurth_ctl(&sh_, c->param() >> 8, v);
      }
      in_queue_.carriage_return(c);
      break;
    }
  }
  
  shmurth_synth(&sh_, lr_interleave, frames);
}

bool Murth::get_event(uint32_t *type, shmach_value_t *value) {
// WOW VERY ASYNC MANY NONBLOCKING MUCH BROKEN
  while (eread_ != ewrite_) {
    uint32_t i = (eread_++)%16;
    *type = events_[i].event_id;
    *value = events_[i].value;
    return true;
  }
  return false;
}

void Murth::emit_event(uint32_t event_id, uint32_t count, shmach_value_t *values)
{
// WOW VERY ASYNC MANY NONBLOCKING MUCH BROKEN
  uint32_t i = (ewrite_++)%16;
  events_[i].event_id = event_id;
  if (count > 0)
    events_[i].value = values[0];
  else
    events_[i].value.v.i = 0;
}

void Murth::murth_emit_event(struct shmurth_t_ *,
  void *param, uint32_t event_id, uint32_t count, shmach_value_t *values)
{
  Murth *murth = static_cast<Murth*>(param);
  murth->emit_event(event_id, count, values);
}