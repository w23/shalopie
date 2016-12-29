#pragma once
#include <CoreMIDI/CoreMIDI.h>

typedef void (*midi_callback_f)(void *param, const void *data, uint32_t size);

class Midi_CoreMidi {
public:
  Midi_CoreMidi(midi_callback_f func, void *param);
  ~Midi_CoreMidi();
  
  bool start();
  void stop();
  
private:
  void * const param_;
  const midi_callback_f func_;
  MIDIClientRef mcli;
  
  void midi_packet(const void *data, uint32_t size) {
    func_(param_, data, size);
  }
  
  static void midiread_cb(
    const MIDIPacketList *pktlist,
    void *readProcRefCon,
    void *srcConnRefCon);
};