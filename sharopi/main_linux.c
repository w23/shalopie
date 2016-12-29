#include "aio_jack.h"
#include "audio_core.h"
#include <string.h> /* memset */
#include <stdlib.h> /* exit */
#include <stdio.h> /* vsprintf */
#include <stdarg.h> /* va_* */
#include <unistd.h> /* sleep */

static void processAudio(struct AIOState *state) {
	aucProcessAudio(state->core);
}

static void processException(struct AIOState *state) {
	(void)state;
	exit(-1);
}

static void processMidiEvent(struct AIOState *state, struct AIOMidiEvent *event) {
	aucProcessMidiEvent(state->core, event);
}

void shlpMessage(SHLPMessageClass msg_class, const char *message, ...) {
	va_list ap;
	const char *classname = "UNKNOWN";
	switch(msg_class) {
		case SHLPMessage_Debug: classname = "Debug"; break;
		case SHLPMessage_Info: classname = "Info"; break;
		case SHLPMessage_Warning: classname = "Warning"; break;
		case SHLPMessage_Error: classname = "Error"; break;
		case SHLPMessage_Fatal: classname = "Fatal"; break;
	}

	va_start(ap, message);
	vfprintf(stderr, message, ap);
	va_end(ap);

	printf("\n");
}

int main(int argc, char *argv[]) {
	const char *jack_name = (argc > 1) ? argv[1] : argv[0];
	const char *server_name = (argc > 2) ? argv[2] : NULL;
	struct AIOChannelAudio output[2];
	struct AIOChannelMidi midiin[1];
	SHLPError sherr;

	struct AudioCore core;
	struct AIOState aio;
	memset(&aio, 0, sizeof aio);

	aio.core = &core;

	aio.processAudio = processAudio;
	aio.processException = processException;
	aio.processMidiEvent = processMidiEvent;

	aio.output_channels_count = COUNTOF(output);
	aio.output_channels = output;
	aio.midiin_channels_count = COUNTOF(midiin);
	aio.midiin_channels = midiin;

	output[0].name = "left";
	output[1].name = "lefter";
	midiin[0].name = "MIDI";

	sherr = aioJackOpen(jack_name, server_name, &aio);
	if (sherr != SHLPError_Ok) {
		shlpMessage(SHLPMessage_Error, "Cannot open Jack client");
		exit(1);
	}

	aucInit(&core, &aio);

	sherr = aioJackActivate();
	if (sherr != SHLPError_Ok) {
		shlpMessage(SHLPMessage_Error, "Cannot activate Jack client");
		goto cleanup;
	}

	for (;;) sleep(1);

	aioJackDeactivate();

cleanup:
	aioJackClose();
	return 0;
}
