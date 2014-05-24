#pragma once
#include <AudioUnit/AudioUnit.h>

class Sound_CoreAudio {
public:
  typedef void (*synth_callback_f)(void *param, float *stream, uint32_t frames);

  Sound_CoreAudio(const synth_callback_f synth, void *param);
  ~Sound_CoreAudio();
  
  int samplerate() const { return samplerate_; }
  
  bool start();
  void stop();
  
private:
  void * const param_;
  const synth_callback_f callback_;
  int samplerate_;
  
  AudioComponentInstance aci;
  
  static OSStatus audio_cb(
    void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
    const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
    UInt32 inNumberFrames, AudioBufferList *ioData);
  
  void callback(float *stream, uint32_t frames);
};