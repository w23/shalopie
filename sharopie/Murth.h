#pragma once
#include "shmurth.h"

class Queue {
public:
  Queue();
  ~Queue();

  struct carriage_t {
    uint32_t type() const { return type_; }
    uint32_t param() const { return param_; }
    const uint8_t *data() const { return buffer_; }
    uint32_t size() const { return buffer_size_; }
    
  private:
    friend class Queue;
    
    carriage_t(uint32_t type, uint32_t param, const void *data, uint32_t size);
    ~carriage_t();
    
    uint32_t type_, param_;
    uint8_t *buffer_;
    uint32_t buffer_size_;
    carriage_t *next_;
  };

  carriage_t *carriage_get();
  void carriage_return(carriage_t *);
  
  void send(uint32_t type, uint32_t param, const void *data, uint32_t size);
  
private:
  carriage_t *inbound_;
  carriage_t *outbound_;
};

class Murth {
public:
  Murth();
  ~Murth();
  
  void init(int samplerate);
  
  void queue_mixer_reprogram(shmach_op_t *section, uint32_t size);
  void queue_note_reprogram(shmach_op_t *section, uint32_t size);
  void queue_ctl_reprogram(shmach_op_t *section, uint32_t size);
  void queue_instrument_reprogram(uint32_t index, shmach_op_t *section, uint32_t size);
  void queue_sequencer_reprogram(uint32_t index, shmach_op_t *section, uint32_t size);
  
  void process_raw_midi(const void *data, uint32_t size);
  void synthesize(float *lr_interleave, uint32_t frames);
  
  bool get_event(uint32_t *type, shmach_value_t value);
  
private:
  shmurth_t sh_;
  
  Queue in_queue_;
  
  Queue::carriage_t *mixer_program_;
  Queue::carriage_t *note_program_;
  Queue::carriage_t *ctl_program_;
  Queue::carriage_t *instrument_programs_[SHMURTH_MAX_INSTRUMENTS];
  Queue::carriage_t *sequencer_programs_[SHMURTH_MAX_SEQUENCERS];
  
  void emit_event(uint32_t event_id, uint32_t count, shmach_value_t *values);
  static void murth_emit_event(struct shmurth_t_ *,
    void *param, uint32_t event_id, uint32_t count, shmach_value_t *values);
}; // class Murth