#pragma once

#include <memory>
#include <ncursesw/ncurses.h>

#include "Base.hpp"
#include "Region.hpp"

namespace frontend::curses {

class View;

class WindowClient : public InputHandler {
public:
  virtual std::shared_ptr<View> GetView() = 0;
};

class Window {
public:
  explicit Window(Region region, std::shared_ptr<WindowClient> client);
  ~Window();

  WINDOW* GetWindow() { return _Win; }

  void SetLayout(Region region);
  void Draw();
  bool OnKey(TermKeyCode key);

private:
  WINDOW* _Win = nullptr;
  std::shared_ptr<WindowClient> _Client;
};

} // namespace frontend::curses
