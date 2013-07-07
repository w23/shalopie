#pragma once
#include <string>
#include <vector>
#include <kapusha/viewport.h>
#include <kapusha/fontain.h>
#include "TextLine.h"

namespace sharopie {
  using namespace kapusha;
  class Editerranean {
  public:
    Editerranean();
    ~Editerranean();
    
    void setFace(const fontain::IFace *face);
    void setText(Context *ctx, const std::string &text);
    void setSize(vec2i size);
    void inputKey(const kapusha::KeyState& keys);
    void draw(Context *ctx) const;
  private:
    void rebuild(const std::string &text);
    struct Cursor {
      int line;
      int glyph, textIndex;
    };
    
    const fontain::IFace *face_;
    vec2i size_;
    vec2f offset_;
    vec2f kPixelsToDevice_;
    Lines lines_;
    Cursor cursor_;
    SMaterial material_;
    
    static const char *shaderVertex_;
    static const char *shaderFragment_;
  }; // class Editerranean
} // namespace sharopie