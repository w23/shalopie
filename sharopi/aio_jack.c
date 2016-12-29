#include "aio_jack.h"
#include <jack/jack.h>
#include <jack/midiport.h>
#include <string.h> /* memset */

static struct AIOJackState {
	struct AIOState *io_state;
	jack_client_t *client;
} state = {
	/*.io_state = */NULL,
	/*.client = */NULL
};

static int aioJackCbProcess(jack_nframes_t nframes, void *arg) {
	struct AIOJackState *state = (struct AIOJackState*)arg;
	struct AIOState *aio = state->io_state;
	int i;

	aio->frame_size = nframes;

	for (i = 0; i < aio->input_channels_count; ++i) {
		jack_port_t *port = (jack_port_t*)aio->input_channels[i].impl_opaque;
		aio->input_channels[i].samples = jack_port_get_buffer(port, nframes);
	}

	for (i = 0; i < aio->output_channels_count; ++i) {
		jack_port_t *port = (jack_port_t*)aio->output_channels[i].impl_opaque;
		aio->output_channels[i].samples = jack_port_get_buffer(port, nframes);
		memset(aio->output_channels[i].samples, 0, nframes * sizeof(float));
	}

	for (i = 0; i < aio->midiin_channels_count; ++i) {
		struct AIOChannelMidi *channel = aio->midiin_channels + i;
		jack_port_t *port = (jack_port_t*)channel->impl_opaque;
		void *buffer = jack_port_get_buffer(port, nframes);

		jack_nframes_t i, events = jack_midi_get_event_count(buffer);
		for (i = 0; i < events; ++i) {
			struct AIOMidiEvent aio_event;
			jack_midi_event_t jack_event;
			jack_midi_event_get(&jack_event, buffer, i);

			aio_event.channel_index = i;
			aio_event.sample = jack_event.time;
			aio_event.size = jack_event.size;
			aio_event.bytes = jack_event.buffer;

			aio->processMidiEvent(aio, &aio_event);
		}
	}

	for (i = 0; i < aio->midiout_channels_count; ++i) {
		/* TODO */
	}

	aio->processAudio(aio);

	return 0;
}

static void aioJackCbShutdown(void *arg) {
	struct AIOJackState *state = (struct AIOJackState*)arg;
	struct AIOState *aio = state->io_state;
	aio->processException(aio);
}

SHLPError aioJackOpen(const char *name, const char *server_name, struct AIOState* aio) {
	SHLPError error_code = SHLPError_Ok;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	int i;

	if (state.io_state || state.client) {
		shlpMessage(SHLPMessage_Error, "jack client is already opened");
		return SHLPError_UnexpectedState;
	}

	state.client = jack_client_open(name, options, &status, server_name);
	if (state.client == NULL) {
		shlpMessage(SHLPMessage_Error, "jack_client_open(name:\"%s\"%s%s%s) failed: %d%s",
			name, server_name?", server_name:\"":"", server_name?server_name:"", server_name?"\"":"",
			status, (status & JackServerFailed) ? " (unable to connect to jackd)" : "");
		error_code = SHLPError_Backend;
		goto error;
	}

	if (status & JackServerStarted) {
		shlpMessage(SHLPMessage_Warning, "JACK server started");
	}
	if (status & JackNameNotUnique) {
		name = jack_get_client_name(state.client);
		shlpMessage(SHLPMessage_Warning, "unique name %s assigned", name);
	}

	for (i = 0; i < aio->input_channels_count; ++i)
	{
		struct AIOChannelAudio *channel = aio->input_channels + i;
		jack_port_t *port = jack_port_register(state.client, channel->name,
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

		if (!port) {
			shlpMessage(SHLPMessage_Error, "jack_port_register(name:\"%s\", audio, input) failed", channel->name);
			error_code = SHLPError_Backend;
			goto error;
		}
		channel->impl_opaque = port;
	}

	for (i = 0; i < aio->output_channels_count; ++i)
	{
		struct AIOChannelAudio *channel = aio->output_channels + i;
		jack_port_t *port = jack_port_register(state.client, channel->name,
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

		if (!port) {
			shlpMessage(SHLPMessage_Error, "jack_port_register(name:\"%s\", audio, output) failed", channel->name);
			error_code = SHLPError_Backend;
			goto error;
		}
		shlpMessage(SHLPMessage_Info, "jack_port_register(name:\"%s\", audio, output)", channel->name);
		channel->impl_opaque = port;
	}

	for (i = 0; i < aio->midiin_channels_count; ++i) {
		struct AIOChannelMidi *channel = aio->midiin_channels + i;
		jack_port_t *port = jack_port_register(state.client, channel->name,
				JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

		if (!port) {
			shlpMessage(SHLPMessage_Error, "jack_port_register(name:\"%s\", midi, input) failed", channel->name);
			error_code = SHLPError_Backend;
			goto error;
		}
		channel->impl_opaque = port;
	}

	for (i = 0; i < aio->midiout_channels_count; ++i) {
		struct AIOChannelMidi *channel = aio->midiout_channels + i;
		jack_port_t *port = jack_port_register(state.client, channel->name,
				JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

		if (!port) {
			shlpMessage(SHLPMessage_Error, "jack_port_register(name:\"%s\", midi, output) failed", channel->name);
			error_code = SHLPError_Backend;
			goto error;
		}
		channel->impl_opaque = port;
	}

	jack_set_process_callback(state.client, aioJackCbProcess, &state);
	jack_on_shutdown(state.client, aioJackCbShutdown, &state);

	/* maximum frame size */
	aio->frame_size = jack_get_buffer_size(state.client);
	aio->samplerate = jack_get_sample_rate(state.client);

	state.io_state = aio;

	return SHLPError_Ok;

error:
	/* TODO */
	aioJackClose();
	return error_code;
}


SHLPError aioJackActivate() {
	int error_code;

	if (!state.client) {
		shlpMessage(SHLPMessage_Error, "jack client is not opened");
		return SHLPError_UnexpectedState;
	}

	error_code = jack_activate(state.client);
	if (error_code) {
		shlpMessage(SHLPMessage_Error, "jack_activate() failed: %d", error_code);
		return SHLPError_Backend;
	}

	return SHLPError_Ok;
}

SHLPError aioJackDeactivate() {
	int error_code;

	if (!state.client) {
		shlpMessage(SHLPMessage_Error, "jack client is not opened");
		return SHLPError_UnexpectedState;
	}

	error_code = jack_deactivate(state.client);
	if (error_code) {
		shlpMessage(SHLPMessage_Error, "jack_deactivate() failed: %d", error_code);
		return SHLPError_Backend;
	}

	return SHLPError_Ok;
}

SHLPError aioJackClose() {
	if (!state.client) {
		shlpMessage(SHLPMessage_Error, "jack client is not opened");
		return SHLPError_UnexpectedState;
	}

	jack_client_close(state.client);

	memset(&state, 0, sizeof state);

	return SHLPError_Ok;
}

