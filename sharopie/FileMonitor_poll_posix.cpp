#include <sys/stat.h>
#include <unistd.h> // ::close()
#include <stdio.h> // FILENAME_MAX
#include <errno.h>

#include <kapusha/core/log.h>
#include <kapusha/core/assert.h>

#include "FileMonitor_poll_posix.h"

FileMonitor::FileMonitor(const char *filename)
: filename_(filename), file_version_(1), checked_version_(0), cancel_(0) {
}

FileMonitor::~FileMonitor() {
  stop();
}

void FileMonitor::start() {
  pthread_create(&thread_, NULL, thread_func, this);
}

bool FileMonitor::changed() const {
  if (checked_version_ == file_version_.load())
    return false;
  
  checked_version_ = file_version_.load();
  return true;
}

void FileMonitor::stop() {
  cancel_ = 1;
  pthread_join(thread_, NULL);
}

void FileMonitor::thread_main() {
  struct stat prevstat;
  memset(&prevstat, 0, sizeof(prevstat));
  for(;!cancel_.load();) {
    sleep(1);
    struct stat st;
    if (0 != stat(filename_.str(), &st)) {
      L("stat(%s): %d", filename_.str(), errno);
      continue;
    }
    
    if (prevstat.st_mtimespec.tv_sec != st.st_mtimespec.tv_sec) {
      file_version_++;
      memcpy(&prevstat, &st, sizeof(st));
    }
  }
}

void *FileMonitor::thread_func(void *monitor) {
  FileMonitor *self = reinterpret_cast<FileMonitor*>(monitor);
  self->thread_main();
  return nullptr;
}
