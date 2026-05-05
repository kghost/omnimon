#pragma once

#include "../../../utils/StringUtils.hpp"

namespace frontend::curses {

class Layout {
public:
  friend bool operator==(const Layout&, const Layout&) = default;
  friend bool operator!=(const Layout&, const Layout&) = default;

  friend Layout operator+(const Layout& a, const Layout& b) { return Layout{a.Height + b.Height, a.Width + b.Width}; }
  friend Layout operator-(const Layout& a, const Layout& b) { return Layout{a.Height - b.Height, a.Width - b.Width}; }

  DisplayLength Height = 0;
  DisplayLength Width = 0;
};

class Region {
public:
  friend bool operator==(const Region&, const Region&) = default;
  friend bool operator!=(const Region&, const Region&) = default;

  Layout _Offset;
  Layout _Size;
};

} // namespace frontend::curses
