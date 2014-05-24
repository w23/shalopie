#pragma once
#include <CoreMIDI/CoreMIDI.h>

class Midi_CoreMidi {
public:
  Midi_CoreMidi();
  ~Midi_CoreMidi();
  
  bool start();
  void stop();
  
private:
  MIDIClientRef mcli;
  
  static void midiread_cb(
    const MIDIPacketList *pktlist,
    void *readProcRefCon,
    void *srcConnRefCon);
};