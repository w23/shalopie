#include <sstream>
#include "Editerranean.h"

namespace sharopie {
  const char *Editerranean::shaderVertex_ =
  "uniform vec2 uv2_offset;\n"
  "uniform vec2 uv2_scale;\n"
  "attribute vec2 av2_vtx;\n"
  "attribute vec2 av2_tex;\n"
  "varying "
#if KAPUSHA_GLES
  "lowp "
#endif
  "vec2 vv2_vertex;\n"
  "void main(){\n"
  "gl_Position = vec4((av2_vtx.xy + uv2_offset) * uv2_scale - vec2(1.), 0., 1.);\n"
  "vv2_vertex = av2_tex;\n"
  "}";
  
  const char *Editerranean::shaderFragment_ =
  "uniform sampler2D us2_atlas;\n"
  "varying "
#if KAPUSHA_GLES
  "lowp "
#endif
  "vec2 vv2_vertex;\n"
  "void main(){\n"
  "gl_FragColor = vec4(texture2D(us2_atlas, vv2_vertex).a+.5);\n"
  "}";
  
  Editerranean::Editerranean() : face_(nullptr) {
    Program *program = new Program(shaderVertex_, shaderFragment_);
    material_ = new Material(program);
    material_->blend().enable();
    material_->blend().setFunction(BlendState::ConstOne,
                                   BlendState::OneMinusSourceAlpha);
  }
  
  Editerranean::~Editerranean() {
    
  }
  
  void Editerranean::setFace(const fontain::IFace *face) {
    face_ = face;
  }
  
  void Editerranean::setText(Context *ctx, const std::string &text) {
    KP_ASSERT(face_);
    lines_.reset(face_, text);
    lines_.prepare(ctx, material_.get());
  }
  
  void Editerranean::setSize(vec2i size) {
    size_ = size;
    kPixelsToDevice_ = vec2f(2.f) / vec2f(size_);
    offset_ = vec2f(0.f, size.y);
  }
  
  void Editerranean::inputKey(const kapusha::KeyState& keys) {
  }
  
  void Editerranean::draw(Context *ctx) const {
    vec2f pen = offset_ - vec2f(0.f, face_->metrics().ascent);
    /// \todo for (const auto& l : lines_) {
    for (u32 i = 0; i < lines_.size(); ++i) {
      lines_[i]->draw(ctx, pen, kPixelsToDevice_);
      //pen.y -= face_->metrics().leading;
      pen.y -= face_->metrics().ascent;
      //pen.y -= face_->lineHeight();
    }
  }
    
} // namespace sharopie