#include "Curses.hpp"

#include <cassert>
#include <sys/ioctl.h>
#include <termios.h>

#include "Base.hpp"
#include "Window.hpp"

namespace frontend::curses {

void Curses::HandleWinChangeSignal() {
  struct winsize ws;
  ioctl(0, TIOCGWINSZ, &ws);
  resizeterm(ws.ws_row, ws.ws_col);

  if (_RootWindow) {
    _RootWindow->SetLayout({{0, 0}, {ws.ws_row, ws.ws_col}});
    Update();
  }
}

Curses::Curses(InputHandler& inputHandler) : _InputHandler(inputHandler), _RootWindow(nullptr) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, true);
  keypad(stdscr, true);
  wrefresh(stdscr);
  wgetch(stdscr);
}
Curses::~Curses() { endwin(); }

void Curses::HandleInput() {
  if (_RootWindow) {
    for (TermKeyCode ch = wgetch(_RootWindow->GetWindow()); ch > 0; ch = wgetch(_RootWindow->GetWindow())) {
      if (_InputHandler.OnKey(ch)) {
        continue;
      }

      assert(!_DrawScheduled);
      _RootWindow->OnKey(ch);
      if (_DrawScheduled) {
        Update();
        _DrawScheduled = false;
      }
    }
  }
}

void Curses::ScheduleDraw() { _DrawScheduled = true; }

void Curses::Update() {
  if (_RootWindow) {
    _RootWindow->Draw();
    doupdate();
  }
}

void Curses::SetWindow(std::shared_ptr<WindowClient> client) {
  _RootWindow = std::make_shared<Window>(Region{{0, 0}, {getmaxy(stdscr), getmaxx(stdscr)}}, client);
}

} // namespace frontend::curses
