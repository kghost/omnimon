#pragma once

#include <chrono>

namespace frontend::ftxui {

class Config {
public:
  static Config& GetInstance() {
    static Config instance;
    return instance;
  }

  std::chrono::milliseconds RefreshInterval = std::chrono::milliseconds(1000);

private:
  Config() = default;
};

} // namespace frontend::ftxui
