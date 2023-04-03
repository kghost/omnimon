#pragma once

#include <signal.h>

#include "Events.hpp"
#include "Options.hpp"
#include "ProcessTree.hpp"
#include "Screen.hpp"
#include "layouts/Curses.hpp"

namespace frontend::curses {

class OmniMon;

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

class OmniMon {
public:
  static OmniMon& GetInstance();

private:
  explicit OmniMon();
  ~OmniMon() = default;

public:
  void ScheduleDraw();

  void Update();
  void Draw();
  void Run() { _Loop.Run(); }
  void Stop() { _Loop.Stop(); }

private:
  EventLoop _Loop;
  // Signal handlers must be created before the curses instance.
  SigInt _SigInt;
  Curses _Curses;
  Timer _Timer;
  std::shared_ptr<ProcessTree> _ProcessTree;
  std::shared_ptr<Screen> _Screen;
};

} // namespace frontend::curses
