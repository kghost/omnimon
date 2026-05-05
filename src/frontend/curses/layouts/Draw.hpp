#pragma once

#include <ncursesw/ncurses.h>
#include <vector>

#include "Region.hpp"

namespace frontend::curses {

using TermAttrs = int;

class DrawPrepareContext {
public:
  explicit DrawPrepareContext(WINDOW* win) : Win(win) {}

  WINDOW* Win;
  bool Visible = true;
  std::vector<Region> InvalidArea;

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

  bool Visible = true;
  bool ForceRedraw = false;

  TermAttrs attrs = A_NORMAL;

  DrawContentContext MergeWith(bool visible) const {
    DrawContentContext result = *this;
    result.Visible &= visible;
    return result;
  }

  DrawContentContext MergeWith(bool visible, TermAttrs changes) const {
    DrawContentContext result = *this;
    result.Visible &= visible;
    result.attrs |= changes;
    return result;
  }
};

} // namespace frontend::curses
