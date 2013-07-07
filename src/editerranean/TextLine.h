#pragma once
#include <string>
#include <kapusha/kapusha.h>
#include <kapusha/fontain.h>

namespace sharopie {
  using namespace kapusha;
  
  class RectalDrawer;
  class Line {
  public:
    Line(const fontain::IFace *face, const std::string &str);
    ~Line();

    void reset(const fontain::IFace *face, const std::string &str);
    void prepare(Context *ctx, Material *mat);
    void draw(Context *ctx, vec2f pos, vec2f scale) const;
    
  private:
    std::string text_;
    fontain::String *visual_;
    RectalDrawer *drawer_;
  };
  
  class Lines : public ArrayOf<Line*> {
  public:
    Lines() = default;
    Lines(const fontain::IFace *face, const std::string &multiline);
    void reset(const fontain::IFace *face, const std::string &multiline);
    void prepare(Context *ctx, Material *mat);
    ~Lines();
  };
} // namespace sharopie
