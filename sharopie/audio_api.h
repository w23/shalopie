#pragma once

struct AudioIOFrame {
	void *user_data;

	float **input;
	float **output;
}; /* struct AudioIOFrame */

typedef void (*audioIoFrameCallback)(struct AudioIOFrame *frame);

struct AudioIOHandle {
	int samplerate;
	int input_channels;
	int output_channels;
	int frame_samples;
}; /* AudioIOHandle */

struct AudioIOOpenSettings {
	const char *name;

	int input_channels;
	int output_channels;
	
	void *user_data;
	audioIoFrameCallback callback;
}; /* struct AudioIOOpenSettings */

/*
struct AudioIOMidiData {
	size_t size;
	const u8 *bytes;
};

typedef void (*audioIoMidiCallback)(
	struct AudioIOMidiData *in,
	struct AudioIOMidiData *out);
*/

struct AudioIOSystem {
	const char *name;

	struct AudioIOHandle *(*open)(struct AudioIOOpenSettings settings);

	void (*start)(struct AudioIOHandle *handle);
	void (*stop)(struct AudioIOHandle *handle);

	void (*close)(struct AudioIOHandle *handle);
}; /* struct AudioIOSystem */