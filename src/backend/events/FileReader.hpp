#pragma once

#include <deque>
#include <functional>
#include <optional>
#include <unistd.h>
#include <vector>

namespace backend::events {

class FileReadState {
public:
  std::optional<std::tuple<ssize_t, std::function<void(std::vector<char>)>>> Request;
  std::deque<char> Buffer;
};

class FileReader {
public:
  explicit FileReader(int fd, FileReadState& state) : _Fd(fd), _State(state) {}

  bool IsReading() const;

  // Requests to read at least `size` bytes. Returns a buffer containing the data if the request is fulfilled, or
  // std::nullopt if the request can not be fulfilled at the moment. The caller can call Continue() when the file
  // becomes readable to continue the pending request. Can be called when there is already a pending request.
  void Request(ssize_t size, std::function<void(std::vector<char>)> callback);

  // Continues a pending request. Returns a buffer containing the data if the request is fulfilled, or std::nullopt if
  // the request is still pending. The request must be pending before calling this method.
  void Continue();

private:
  int _Fd;
  FileReadState& _State;
};

} // namespace backend::events