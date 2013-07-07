#include "Viewport.h"
#include "../editerranean/Editerranean.h"

namespace sharopie {
  Viewport::Viewport(fontain::IFace *face) : face_(face), edit_(nullptr) {
  }
  
  Viewport::~Viewport() {
  }
  
  void Viewport::init(IViewportController* controller, Context *context) {
    controller_ = controller;
    context_ = context;
    edit_ = new Editerranean();
    edit_->setFace(face_);
    edit_->setText(context_,
                   "WAVAfl\n"
                   "В чащах юга жил бы цитрус?\nДа, но фальшивый экземпляр!\n"
                   "/usr/share/fonts/liberation-fonts/LiberationMono-Regular.ttf\n"
);
  }
  
  void Viewport::resize(vec2i size) {
    glViewport(0, 0, size.x, size.y);
    edit_->setSize(size);
  }
  
  void Viewport::inputPointer(const PointerState& pointers) {
  }
  
  void Viewport::inputKey(const KeyState& keys) {
    edit_->inputKey(keys);
  }
  
  void Viewport::draw(int ms, float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    edit_->draw(context_);
  }
  
  void Viewport::close() {
    delete edit_;
  }
} // namespace sharopie