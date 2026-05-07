#pragma once

namespace frontend::curses {

using TermKeyCode = int;

class InputHandler {
public:
  virtual ~InputHandler() = default;
  virtual bool OnKey(TermKeyCode key) = 0;
};

} // namespace frontend::curses