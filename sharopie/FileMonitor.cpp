#include <sys/select.h>
#include <sys/inotify.h>
#include <unistd.h> // ::close()
#include <stdio.h> // FILENAME_MAX

#include <kapusha/core/log.h>
#include <kapusha/core/assert.h>
#include "FileMonitor.h"

FileMonitor::FileMonitor(const char *filename)
  : filename_(filename), file_version_(1), checked_version_(0) {
  inotify_fd_ = inotify_init();
  file_watch_ = inotify_add_watch(inotify_fd_, filename, IN_MODIFY);
}

FileMonitor::~FileMonitor() {
  stop();
  ::close(inotify_fd_);
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
  fd_set fds;
  struct timeval tv;
  char buffer[sizeof(struct inotify_event) + FILENAME_MAX + 1];
  for(;;) {
    FD_ZERO(&fds);
    FD_SET(inotify_fd_, &fds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int sel = select(inotify_fd_ + 1, &fds, NULL, NULL, &tv);

    if (cancel_.load())
      break;

    if (sel == 0) // timeout
      continue;

    ssize_t size = read(inotify_fd_, buffer, sizeof buffer);
    KP_ASSERT(size >= static_cast<ssize_t>(sizeof(struct inotify_event)));

    struct inotify_event *event = reinterpret_cast<struct inotify_event*>(buffer);
    if (event->wd == file_watch_)
      file_version_++;

    /// \todo this is not entirely correct -- a file might be not created yet
    /// \fixme A RACE!
    if (event->mask & IN_IGNORED) {
      L("re-adding watch (FIXME there is a race here)");
      file_watch_ = inotify_add_watch(inotify_fd_, filename_.str(), IN_MODIFY);
    }
  }
}

void *FileMonitor::thread_func(void *monitor) {
  FileMonitor *self = reinterpret_cast<FileMonitor*>(monitor);
  self->thread_main();
  return nullptr;
}
