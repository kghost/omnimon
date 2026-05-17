#pragma once

#include <list>
#include <string>

namespace utils {

enum class TreeStringPosition { NotLast, Last };

std::string TreeString(const std::list<TreeStringPosition>& positions);

} // namespace utils