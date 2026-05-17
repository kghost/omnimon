
#include "TreeString.hpp"

namespace utils {

std::string TreeString(const std::list<TreeStringPosition>& positions) {
  std::string result;
  for (auto it = positions.begin(); it != positions.end(); ++it) {
    if (std::next(it) != positions.end()) {
      switch (*it) {
      case TreeStringPosition::NotLast:
        result += "│ ";
        break;
      case TreeStringPosition::Last:
        result += "  ";
        break;
      }
    } else {
      switch (*it) {
      case TreeStringPosition::NotLast:
        result += "├─";
        break;
      case TreeStringPosition::Last:
        result += "└─";
        break;
      }
    }
  }
  return result;
}

} // namespace utils
