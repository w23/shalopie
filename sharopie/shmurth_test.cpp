#include <AudioUnit/AudioUnit.h>
#include <CoreMIDI/CoreMIDI.h>
#include <stdio.h>
#include <fstream>
#include "shmurth.h"
#include "Ashembler.h"

#define CHECK_ERR(msg) {if(err!=noErr){printf("Error %d(0x%8x): %s\n", err, err, msg);exit(-1);}}

static AudioComponentInstance aci;
static MIDIClientRef mcli;
static MIDIPortRef mport;
static shmurth_t shmurth;

static OSStatus audio_cb(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
                  const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                  UInt32 inNumberFrames, AudioBufferList *ioData) {
  AudioBuffer* buf = &ioData->mBuffers[0];
  shmurth_synth(&shmurth, (float*)buf->mData, inNumberFrames);
  return noErr;
}

unsigned current_note = 0;

void midiread_cb(const MIDIPacketList *pktlist,
                 void *readProcRefCon,
                 void *srcConnRefCon) {
  const MIDIPacket *packet = &pktlist->packet[0];
  for (int i = 0; i < pktlist->numPackets; ++i) {
    //murth_process_raw_midi(packet->data, packet->length);
    printf("%d: ", packet->length);
    for (int i = 0; i < packet->length; ++i)
      printf("%02x ", packet->data[i]);
    printf("\n");
    packet = MIDIPacketNext(packet);
  }
}

void osx_audio_init(int *samplerate) {
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
  asbd.mSampleRate = *samplerate = 44100;
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
  
  err = AudioUnitSetProperty(aci,
                             kAudioUnitProperty_SetRenderCallback,
                             kAudioUnitScope_Input,
                             0,
                             &arcs, sizeof(arcs));
  CHECK_ERR("AudioUnitSetProperty")

  err = MIDIClientCreate(CFSTR("murth"), 0, 0, &mcli);
  CHECK_ERR("MIDIClientCreate");
  err = MIDIInputPortCreate(mcli, CFSTR("input"), midiread_cb, 0, &mport);
  CHECK_ERR("MIDIInputPortCreate");
}

void osx_audio_start() {
  unsigned long midi_sources = MIDIGetNumberOfSources();
  for (int i = 0; i < midi_sources; ++i)
    MIDIPortConnectSource(mport, MIDIGetSource(i), 0);
  
  OSStatus err = AudioOutputUnitStart(aci);
  CHECK_ERR("AudioOutputUnitStart");
}

void osx_audio_close() {
  OSStatus err = AudioComponentInstanceDispose(aci);
  CHECK_ERR("AudioComponentInstanceDispose")
  MIDIClientDispose(mcli);
}

int main(int argc, char *argv[]) {
  std::ifstream in(argv[1], std::ios::in);
  std::string sprog;
  in.seekg(0, std::ios::end);
  sprog.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&sprog[0], sprog.size());
  in.close();

  shmach::Ashembler ashm;
  auto prog = ashm.parse(sprog.c_str(), static_cast<uint32_t>(sprog.length()));
  
  auto section = prog.sections.begin();
  if (section == prog.sections.end()) {
    printf("ERROR: %s\n", prog.error_desc.c_str());
    return 1;
  }
  
  int samplerate;
  osx_audio_init(&samplerate);
  shmurth_init(&shmurth, samplerate, NULL, NULL);
  shmach_section_t sec;
  sec.text = section->second.data();
  shmurth_set_mixer_section(&shmurth, sec);
  osx_audio_start();
  sleep(600);
  osx_audio_close();
  return 0;
}