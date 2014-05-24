#include "Sound_CoreAudio.h"

#define CHECK_ERR(msg) {if(err!=noErr){printf("Error %d(0x%8x): %s\n", err, err, msg);exit(-1);}}

OSStatus Sound_CoreAudio::audio_cb(
  void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
  const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
  UInt32 inNumberFrames, AudioBufferList *ioData)
{
  Sound_CoreAudio *a = static_cast<Sound_CoreAudio*>(inRefCon);
  AudioBuffer* buf = &ioData->mBuffers[0];
  a->callback((float*)buf->mData, inNumberFrames);
  return noErr;
}

void Sound_CoreAudio::callback(float *stream, uint32_t frames) {
  callback_(param_, stream, frames);
}

Sound_CoreAudio::Sound_CoreAudio(const synth_callback_f synth, void *param)
  : param_(param)
  , callback_(synth)
{
}

Sound_CoreAudio::~Sound_CoreAudio() {
}

bool Sound_CoreAudio::start() {
  AudioComponentDescription ac_desc;
  ac_desc.componentType = kAudioUnitType_Output;
  ac_desc.componentSubType = kAudioUnitSubType_DefaultOutput;
  ac_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  ac_desc.componentFlags = ac_desc.componentFlagsMask = 0;
  AudioComponent ac = AudioComponentFindNext(0, &ac_desc);
  
  OSStatus err = AudioComponentInstanceNew(ac, &aci);
  CHECK_ERR("AudioComponentInstanceNew")
  
  err = AudioUnitInitialize(aci);
  CHECK_ERR("AudioUnitInitialize")
  
  AudioStreamBasicDescription asbd;
  asbd.mSampleRate = samplerate_ = 44100;
  asbd.mFormatID = kAudioFormatLinearPCM;
  asbd.mFormatFlags = kLinearPCMFormatFlagIsFloat;//kLinearPCMFormatFlagIsSignedInteger;
  asbd.mFramesPerPacket = 1;
  asbd.mBitsPerChannel = 32;//16;
  asbd.mChannelsPerFrame = 2;
  asbd.mBytesPerFrame = asbd.mBitsPerChannel * asbd.mChannelsPerFrame / 8;
  asbd.mBytesPerPacket = asbd.mBytesPerFrame * asbd.mFramesPerPacket;
  asbd.mReserved = 0;
  err = AudioUnitSetProperty(aci,
                             kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Input,
                             0,
                             &asbd, sizeof(asbd));
  CHECK_ERR("AudioUnitSetProperty")
  
  AURenderCallbackStruct arcs;
  arcs.inputProc = audio_cb;
  arcs.inputProcRefCon = this;

  err = AudioUnitSetProperty(aci,
                             kAudioUnitProperty_SetRenderCallback,
                             kAudioUnitScope_Input,
                             0,
                             &arcs, sizeof(arcs));
  CHECK_ERR("AudioUnitSetProperty")

  err = AudioOutputUnitStart(aci);
  CHECK_ERR("AudioOutputUnitStart");
  return true;
}

void Sound_CoreAudio::stop() {
  OSStatus err = AudioComponentInstanceDispose(aci);
  CHECK_ERR("AudioComponentInstanceDispose")
}
