#include <kapusha/app.h>
#include "Viewport.h"

class ViewportFactory : public IViewportFactory {
public:
  virtual IViewport *create(IViewportController *controller) const {
    return new Viewport(controller);
  }
};

ViewportFactory the_factory;

namespace kapusha {
  Application the_application = {
    "Shalopie",
    vec2i(1280, 720),
    &the_factory
  };
} // namespace kapusha