#pragma once

#include <signal.h>

#include "../../backend/metrics/SimplePublisher.hpp"
#include "Events.hpp"
#include "Options.hpp"
#include "layouts/Base.hpp"
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

class SigWinChange : public EventSignal {
public:
  explicit SigWinChange(EventLoop& loop, OmniMon& mon) : EventSignal(loop, SIGWINCH), _OmniMon(mon) {}

  void OnSignal(SigNumType signum) override;

private:
  OmniMon& _OmniMon;
};

class IO : public EventHandle {
public:
  explicit IO(EventLoop& loop, OmniMon& mon) : EventHandle(loop, 0), _OmniMon(mon) { ScheduleRead(); }

  void OnRead() override;
  void OnWrite() override {}

private:
  OmniMon& _OmniMon;
};

class OmniMon : public InputHandler {
public:
  static OmniMon& GetInstance();

private:
  explicit OmniMon();
  ~OmniMon() = default;

public:
  void ScheduleDraw(); // TODO: remove this function
  bool OnKey(TermKeyCode key) override;

  void HandleWinChangeSignal();
  void OnStdInRead();

  void Update();
  void Run() { _Loop.Run(); }
  void Stop() { _Loop.Stop(); }

private:
  EventLoop _Loop;
  // Signal handlers must be created before the curses instance.
  SigInt _SigInt;
  SigWinChange _SigWinChange;
  IO _StdIO;
  Curses _Curses;
  std::shared_ptr<backend::metrics::SimplePublisher<int>> _Tick;
  Timer _Timer;
};

} // namespace frontend::curses
