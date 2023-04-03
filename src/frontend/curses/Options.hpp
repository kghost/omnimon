#pragma once

#include <chrono>

namespace frontend::curses {

class Config {
public:
  static Config GetInstance();

  std::chrono::steady_clock::duration RefreshInterval = std::chrono::seconds(1);
};

} // namespace frontend::curses
