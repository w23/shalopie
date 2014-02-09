#pragma once
#include <kapusha/core/String.h>
#include <kapusha/io/IFilesystem.h>
#include "ISource.h"
#include "FileMonitor.h"

class FileSource : public ISource {
public:
  FileSource(const char *filename);

  // ISource
  ~FileSource();
  kapusha::render::Shader *new_shader();

private:
  kapusha::core::string_t filename_;
  kapusha::io::IFilesystem::shared filesystem_;
  FileMonitor monitor_;
};
