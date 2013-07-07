#pragma once
#include <kapusha/viewport.h>
#include <kapusha/fontain.h>

namespace sharopie {
  using namespace kapusha;
  class Editerranean;
  class Viewport : public kapusha::IViewport {
  public:
    Viewport(fontain::IFace *face);
    virtual ~Viewport();

    virtual void init(IViewportController* controller, Context *context);
    virtual void resize(vec2i size);
    virtual void inputPointer(const PointerState& pointers);
    virtual void inputKey(const KeyState& keys);
    virtual void draw(int ms, float dt);
    virtual void close();

  private:
    fontain::IFace *face_;
    IViewportController *controller_;
    Context *context_;
    Editerranean *edit_;
  }; // class Viewport
} // namespace sharopie