#pragma once

#include <system_error>

static int PosixE(int err) {
  if (err < 0) {
    throw std::system_error(errno, std::generic_category());
  }

  return err;
}

template <typename PredPassThrough> int PosixE(int err, PredPassThrough pred) {
  if (err < 0) {
    if (pred()) {
      return err;
    }

    throw std::system_error(errno, std::generic_category());
  }

  return err;
}

namespace utils {

class FileHandle {
public:
  explicit FileHandle(int fd) : _Fd(fd) {}
  ~FileHandle();

  FileHandle(const FileHandle&) = delete;
  FileHandle(FileHandle&&) = delete;
  FileHandle& operator=(const FileHandle&) = delete;
  FileHandle& operator=(FileHandle&&) = delete;

  operator int() const { return _Fd; }

private:
  int _Fd;
};

} // namespace utils
