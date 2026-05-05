#pragma once

#include <memory>
#include <ncursesw/ncurses.h>

#include "../Events.hpp"

namespace frontend::curses {

using TermKeyCode = int;

class View;
class Curses {
public:
  explicit Curses(EventLoop& loop);
  ~Curses();

  void SetRoot(std::shared_ptr<View> root);
  void HandleWinChangeSignal();

  void OnStdInRead();
  void ScheduleDraw();
  void Update();

private:
  void Resize();

  class SigWinChange : public frontend::curses::EventSignal {
  public:
    explicit SigWinChange(EventLoop& loop, Curses& curses);
    void OnSignal(SigNumType signum) override;

  private:
    Curses& _Curses;
  } _SigWinChange;

  class IO : public frontend::curses::EventHandle {
  public:
    explicit IO(EventLoop& loop, Curses& curses) : EventHandle(loop, 0), _Curses(curses) { ScheduleRead(); }
    void OnRead() override {
      _Curses.OnStdInRead();
      ScheduleRead();
    }
    void OnWrite() override {}

  private:
    Curses& _Curses;
  } _StdIO;

  std::shared_ptr<View> _Root;
  bool _DrawScheduled = false;
};

} // namespace frontend::curses
