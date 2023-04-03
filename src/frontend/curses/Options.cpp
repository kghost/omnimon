#include "Options.hpp"

#include <chrono>

namespace frontend::curses {

Config Config::GetInstance() {
  static Config config;
  return config;
}

} // namespace frontend::curses
