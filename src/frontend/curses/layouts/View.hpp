#pragma once

#include <memory>

#include "Curses.hpp"
#include "Draw.hpp"
#include "Region.hpp"

namespace frontend::curses {

class InputHandler {
public:
  virtual ~InputHandler() = default;
  virtual bool OnKey(TermKeyCode key) = 0;
};

class ViewDataBinding : public InputHandler {};

class View : public InputHandler {
public:
  explicit View() : _Region({{0, 0}, {0, 0}}) {}
  virtual ~View() = default;

  View(const View&) = delete;
  View(View&&) = delete;
  View& operator=(View&) = delete;
  View& operator=(View&&) = delete;

  const Layout& GetOffset() const { return _Region._Offset; }
  const Layout& GetLayout() const { return _Region._Size; }
  const Region& GetRegion() const { return _Region; }
  virtual bool SetLayout(Region region);

  bool GetVisible() const { return _Visible; }
  void SetVisible(bool visible) { _Visible = visible; }

  void Bind(std::weak_ptr<ViewDataBinding> binding) { _Binding = binding; }
  std::shared_ptr<ViewDataBinding> GetBinding() const { return _Binding.lock(); }

  // Each update contains 2 stages:
  // 1. DrawPrepare: clear/remove old content.
  // 2. DrawContent: draw current content.
  virtual void DrawPrepare(const DrawContentContext& attrs) = 0;
  virtual void DrawContent(const DrawContentContext& attrs) = 0;

protected:
  Region _Region;
  bool _Visible = true;
  std::weak_ptr<ViewDataBinding> _Binding;
};

} // namespace frontend::curses
