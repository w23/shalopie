#include <kapusha/math/rand_lcg.h>
#include "Viewport.h"
#include "Ashembler.h"

const char *Viewport::s_vertex_shader_ =
  "attribute vec4 av4_vertex;\n"
  "void main(){\n"
    "gl_Position = av4_vertex;\n"
  "}";

/*
static const char *blit_vshader =
"attribute vec4 av4_vertex;\n"
"varying vec2 uv2_tex;\n"
"void main(){\n"
"gl_Position = av4_vertex;\n"
"uv2_tex = av4_vertex.xy * .5 + .5;\n"
"}";

static const char *blit_fshader =
"uniform sampler2D us2_frame;\n"
"varying vec2 uv2_tex;\n"
"void main(){\n"
"gl_FragColor = texture2D(us2_frame, uv2_tex);\n"
"}";*/

static void viewport_func(struct shmach_object_t_ *obj, struct shmach_core_t_ *core, uint32_t index)
{
}

Viewport::viewport_proxy_t::viewport_proxy_t(Viewport *master)
  : master_(master)
{
  o_.dtor = nullptr;
  o_.func = viewport_func;
  o_.ref = 0;
}

void Viewport::synth_callback(void *param, float *stream, uint32_t frames) {
  Murth *m = static_cast<Murth*>(param);
  m->synthesize(stream, frames);
}

Viewport::Viewport(IViewportController *controller, ISource *source)
  : controller_(controller)
  , viewport_proxy_(this) // \fixme incorrect! 'this' is not ready yet!
  , sound_(synth_callback, &murth_)
{

/*
  static const char* shader_fragment =
    "uniform vec2 uv2_resolution;\n"
    "uniform float uf_time;\n"
    "void main(){\n"
      "vec2 p = gl_FragCoord.xy / uv2_resolution;\n"
      "gl_FragColor = vec4(sin(10.*(uf_time+p.x+p.y)));\n"
    "}";

  vec2f rect[4] = {
    vec2f(-1.f,  1.f),
    vec2f(-1.f, -1.f),
    vec2f( 1.f,  1.f),
    vec2f( 1.f, -1.f)
  };

  render::Buffer *buf_rect = new render::Buffer();
  buf_rect->load(rect, sizeof rect);

  render::shader_t sh_vertex(s_vertex_shader_, render::shader_t::type_e::vertex);
  render::shader_t sh_fragment(shader_fragment, render::shader_t::type_e::fragment);
  render::Program *prog = new render::Program(sh_vertex, sh_fragment);

  fullscreen_ = new render::Batch();
  render::Ba
  fullscreen_->set_material(new render::Material(prog));
  fullscreen_->set_attrib_source("av4_vertex", buf_rect, 2);
  fullscreen_->set_geometry(render::Batch::Geometry::TriangleStrip, 0, 4);

  {
    framebuffer_ = new render::Framebuffer();
    frame_ = new render::Sampler();

    render::shader_t blitv(blit_vshader, render::shader_t::type_e::vertex);
    if (!blitv) L("err: %s", blitv.info_log()->str());
    render::shader_t blitf(blit_fshader, render::shader_t::type_e::fragment);
    if (!blitf) L("err: %s", blitf.info_log()->str());
    blit_ = new render::Batch();
    blit_->set_material(new render::Material(new render::Program(blitv, blitf)));
    blit_->set_attrib_source("av4_vertex", buf_rect, 2);
    blit_->set_geometry(render::Batch::Geometry::TriangleStrip, 0, 4);
    blit_->material()->set_uniform("us2_frame", frame_);
    
  }
    
  core::Surface *surf = new core::Surface(
    core::Surface::Meta(vec2i(256,256), core::Surface::Meta::RGBA8888));
  rand_lcg32_t rand(17);
  for (int i = 0; i < surf->meta().size.x * surf->meta().size.y; ++i)
    surf->pixels<u32>()[i] = rand.get();
  sampler_noise_ = new render::Sampler(surf);
  delete surf;
  */
  
  static const char *sadness =
" .mixer "
" load0 "
" loop: "
" 0.02 fadd "
" fphase "
" dup fph2rad fsin dup "
" 2 yield "
" :loop jmp ";

  shmach::Ashembler ashm;
  
  auto prog = ashm.parse(sadness, static_cast<uint32_t>(strlen(sadness)));
  
  auto section = prog.sections.begin();
  if (section == prog.sections.end()) {
    printf("ERROR: %s\n", prog.error_desc.c_str());
    exit(-1);
  }

  murth_.queue_mixer_reprogram(section->second.data(),
    static_cast<uint32_t>(section->second.size()));
  
  midi_.start();
  sound_.start();
}

void Viewport::resize(vec2i size) {
  size_ = size;
  glViewport(0, 0, size_.x, size_.y);
}

void Viewport::draw(int ms, float dt) {
/*
  shmach_core_return_t ret = shmach_core_run(&core_, 128);
  switch(ret.result) {
  case shmach_core_return_t::shmach_core_return_result_hang:
    L("core hang");
  case shmach_core_return_t::shmach_core_return_result_return:
    L("quit");
    controller_->quit();
    break;
  default: break;
  }
  */
}

