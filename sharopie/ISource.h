#pragma once

namespace kapusha {
namespace render {
class Shader;
} // namespace render
} // namespace kapusha

class ISource {
public:
  virtual ~ISource() {}
  virtual kapusha::render::Shader *new_shader() = 0;

  static ISource *create(const char *uri);
}; // class ISource
