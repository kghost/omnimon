#pragma once

#include <chrono>
#include <csignal>
#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>

#include "../../backend/metrics/SimplePublisher.hpp"
#include "Events.hpp"
#include "FtxuiView.hpp"
#include "Options.hpp"

namespace frontend::ftxui {

class OmniMon;
class TabChoice;

class Timer : public EventTimer {
public:
  Timer(EventLoop& loop, OmniMon& mon)
      : EventTimer(loop, std::chrono::microseconds(1), Config::GetInstance().RefreshInterval), _OmniMon(mon) {}

  void OnTimer() override;

private:
  OmniMon& _OmniMon;
};

class SigInt : public EventSignal {
public:
  explicit SigInt(EventLoop& loop) : EventSignal(loop, SIGINT), _Loop(loop) {}

  void OnSignal(SigNumType signum) override { _Loop.Stop(); }

private:
  EventLoop& _Loop;
};

class SigWinChange : public EventSignal {
public:
  explicit SigWinChange(EventLoop& loop, OmniMon& mon) : EventSignal(loop, SIGWINCH), _OmniMon(mon) {}

  void OnSignal(SigNumType signum) override;

private:
  OmniMon& _OmniMon;
};

class IO : public EventHandle {
public:
  explicit IO(EventLoop& loop, OmniMon& mon) : EventHandle(loop, STDIN_FILENO, false), _OmniMon(mon) { ScheduleRead(); }

  void OnRead() override;
  void OnWrite() override {}

private:
  OmniMon& _OmniMon;
};

class OmniMon {
public:
  static OmniMon& GetInstance();

private:
  explicit OmniMon();
  ~OmniMon() = default;

public:
  void HandleWinChangeSignal();
  void OnStdInRead();

  void Update();
  void Run();
  void Stop();

  void SelectTab(std::shared_ptr<TabChoice> choice);
  void ShowTabSelector();
  void CloseTabSelector();

private:
  EventLoop _Loop;
  SigInt _SigInt;
  SigWinChange _SigWinChange;
  IO _StdIO;

  std::shared_ptr<backend::metrics::SimplePublisher<int>> _Tick;
  Timer _Timer;

  ::ftxui::App _Screen;
  std::unique_ptr<::ftxui::Loop> _FtxuiLoop;
  std::shared_ptr<FtxuiView> _ActiveView;
  bool _TabSelectorShown = false;
};

} // namespace frontend::ftxui
