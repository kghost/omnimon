#pragma once

#include <chrono>
#include <memory>

#include "../metrics/SharedGauge.hpp"

namespace backend::system {

class Uptime : public metrics::LastUpdateOwner {
public:
  Uptime();
  ~Uptime() override = default;

  void Update();

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  std::shared_ptr<metrics::SharedPublisher<std::chrono::duration<double>>> GetUptime() const { return _Uptime; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::duration<double>>> GetIdle() const { return _Idle; }

private:
  std::chrono::steady_clock::time_point _LastUpdate;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::duration<double>>> _Uptime;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::duration<double>>> _Idle;
};

} // namespace backend::system
