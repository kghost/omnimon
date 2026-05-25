#include "Uptime.hpp"

#include <chrono>
#include <fstream>

namespace backend::system {

Uptime::Uptime() {
  _Uptime = std::make_shared<metrics::SharedPublisher<std::chrono::duration<double>>>(*this);
  _Idle = std::make_shared<metrics::SharedPublisher<std::chrono::duration<double>>>(*this);
}

void Uptime::Update() {
  std::ifstream file("/proc/uptime");
  if (file.is_open()) {
    double uptime, idle;
    file >> uptime >> idle;
    _Uptime->SetValue(std::chrono::duration<double>(uptime));
    _Idle->SetValue(std::chrono::duration<double>(idle));
    _LastUpdate = std::chrono::steady_clock::now();
  }
}

} // namespace backend::system
