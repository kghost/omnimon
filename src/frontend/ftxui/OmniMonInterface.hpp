#pragma once

#include <functional>
#include <memory>
#include <string>

#include "DebugWindow.hpp"

namespace backend::events {
class EventLoop;
}

namespace backend::metrics {
class SubscriberBase;
}

namespace frontend::ftxui {

class OmniMonInterface {
public:
  virtual ~OmniMonInterface() = default;
  virtual void ScheduleRefresh() = 0;
  virtual void Debug(std::string msg, DebugWindow::DebugLevel level = DebugWindow::DebugLevel::Info) = 0;
  virtual backend::events::EventLoop& GetLoop() = 0;
  virtual std::shared_ptr<backend::metrics::SubscriberBase> OnTickUpdate(std::function<void(int)> callback) = 0;
};

} // namespace frontend::ftxui
