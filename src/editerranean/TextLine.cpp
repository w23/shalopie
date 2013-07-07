#include <sstream>
#include "TextLine.h"
#include "RectalDrawer.h"

namespace sharopie {
  Line::Line(const fontain::IFace *face, const std::string &str)
  : text_(str), visual_(face->createString(text_.c_str())), drawer_(nullptr) {
  }
  
  Line::~Line() {
    delete drawer_;
    delete visual_;
  }
  
  void Line::prepare(kapusha::Context *ctx, kapusha::Material *mat) {
    if (!drawer_) drawer_ = new RectalDrawer(mat);
    drawer_->reset(ctx, visual_);
  }
  
  void Line::draw(Context *ctx, vec2f pos, vec2f scale) const {
    KP_ASSERT(drawer_);
    drawer_->draw(ctx, pos, scale);
  }

  Lines::Lines(const fontain::IFace *face, const std::string &multiline) {
    reset(face, multiline);
  }
  
  void Lines::reset(const fontain::IFace *face, const std::string &multiline) {
    clear();
    reserve(1 + static_cast<u32>(multiline.size()) / 80);
    std::stringstream ss(multiline);
    std::string line;
    while (std::getline(ss, line)) push_back(new Line(face, line));
  }
  
  Lines::~Lines() {
    for (u32 i = 0; i < size(); ++i) (*this)[i]->~Line();
  }
  
  void Lines::prepare(kapusha::Context *ctx, kapusha::Material *mat) {
    for (u32 i = 0; i < size(); ++i) (*this)[i]->prepare(ctx, mat);
  }
} // namespace sharopie
