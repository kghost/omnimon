#pragma once

#include <ncursesw/ncurses.h>

#include "../Events.hpp"
#include <memory>

namespace frontend::curses {

using TermAttrs = int;
using TermKeyCode = int;

class Attrs {
public:
  explicit Attrs();
  TermAttrs attrs;
};

class UpdateContext {
public:
  explicit UpdateContext(WINDOW* win);

  WINDOW* Win;

  bool Visible = true;
  bool ForceRedraw = false;

  TermAttrs attrs;

  UpdateContext MergeWith(bool visible) const {
    UpdateContext result = *this;
    result.Visible &= visible;
    return result;
  }

  UpdateContext MergeWith(bool visible, const Attrs& changes) const {
    UpdateContext result = *this;
    result.Visible &= visible;
    result.attrs |= changes.attrs;
    return result;
  }
};

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
