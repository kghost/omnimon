#pragma once

#include <chrono>
#include <csignal>
#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>
#include <optional>
#include <string>

#include "../../backend/events/Events.hpp"
#include "../../backend/metrics/SimplePublisher.hpp"
#include "DebugWindow.hpp"
#include "OmniMonInterface.hpp"
#include "Options.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

class OmniMon;

class Timer : public backend::events::EventTimer {
public:
  Timer(backend::events::EventLoop& loop, OmniMon& mon)
      : backend::events::EventTimer(loop, std::chrono::microseconds(1), Config::GetInstance().RefreshInterval),
        _OmniMon(mon) {}

  void OnTimer() override;

private:
  OmniMon& _OmniMon;
};

class SigInt : public backend::events::EventSignal {
public:
  explicit SigInt(backend::events::EventLoop& loop) : backend::events::EventSignal(loop, SIGINT), _Loop(loop) {}

  void OnSignal(backend::events::SigNumType signum) override { _Loop.Stop(); }

private:
  backend::events::EventLoop& _Loop;
};

class SigWinChange : public backend::events::EventSignal {
public:
  explicit SigWinChange(backend::events::EventLoop& loop, OmniMon& mon)
      : backend::events::EventSignal(loop, SIGWINCH), _OmniMon(mon) {}

  void OnSignal(backend::events::SigNumType signum) override;

private:
  OmniMon& _OmniMon;
};

class IO : public backend::events::EventHandle {
public:
  explicit IO(backend::events::EventLoop& loop, OmniMon& mon)
      : backend::events::EventHandle(loop, STDIN_FILENO, false), _OmniMon(mon) {
    ScheduleRead();
  }

  void OnRead() override;
  void OnWrite() override {}

private:
  OmniMon& _OmniMon;
};

class DeferredRender : public backend::events::EventNotification {
public:
  explicit DeferredRender(backend::events::EventLoop& loop, OmniMon& mon)
      : backend::events::EventNotification(loop), _OmniMon(mon) {}
  void OnNotification(uint64_t amount) override;

private:
  OmniMon& _OmniMon;
};

class OmniMon : public OmniMonInterface {
public:
  explicit OmniMon();
  ~OmniMon() override = default;

  void HandleWinChangeSignal();
  void OnStdInRead();
  void TimerUpdate();
  void Refresh();

  void ScheduleRefresh() override;
  void Debug(std::string msg, DebugWindow::DebugLevel level = DebugWindow::DebugLevel::Info) override;
  backend::events::EventLoop& GetLoop() override;
  std::shared_ptr<backend::metrics::SubscriberBase> OnTickUpdate(std::function<void(int)> callback) override;

  void Run();
  void Stop();

private:
  backend::events::EventLoop _Loop;
  SigInt _SigInt;
  SigWinChange _SigWinChange;
  DeferredRender _DeferredRender;
  IO _StdIO;

  std::shared_ptr<backend::metrics::SimplePublisher<int>> _Tick;
  Timer _Timer;

  ::ftxui::App _Screen;
  std::unique_ptr<::ftxui::Loop> _FtxuiLoop;
  std::unique_ptr<DebugWindow> _DebugWindow;
  std::unique_ptr<FtxuiTabView> _ActiveView;
  std::optional<std::unique_ptr<TabSelector>> _TabSelector;
};

} // namespace frontend::ftxui
