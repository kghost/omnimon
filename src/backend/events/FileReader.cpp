#include "FileReader.hpp"

#include <algorithm>
#include <cassert>
#include <errno.h>
#include <ranges>
#include <system_error>
#include <unistd.h>

namespace backend::events {

bool FileReader::IsReading() const { return _State.Request.has_value(); }

void FileReader::Request(ssize_t request, std::function<void(std::vector<char>)> callback) {
  assert(request > 0);
  assert(!IsReading());
  _State.Request = std::make_tuple(request, callback);
  return Continue();
}

void FileReader::Continue() {
  assert(IsReading());
  auto& [request, callback] = *_State.Request;
  ssize_t available = _State.Buffer.size();

  while (available < request) {
    std::array<char, 4096> temp;
    ssize_t bytes = read(_Fd, temp.data(), temp.size());
    if (bytes < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      } else {
        throw std::system_error(errno, std::generic_category(), "read file");
      }
    } else if (bytes == 0) {
      throw std::system_error(errno, std::generic_category(), "read file, end of file");
    } else {
      _State.Buffer.append_range(std::span{temp.data(), temp.data() + bytes});
      available += bytes;
    }
  }

  std::vector<char> result(_State.Buffer.begin(), _State.Buffer.begin() + request);
  _State.Buffer.erase(_State.Buffer.begin(), _State.Buffer.begin() + request);
  _State.Request.reset();
  callback(result);
}

} // namespace backend::events