#pragma once

class FileMonitor {
public:
  FileMonitor(const char *filename) : changed_(true) {}

  void start() {}

  bool changed() {
    bool ret = changed_;
    changed_ = false;
    return ret;
  }

private:
  bool changed_;
}; // class FileMonitor
