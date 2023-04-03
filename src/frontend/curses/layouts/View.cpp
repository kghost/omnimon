#include "View.hpp"

#include <cassert>

namespace frontend::curses {

bool View::SetLayout(Layout offset, Layout layout) {
  assert(offset.Height >= 0);
  assert(offset.Width >= 0);
  assert(layout.Height >= 0);
  assert(layout.Width >= 0);

  bool changed = false;

  if (_Offset != offset) {
    _Offset = offset;
    changed = true;
  }

  if (_Layout != layout) {
    _Layout = layout;
    changed = true;
  }

  return changed;
}

} // namespace frontend::curses
