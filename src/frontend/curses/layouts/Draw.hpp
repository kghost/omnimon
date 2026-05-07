#pragma once

#include <ncursesw/ncurses.h>

namespace frontend::curses {

using TermAttrs = int;

class DrawPrepareContext {
public:
  explicit DrawPrepareContext(WINDOW* win) : Win(win) {}

  WINDOW* Win;
  bool Visible = true;

  DrawPrepareContext MergeWith(bool visible) const {
    DrawPrepareContext result = *this;
    result.Visible &= visible;
    return result;
  }
};

class DrawContentContext {
public:
  explicit DrawContentContext(WINDOW* win) : Win(win) {}

  WINDOW* Win;
  TermAttrs Attrs = A_NORMAL;

  DrawContentContext MergeWith(TermAttrs changes) const {
    DrawContentContext result = *this;
    result.Attrs |= changes;
    return result;
  }
};

} // namespace frontend::curses
