#include <kapusha/sys.h>
#include "Viewport.h"

class ViewportFactory : public kapusha::IViewportFactory {
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

const kapusha::IViewportFactory *kapusha_main(kapusha::core::StringArray *args) {
  static const ViewportFactory factory;
  return &factory;
}
