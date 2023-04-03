#include "Formatter.hpp"

#include <format>

namespace utils {

std::string DiskSizeToString(uint64_t size, DisplayLength width) {
  if (size < 10 * 1000ull /* ~10K */) {
    return std::format("{:>{}d}B", size, width - 1);
  } else if (size < 10ull * 1000ull * 1024ull /* ~10M */) {
    return std::format("{:>{}.0f}K", static_cast<double>(size) / 1024ull, width - 1);
  } else if (size < 10ull * 1000ull * 1024ull * 1024ull /* ~10G */) {
    return std::format("{:>{}.0f}M", static_cast<double>(size) / (1024ull * 1024ull), width - 1);
  } else if (size < 10ull * 1000ull * 1024ull * 1024ull * 1024ull /* ~10T */) {
    return std::format("{:>{}.0f}G", static_cast<double>(size) / (1024ull * 1024ull * 1024ull), width - 1);
  } else {
    return std::format("{:>{}.0f}T", static_cast<double>(size) / (1024ull * 1024ull * 1024ull * 1024ull), width - 1);
  }
}

} // namespace utils
