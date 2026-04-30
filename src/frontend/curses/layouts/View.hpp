#pragma once

#include "Curses.hpp"

#include "../../../utils/StringUtils.hpp"
#include <memory>

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

class ViewDataBinding : public InputHandler {};

class View : public InputHandler {
public:
  explicit View() : _Layout({0, 0}), _Offset({0, 0}) {}
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

  void Bind(std::weak_ptr<ViewDataBinding> binding) { _Binding = binding; }
  std::shared_ptr<ViewDataBinding> GetBinding() const { return _Binding.lock(); }

  // Each update contains 2 stages:
  // 1. DrawPrepare: clear/remove old content.
  // 2. DrawContent: draw current content.
  virtual void DrawPrepare(const UpdateContext& attrs) = 0;
  virtual void DrawContent(const UpdateContext& attrs) = 0;

protected:
  Layout _Offset, _Layout;
  bool _Visible = true;
  std::weak_ptr<ViewDataBinding> _Binding;
};

} // namespace frontend::curses
