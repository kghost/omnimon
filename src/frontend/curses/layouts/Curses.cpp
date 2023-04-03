#include "Curses.hpp"

#include <cassert>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "View.hpp"

namespace frontend::curses {

Curses::SigWinChange::SigWinChange(EventLoop& loop, Curses& curses) : EventSignal(loop, SIGWINCH), _Curses(curses) {}
void Curses::SigWinChange::OnSignal(SigNumType signum) { _Curses.HandleWinChangeSignal(); }

Curses::Curses(EventLoop& loop) : _SigWinChange(loop, *this), _StdIO(loop, *this), _Root(nullptr) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, true);
  keypad(stdscr, true);
  wrefresh(stdscr);
}
Curses::~Curses() { endwin(); }

void Curses::OnStdInRead() {
  for (TermKeyCode ch = getch(); ch > 0; ch = getch()) {
    if (_Root) {
      assert(!_DrawScheduled);
      _Root->OnKey(ch);
      if (_DrawScheduled) {
        Update();
        _DrawScheduled = false;
      }
    }
  }
}

void Curses::ScheduleDraw() { _DrawScheduled = true; }

void Curses::Update() {
  if (_Root) {
    UpdateContext attrs(stdscr);
    _Root->DrawPrepare(attrs);
    _Root->DrawContent(attrs);
    wnoutrefresh(stdscr);
    doupdate();
  }
}

void Curses::SetRoot(std::shared_ptr<View> root) {
  _Root = root;
  Resize();
}

void Curses::HandleWinChangeSignal() {
  struct winsize ws;
  ioctl(0, TIOCGWINSZ, &ws);
  resizeterm(ws.ws_row, ws.ws_col);

  if (_Root) {
    Resize();
    Update();
  }
}

void Curses::Resize() {
  DisplayLength height = getmaxy(stdscr), width = getmaxx(stdscr);
  assert(height > 0);
  assert(width > 0);

  _Root->SetLayout({0, 0}, {height, width});
}

} // namespace frontend::curses
