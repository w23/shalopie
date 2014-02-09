#include "Source.h"

#include <kapusha/io/IFile.h>
#include <kapusha/render/Shader.h>

FileSource::FileSource(const char *filename)
  : filename_(filename)
  , filesystem_(kapusha::io::IFilesystem::create_native())
  , monitor_(filename) {
  monitor_.start();
}
FileSource::~FileSource() {}

kapusha::render::Shader *FileSource::new_shader() {
  if (!monitor_.changed())
    return nullptr;

  kapusha::io::IFile::shared file(filesystem_->open_file(filename_));
  if (!file)
    return nullptr;

  kapusha::io::buffered_stream_t stream;
  file->stream_chunk(stream, 0);
  kapusha::core::buffer_t buffer;
  while(stream.status() == kapusha::io::buffered_stream_t::status_e::ok) {
    buffer.append(stream.cursor(), stream.left());
    stream.advance(stream.left());
  }
  kapusha::render::Shader *shader
    = new kapusha::render::Shader(
      kapusha::core::string_desc_t(buffer.data(), buffer.size()),
      kapusha::render::shader_t::type_e::fragment);
  if (*shader)
    return shader;
  
  delete shader;
  return nullptr;
}

ISource *ISource::create(const char *uri) {
  return new FileSource(uri);
}
