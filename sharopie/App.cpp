#include <kapusha/app.h>
#include "Viewport.h"

class ViewportFactory : public IViewportFactory {
public:
  ViewportFactory() {
    p_.window_title = "Shalolpipe";
    p_.flags = Preferences::FlagOpenGL2_1 | Preferences::FlagOpenGLES2;
  }
  virtual IViewport *create(IViewportController *controller) const {
    return new Viewport(controller);
  }
  virtual const Preferences &preferences() const { return p_; }
private:
  Preferences p_;
};

ViewportFactory the_factory;

namespace kapusha {
  Application the_application = {
    &the_factory
  };
} // namespace kapusha
