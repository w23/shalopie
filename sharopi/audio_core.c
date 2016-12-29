#include "aio.h"
#include "shlp.h"
#include <math.h> /* pow, sin, ... */
#include <string.h>
#define LFQ_IMPLEMENT
#include "lfqueue.h"
#include "audio_core.h"

#include <stdio.h>

const float c_pi = 3.1415926f, c_2pi = c_pi * 2.f;

void aucVoiceOn(struct Voice *voice, int samplerate, int note, int velocity) {
	voice->active = 1;
	voice->phase = 0;
	voice->env = velocity / 127.f;
	voice->dphase = 440.f * pow(2., (note-48) / 12.) / samplerate;
}

void aucVoiceProcess(struct Voice* voice, int samples, float *output, float mix) {
	int i;
	for (i = 0; i < samples; ++i) {
		voice->phase = fmodf(voice->phase + voice->dphase, 1.0f);
		/*voice->phase = voice->phase + voice->dphase;*/
		output[i] += mix * voice->env * (2.f * fabs(voice->phase * 2.f - 1.f) - 1.f); /* sin(voice->phase * c_2pi)*/;
	}
}

void aucInit(struct AudioCore *core, struct AIOState *aio) {
	memset(core, 0, sizeof *core);
	core->aio = aio;
	lfqInit(&core->midi_queue, sizeof(core->midi_queue_buffer), &core->midi_queue_buffer);
}

void aucProcessAudio(struct AudioCore *core) {
	struct AIOState* aio = core->aio;

	unsigned long i = 0;

	for (;;) {
		int i;
		unsigned char event_size;
		unsigned char event_buffer[255];

		if (sizeof event_size != lfqRead(&core->midi_queue, sizeof event_size, &event_size))
			break;

		lfqRead(&core->midi_queue, event_size, event_buffer);

		/* TODO what if the buffer is truncated? */

		/* TODO parse buffer */

		/* for (i = 0; i < event_size; ++i) printf("%02x%s", event_buffer[i], i == event_size - 1 ? "\n" : " "); */

		for (i = 0; i + 2 < event_size;) {
			int on = 0;
			const unsigned char status = event_buffer[i] & 0xf0;

			switch(status) {
				case 0x90: /* note on */
					on = 1;
				case 0x80: /* note off */
				{
					struct Voice *empty = NULL;
					unsigned long v;
					const unsigned char channel = event_buffer[i] & 0x0f;
					const unsigned char key = event_buffer[i+1] & 0x7f;
					const unsigned char vel = event_buffer[i+2] & 0x7f;

					for (v = 0; v < COUNTOF(core->voices); ++v) {
						struct Voice *vc = core->voices + v;
						if (vc->active && vc->channel == channel && vc->note == key) {
							if (on) {
								empty = NULL;
								break;
							} else {
								printf("voice %lu c%d n%d off\n", v, channel, key);
								vc->active = 0;
							}
						} else if (!empty && !vc->active) empty = vc;
					}
					if (on && empty) {
						printf("voice %lu c%d n%d on\n", empty - core->voices, channel, key);
						empty->channel = channel;
						empty->note = key;
						aucVoiceOn(empty, aio->samplerate, key, vel);
					}

					i += 3;
					break;
				}
			}
		}
	}

	if (!aio->output_channels_count)
		return;

	for (i = 0; i < COUNTOF(core->voices); ++i) {
		struct Voice *voice = core->voices + i;
		if (voice->active) {
			aucVoiceProcess(voice, aio->frame_size, aio->output_channels[0].samples, 1.0f);
/*			printf("%f\t", aio->output_channels[0].samples[0]);*/
		}
	}

	if (aio->output_channels_count > 1) {
		int i;
		for (i = 1; i < aio->output_channels_count; ++i) {
			memcpy(aio->output_channels[i].samples, aio->output_channels[0].samples,
				sizeof(float) * aio->frame_size);
		}
	}
}

void aucProcessMidiEvent(struct AudioCore *core, struct AIOMidiEvent *event) {
	if (event->size > 255) {
			/* TODO log error that we've dropped midi event */
			return;
	}

	{
		unsigned char event_buffer[256];
		event_buffer[0] = event->size;
		memcpy(event_buffer + 1, event->bytes, event->size);
		lfqWrite(&core->midi_queue, 1 + event->size, event_buffer);
		/* TODO check error and log it */
	}
}
