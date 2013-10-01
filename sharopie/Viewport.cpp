#include "Viewport.h"

const char *Viewport::s_vertex_shader_ =
  "attribute vec4 av4_vertex;\n"
  "void main(){\n"
    "gl_Position = av4_vertex;\n"
  "}";


Viewport::Viewport(IViewportController *controller)
: controller_(controller) {
  socket_.bind(Socket::Address(5470));

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
  
  Buffer *buf_rect = new Buffer();
  buf_rect->load(rect, sizeof rect);
  Program *prog = new Program(s_vertex_shader_, shader_fragment);
  
  fullscreen_ = new Batch();
  fullscreen_->setMaterial(new Material(prog));
  fullscreen_->setAttribSource("av4_vertex", buf_rect, 2);
  fullscreen_->setGeometry(Batch::GeometryTriangleStrip, 0, 4);
}

void Viewport::resize(vec2i size) {
  size_ = size;
  glViewport(0, 0, size_.x, size_.y);
  fullscreen_->getMaterial()->setUniform("uv2_resolution", vec2f(size_));
}

void Viewport::draw(int ms, float dt) {
  
  char buffer[65536];;
  Socket::Address remote;
  u32 recv = socket_.recv_from(remote, buffer, sizeof(buffer) - 1);
  if (recv) {
    buffer[recv] = 0;
    SProgram new_prog(new Program(s_vertex_shader_, buffer, Program::TolerateInvalid));
    if (new_prog->valid()) {
      fullscreen_->setMaterial(new Material(new_prog.get()));
      fullscreen_->getMaterial()->setUniform("uv2_resolution", vec2f(size_));
    }
  }
  
  fullscreen_->getMaterial()->setUniform("uf_time", ms / 1000.f);
  fullscreen_->draw();
}
