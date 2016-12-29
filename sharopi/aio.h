#ifndef AIO_H__INCLUDED
#define AIO_H__INCLUDED

struct AIOChannelAudio {
	const char *name;
	void *impl_opaque;
	float *samples;
};

struct AIOChannelMidi {
	const char *name;
	void *impl_opaque;
};

struct AIOMidiEvent {
	int channel_index;
	int sample;
	int size;
	const unsigned char* bytes;
};

struct AudioCore;

struct AIOState {
	int samplerate; /* desired set by caller; factual set by implementation */
	int frame_size; /* desired set by caller; factual set by implementation */

	int input_channels_count;
	struct AIOChannelAudio *input_channels/*[input_channels_count]*/;

	int output_channels_count;
	struct AIOChannelAudio *output_channels/*[output_channels_count]*/;

	int midiin_channels_count;
	struct AIOChannelMidi *midiin_channels/*[input_channels_count]*/;

	int midiout_channels_count;
	struct AIOChannelMidi *midiout_channels/*[out_channels_count]*/;

	/* asynchronously called from realtime thread(s) by implementation */
	void (*processAudio)(struct AIOState *state);
	void (*processException)(struct AIOState *state);

	void (*processMidiEvent)(struct AIOState *state, struct AIOMidiEvent *event);

	struct AudioCore *core;
};

#endif /*ifndef AIO_H__INCLUDED*/
