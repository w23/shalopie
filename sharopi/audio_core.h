#ifndef AUDIO_CORE_H__INCLUDED
#define AUDIO_CORE_H__INCLUDED

#include "lfqueue.h"

struct Voice {
	int active;
	int channel, note;
	float phase;
	float dphase;
	float env;
};

#define MIDI_QUEUE_SIZE 256

struct AudioCore {
	struct AIOState* aio;

	struct LFQueue midi_queue;
	unsigned char midi_queue_buffer[MIDI_QUEUE_SIZE];

	struct Voice voices[16];
};

void aucInit(struct AudioCore *core, struct AIOState *aio);

void aucProcessAudio(struct AudioCore *core);

struct AIOMidiEvent;
void aucProcessMidiEvent(struct AudioCore *core, struct AIOMidiEvent *event);


#endif /*ifndef AUDIO_CORE_H__INCLUDED*/
