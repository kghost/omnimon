#pragma once

#include <stdint.h>

#include "StringUtils.hpp"

namespace utils {

std::string DiskSizeToString(uint64_t size, DisplayLength width);

} // namespace utils
