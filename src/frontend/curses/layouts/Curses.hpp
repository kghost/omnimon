#pragma once

#include <memory>
#include <ncursesw/ncurses.h>

#include "Base.hpp"

namespace frontend::curses {

class Window;
class WindowClient;

class Curses {
public:
  explicit Curses(InputHandler& inputHandler);
  ~Curses();

  void SetWindow(std::shared_ptr<WindowClient> client);

  void SetPopupWindow(std::shared_ptr<WindowClient> client);
  void ClosePopupWindow();

  void HandleWinChangeSignal();
  void HandleInput();
  void ScheduleDraw();
  void Update();

private:
  InputHandler& _InputHandler;
  std::shared_ptr<Window> _Windows;
  std::shared_ptr<Window> _PopupWindows;
  bool _DrawScheduled = false;
};

} // namespace frontend::curses
