#pragma once
#include <atomic>
#include <pthread.h>
#include <kapusha/core/String.h>

class FileMonitor {
public:
  FileMonitor(const char *filename);
  ~FileMonitor();

  void start();

  bool changed() const;

private:
  void stop();
  void thread_main();
  static void *thread_func(void *monitor);

  kapusha::core::string_t filename_;
  int inotify_fd_;
  int file_watch_;
  std::atomic_uint file_version_;
  mutable unsigned int checked_version_;

  pthread_t thread_;
  std::atomic_int cancel_;
}; // class FileMonitor
