#include "View.hpp"

#include <cassert>

namespace frontend::curses {

bool View::SetLayout(Region region) {
  assert(region._Offset.Height >= 0);
  assert(region._Offset.Width >= 0);
  assert(region._Size.Height >= 0);
  assert(region._Size.Width >= 0);

  if (_Region != region) {
    _Region = region;
    return true;
  }

  return false;
}

} // namespace frontend::curses
