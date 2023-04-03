#pragma once

#include "Curses.hpp"

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

class InputHandler {
public:
  virtual ~InputHandler() = default;
  virtual bool OnKey(TermKeyCode key) = 0;
};

class View : public InputHandler {
public:
  View() = default;
  virtual ~View() = default;

  View(const View&) = delete;
  View(View&&) = delete;
  View& operator=(View&) = delete;
  View& operator=(View&&) = delete;

  const Layout& GetOffset() const { return _Offset; }
  const Layout& GetLayout() const { return _Layout; }
  virtual bool SetLayout(Layout offset, Layout layout);

  bool GetVisible() const { return _Visible; }
  void SetVisible(bool visible) { _Visible = visible; }

  // Each update contains 2 stages:
  // 1. DrawPrepare: clear/remove old content.
  // 2. DrawContent: draw current content.
  virtual void DrawPrepare(const UpdateContext& attrs) = 0;
  virtual void DrawContent(const UpdateContext& attrs) = 0;

protected:
  Layout _Offset, _Layout;
  bool _Visible = true;
};

} // namespace frontend::curses
