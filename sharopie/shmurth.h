#pragma once

#include "shmach.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  SHMURTH_MAX_GLOBALS = 16,
  SHMURTH_MAX_TRACKS = 8,
  SHMURTH_MAX_INSTRUMENTS = 8,
  SHMURTH_MAX_ACTIVE_NOTES = 16,
  SHMURTH_MAX_SEQUENCERS = 8,
  SHMURTH_MAX_INSTRUCTIONS = 128
};

struct shmurth_t_;

typedef void (*shmurth_event_emit_f)(struct shmurth_t_ *,
  void *param, uint32_t event_id, uint32_t count, shmach_value_t *values);

struct shmurth_t_ {
  shmach_object_t o;
  uint32_t samplerate;
  shmurth_event_emit_f event_emit;
  void *event_emit_param;
  shmach_value_t globals[SHMURTH_MAX_GLOBALS];
  float tracks[SHMURTH_MAX_TRACKS * 2];
  shmach_core_t note_core;
  shmach_core_t ctl_core;
  shmach_core_t mixer_core;
  shmach_core_t seqencer_cores[SHMURTH_MAX_SEQUENCERS];
  shmach_section_t instruments[SHMURTH_MAX_INSTRUMENTS];
  struct inststance_t_ {
    uint32_t note;
    uint32_t track;
    shmach_core_t core;
  } inststances[SHMURTH_MAX_ACTIVE_NOTES];
  uint32_t frames_to_next_tick;
};

typedef struct shmurth_t_ shmurth_t;

void shmurth_init(shmurth_t *shmurth,
  uint32_t samplerate,
  shmurth_event_emit_f event_emit, void *event_emit_param);
void shmurth_set_note_section(shmurth_t *shmurth, shmach_section_t section);
void shmurth_set_ctl_section(shmurth_t *shmurth, shmach_section_t section);
void shmurth_set_mixer_section(shmurth_t *shmurth, shmach_section_t section);
void shmurth_set_instrument_section(shmurth_t *shmurth,
  uint32_t instrument, shmach_section_t section);
void shmurth_set_sequencer_section(shmurth_t *shmurth,
  uint32_t sequencer, shmach_section_t section);
void shmurth_silence(shmurth_t *shmurth);
void shmurth_note(shmurth_t *shmurth,
  uint32_t inst_id, uint32_t note, float velocity);
void shmurth_ctl(shmurth_t *shmurth,
  uint32_t ctl, shmach_value_t value);
void shmurth_synth(shmurth_t *shmurth, float *output_ilv, uint32_t samples);

#ifdef __cplusplus
}
#endif
