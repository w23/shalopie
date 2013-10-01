#include <kapusha/viewport.h>
#include <kapusha/io/Socket.h>
#include <kapusha/render/Batch.h>

using namespace kapusha;

class Viewport : public IViewport {
public:
  Viewport(IViewportController *controller);
  virtual ~Viewport() {}
  virtual void resize(vec2i size);
  virtual void inputPointer(const PointerState& pointers) {}
  virtual void inputKey(const KeyState& keys) {}
  virtual void draw(int ms, float dt);
  
private:
  IViewportController *controller_;
  Socket socket_;
  SBatch fullscreen_;
  vec2i size_;
  
  static const char *s_vertex_shader_;
};