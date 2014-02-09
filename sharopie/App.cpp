#include <kapusha/sys.h>
#include "Viewport.h"

class ViewportFactory : public kapusha::IViewportFactory {
public:
  ViewportFactory(ISource *source) : source_(source) {
    p_.window_title = "Shalolpipe";
    p_.flags = Preferences::FlagOpenGL2_1 | Preferences::FlagOpenGLES2;
  }
  virtual IViewport *create(IViewportController *controller) const {
    return new Viewport(controller, source_);
  }
  virtual const Preferences &preferences() const { return p_; }
private:
  Preferences p_;
  ISource *source_;
};

const kapusha::IViewportFactory *kapusha_main(kapusha::core::StringArray *args) {
  if (args->size() < 2)
    exit(-1);

  ISource *source = ISource::create((*args)[1]->str());

  static const ViewportFactory factory(source);
  return &factory;
}
