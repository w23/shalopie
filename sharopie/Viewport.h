#include <kapusha/viewport.h>
#include <kapusha/io/Socket.h>
#include <kapusha/render/Batch.h>
#include <kapusha/render/Framebuffer.h>
#include "shmach.h"
#include "Murth.h"
#include "Sound_CoreAudio.h"
#include "Midi_CoreMidi.h"
#include "Ashembler.h"
#include "ISource.h"

using namespace kapusha;

class Viewport : public IViewport {
public:
  Viewport(IViewportController *controller, ISource *source, const kapusha::core::string_t &murth_cores);
  virtual ~Viewport() {}
  virtual void resize(vec2i size);
  virtual void draw(int ms, float dt);
  
private:
  void clear(vec4f color);
  render::Program::shared create_program();
  
  void murth_program(const char *filename);

  struct viewport_proxy_t {
    viewport_proxy_t(Viewport *master);
  private:
    shmach_object_t o_;
    Viewport *master_;
  };

  IViewportController *controller_;
  vec2i size_;

  viewport_proxy_t viewport_proxy_;
  shmach_core_t core_;
  
  ISource *shader_src_;
  render::Batch::shared fullscreen_;
  render::Sampler::shared sampler_noise_;
  render::Sampler::shared frame_;
  render::Framebuffer::shared framebuffer_;
  render::Batch::shared blit_;
  
  Murth murth_;
  Sound_CoreAudio sound_;
  Midi_CoreMidi midi_;
  shmach::Ashembler ashm_;
  
  static void synth_callback(void *param, float *stream, uint32_t frames);
  static void midi_callback(void *param, const void *data, uint32_t size);
};
