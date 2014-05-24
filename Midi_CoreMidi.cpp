#include "Midi_CoreMidi.h"

#define CHECK_ERR(msg) {if(err!=noErr){printf("Error %d(0x%8x): %s\n", err, err, msg);exit(-1);}}

void Midi_CoreMidi::midiread_cb(const MIDIPacketList *pktlist,
                 void *readProcRefCon,
                 void *srcConnRefCon) {
  Midi_CoreMidi *m = static_cast<Midi_CoreMidi*>(readProcRefCon);
  const MIDIPacket *packet = &pktlist->packet[0];
  for (int i = 0; i < pktlist->numPackets; ++i) {
    //murth_process_raw_midi(packet->data, packet->length);
    m->midi_packet(packet->data, packet->length);
    printf("%d: ", packet->length);
    for (int i = 0; i < packet->length; ++i)
      printf("%02x ", packet->data[i]);
    printf("\n");
    packet = MIDIPacketNext(packet);
  }
}

Midi_CoreMidi::Midi_CoreMidi(midi_callback_f func, void *param)
  : param_(param), func_(func) {}
Midi_CoreMidi::~Midi_CoreMidi() {}


bool Midi_CoreMidi::start() {
  OSStatus err;
  MIDIPortRef mport;
  err = MIDIClientCreate(CFSTR("murth"), 0, NULL, &mcli);
  CHECK_ERR("MIDIClientCreate");
  err = MIDIInputPortCreate(mcli, CFSTR("input"), midiread_cb, this, &mport);
  CHECK_ERR("MIDIInputPortCreate");
  
  unsigned long midi_sources = MIDIGetNumberOfSources();
  for (int i = 0; i < midi_sources; ++i)
    MIDIPortConnectSource(mport, MIDIGetSource(i), 0);
  return true;
}

void Midi_CoreMidi::stop() {
  MIDIClientDispose(mcli);
}