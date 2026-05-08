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

  if (_Windows) {
    _Windows->SetLayout({{0, 0}, {ws.ws_row, ws.ws_col}});
  }
  Update();
}

Curses::Curses(InputHandler& inputHandler) : _InputHandler(inputHandler) {
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
  if (_Windows) {
    for (TermKeyCode ch = wgetch(_Windows->GetWindow()); ch > 0; ch = wgetch(_Windows->GetWindow())) {
      if (_PopupWindows && _PopupWindows->OnKey(ch)) {
        continue;
      }

      assert(!_DrawScheduled);
      if (_Windows->OnKey(ch)) {
        if (_DrawScheduled) {
          Update();
          _DrawScheduled = false;
        }
        continue;
      }

      if (_InputHandler.OnKey(ch)) {
        continue;
      }
    }
  }
}

void Curses::ScheduleDraw() { _DrawScheduled = true; }

void Curses::Update() {
  if (_Windows) {
    _Windows->Draw();
  }
  if (_PopupWindows) {
    _PopupWindows->Draw();
  }
  doupdate();
}

void Curses::SetWindow(std::shared_ptr<WindowClient> client) {
  _Windows = std::make_shared<Window>(Region{{0, 0}, {getmaxy(stdscr), getmaxx(stdscr)}}, client);
}

void Curses::SetPopupWindow(std::shared_ptr<WindowClient> client) {
  _PopupWindows = std::make_shared<Window>(Region{{0, 0}, {getmaxy(stdscr), getmaxx(stdscr)}}, client);
}

void Curses::ClosePopupWindow() {
  _PopupWindows = nullptr;
  Update();
}

} // namespace frontend::curses
