#pragma once
#include <kapusha/kapusha.h>
#include <kapusha/fontain.h>

namespace sharopie {
  using namespace kapusha;

  class RectalDrawer {
  public:
    RectalDrawer(Material *mat);
    ~RectalDrawer();
    void draw(Context *ctx, vec2f offset, vec2f scale);
    void reset(Context *ctx, const fontain::String *string);
  private:
    void makeRect(rect2f vtx, rect2f tex);
    struct Vertex {
      vec2f vtx, tex;
      Vertex() = default;
      Vertex(vec2f x, vec2f t) : vtx(x), tex(t) {}
    };
    ArrayOf<Vertex> vertices_;
    ArrayOf<u16> indices_;
    Batch batch_;
    Buffer vertexBuffer_, indexBuffer_;
    u32 locOffset_, locScale_, locAtlas_;
    SAtlas atlas_;
  }; // class RectalDrawer

} // namespace sharopie