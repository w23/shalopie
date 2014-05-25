#include <string.h>
#include <assert.h>
#include <math.h>

#include "shmurth.h"

enum {
  SHMURTH_FUNC_GET_GLOBAL = 0,
  SHMURTH_FUNC_SET_GLOBAL = 1,
  SHMURTH_FUNC_READ_TRACKS = 2,
  SHMURTH_FUNC_SPAWN_INSTRUMENT = 3,
  SHMURTH_FUNC_EMIT_EVENT = 4
};

static void shmurth_func(struct shmach_object_t_ *obj,
  struct shmach_core_t_ *core, uint32_t index)
{
  shmurth_t *this = (shmurth_t*)obj;
  switch (index) {
  case SHMURTH_FUNC_GET_GLOBAL:
    assert(core->sp[0].v.i < SHMURTH_MAX_GLOBALS);
    core->sp[0] = this->globals[core->sp[0].v.i];
    break;
  case SHMURTH_FUNC_SET_GLOBAL:
    this->globals[core->sp[0].v.i] = core->sp[1];
    core->sp += 2;
    break;
  case SHMURTH_FUNC_READ_TRACKS: {
      uint32_t index = core->sp->v.i;
      assert(index < SHMURTH_MAX_TRACKS);
      --core->sp;
      core->sp[0].v.f = this->tracks[index * 2 + 0];
      core->sp[1].v.f = this->tracks[index * 2 + 1];
    }
    break;
  case SHMURTH_FUNC_SPAWN_INSTRUMENT: {
      const shmach_op_t *text = this->instruments[core->sp[0].v.i];
      uint32_t track = core->sp[1].v.i;
      uint32_t note = core->sp[2].v.i;
      float velocity = core->sp[3].v.f;
      struct inststance_t_ *ifree = NULL;
    
      for (uint32_t i = 0; i < SHMURTH_MAX_ACTIVE_NOTES; ++i) {
        struct inststance_t_ *inst = this->inststances + i;
        if (inst->core.text == NULL) {
          if (ifree == NULL)
            ifree = inst;
        } else if (text == inst->core.text &&
          inst->note == note)
        {
          inst->core.stack[SHMACH_MAX_STACK - 2].v.f = velocity;
          ifree = NULL;
          break;
        }
      }
    
      if (ifree != NULL) {
        ifree->core.text = text;
        ifree->note = note;
        ifree->track = track;
        shmach_value_t *v = shmach_core_reset(&ifree->core, 4);
        
        v[0].v.f = powf(1.059463, (float)note) * 55.f / (float)this->samplerate;
        v[1].v.i = note;
        v[2].v.f = velocity;
        v[3].v.o = obj;
      }
      core->sp += 4;
    }
    break;
  case SHMURTH_FUNC_EMIT_EVENT:
    if (this->event_emit != NULL)
      this->event_emit(this, this->event_emit_param,
        core->sp[0].v.i, core->sp[1].v.i, core->sp + 2);
    core->sp += 2 + core->sp[1].v.i;
    break;
  }
}

void shmurth_init(shmurth_t *this, uint32_t samplerate,
  shmurth_event_emit_f event_emit, void *event_emit_param)
{
  memset(this, 0, sizeof(*this));
  this->samplerate = samplerate;
  this->event_emit = event_emit;
  this->event_emit_param = event_emit_param;
  
  this->o.dtor = NULL;
  this->o.func = shmurth_func;
}

void shmurth_set_note_section(shmurth_t *this, shmach_section_t section) {
  this->note_core.text = section;
  // midi is always non-yield, so no reset needed
}

void shmurth_set_ctl_section(shmurth_t *this, shmach_section_t section) {
  this->ctl_core.text = section;
  // midi is always non-yield, so no reset needed
}

void shmurth_set_mixer_section(shmurth_t *this, shmach_section_t section) {
  this->mixer_core.text = section;
  // mixer can yield, so reset is necessary
  shmach_core_reset(&this->mixer_core, 1)[0].v.o = &this->o;
}

void shmurth_set_instrument_section(shmurth_t *this,
  uint32_t instrument, shmach_section_t section)
{
  assert(instrument < SHMURTH_MAX_INSTRUMENTS);
  const shmach_op_t *oldtext = this->instruments[instrument];
  // stop all notes with old section
  for (uint32_t i = 0; i < SHMURTH_MAX_ACTIVE_NOTES; ++i)
    if (this->inststances[i].core.text == oldtext)
      this->inststances[i].core.text = NULL;
  this->instruments[instrument] = section;
}

void shmurth_set_sequencer_section(shmurth_t *this,
  uint32_t sequencer, shmach_section_t section)
{
  assert(sequencer < SHMURTH_MAX_SEQUENCERS);
  this->seqencer_cores[sequencer].text = section;
  shmach_value_t *v = shmach_core_reset(this->seqencer_cores + sequencer, 1);
  v[0].v.o = &this->o;
  
  // \todo synchronize properly
  for (uint32_t i = 0; i < SHMURTH_MAX_SEQUENCERS; ++i) {
    shmach_value_t *v = shmach_core_reset(this->seqencer_cores + i, 1);
    v[0].v.o = &this->o;
  }
  this->frames_to_next_tick = 0;
}

void shmurth_silence(shmurth_t *this) {
  shmach_core_reset(&this->mixer_core, 0);
  for (uint32_t i = 0; i < SHMURTH_MAX_ACTIVE_NOTES; ++i) {
    this->inststances[i].core.text = NULL;
  }
}

void shmurth_note(shmurth_t *this,
  uint32_t inst_id, uint32_t note, float velocity)
{
  shmach_value_t *v = shmach_core_reset(&this->note_core, 4);
  v[0].v.i = inst_id;
  v[1].v.i = note;
  v[2].v.f = velocity;
  v[3].v.o = &this->o;
  shmach_core_return_t r
    = shmach_core_run(&this->note_core, SHMURTH_MAX_INSTRUCTIONS);
  assert(r.result == shmach_core_return_result_return);
}

void shmurth_ctl(shmurth_t *this, uint32_t ctl, shmach_value_t value)
{
  shmach_value_t *v = shmach_core_reset(&this->ctl_core, 3);
  v[0].v.i = ctl;
  v[1] = value;
  v[2].v.o = &this->o;
  shmach_core_return_t r
    = shmach_core_run(&this->ctl_core, SHMURTH_MAX_INSTRUCTIONS);
  assert(r.result == shmach_core_return_result_return);
}

void shmurth_synth(shmurth_t *this, float *output_ilv, uint32_t samples) {
  while (samples-- != 0) {
    if (this->frames_to_next_tick-- == 0) {
      for (uint32_t j = 0; j < SHMURTH_MAX_SEQUENCERS; ++j)
        if (this->seqencer_cores[j].text != NULL) {
          shmach_core_return_t r = shmach_core_run(
            this->seqencer_cores + j, SHMURTH_MAX_INSTRUCTIONS);
          assert(r.result != shmach_core_return_result_hang);
          if (r.result == shmach_core_return_result_return)
            this->seqencer_cores[j].text = NULL; // stop
          assert(r.count == 0);
        }
      this->frames_to_next_tick = 4096; // \todo controllable
    }

    memset(this->tracks, 0, sizeof(this->tracks));
    for (uint32_t i = 0; i < SHMURTH_MAX_ACTIVE_NOTES; ++i) {
      struct inststance_t_ *inst = this->inststances + i;
      if (inst->core.text != NULL) {
        shmach_core_return_t r = shmach_core_run(&(inst->core), SHMURTH_MAX_INSTRUCTIONS);
        if (r.result == shmach_core_return_result_return) {
          inst->core.text = NULL;
          continue;
        }
        assert(r.result != shmach_core_return_result_hang);
        assert(r.count > 0);
        assert(r.count < 3);
        if (r.count == 1) {
          this->tracks[inst->track * 2] += r.values[0].v.f;
          this->tracks[inst->track * 2 + 1] += r.values[0].v.f;
        } else {
          this->tracks[inst->track * 2] += r.values[0].v.f;
          this->tracks[inst->track * 2 + 1] += r.values[1].v.f;
        }
      }
    }

    shmach_core_return_t r = shmach_core_run(&this->mixer_core, SHMURTH_MAX_INSTRUCTIONS);
    assert(r.result != shmach_core_return_result_hang);
    assert(r.count == 2);
    output_ilv[0] = r.values[0].v.f;
    output_ilv[1] = r.values[1].v.f;
    output_ilv += 2;
  }
}
