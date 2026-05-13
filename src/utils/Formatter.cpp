#include "Formatter.hpp"

#include <format>

namespace utils {

std::string DiskSizeToString(uint64_t size) {
  constexpr const uint64_t K = 1024ull;
  constexpr const uint64_t M = K * 1024ull;
  constexpr const uint64_t G = M * 1024ull;
  constexpr const uint64_t T = G * 1024ull;
  constexpr const uint64_t P = T * 1024ull;
  constexpr const uint64_t E = P * 1024ull;

  if (size < 10 * K) {
    return std::format("{:d}B", size);
  } else if (size < 10 * M) {
    return std::format("{:d}K", size / K);
  } else if (size < 10 * G) {
    return std::format("{:d}M", size / M);
  } else if (size < 10 * T) {
    return std::format("{:d}G", size / G);
  } else if (size < 10 * P) {
    return std::format("{:d}T", size / T);
  } else if (size < 10 * E) {
    return std::format("{:d}P", size / P);
  } else {
    return std::format("{:d}E", size / E);
  }
}

} // namespace utils
