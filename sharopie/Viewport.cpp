#include "Viewport.h"

const char *Viewport::s_vertex_shader_ =
  "attribute vec4 av4_vertex;\n"
  "void main(){\n"
    "gl_Position = av4_vertex;\n"
  "}";

Viewport::Viewport(IViewportController *controller, ISource *source)
  : controller_(controller), source_(source) {

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
  fullscreen_->set_material(new render::Material(prog));
  fullscreen_->set_attrib_source("av4_vertex", buf_rect, 2);
  fullscreen_->set_geometry(render::Batch::Geometry::TriangleStrip, 0, 4);
}

void Viewport::resize(vec2i size) {
  size_ = size;
  glViewport(0, 0, size_.x, size_.y);
  fullscreen_->material()->set_uniform("uv2_resolution", vec2f(size_));
}

void Viewport::draw(int ms, float dt) {
  render::Shader *new_shader = source_->new_shader();
  if (new_shader) {
    render::shader_t sh_vertex(s_vertex_shader_, render::shader_t::type_e::vertex);
    render::Program *prog = new render::Program(sh_vertex, *new_shader);
    if (*prog) {
      fullscreen_->set_material(new render::Material(prog));
      fullscreen_->material()->set_uniform("uv2_resolution", vec2f(size_));
    }
  }

  fullscreen_->material()->set_uniform("uf_time", ms / 1000.f);
  fullscreen_->draw();
}

