#pragma once

#include "layouts/Window.hpp"
#include "layouts/views/TextView.hpp"

namespace frontend::curses {

class EmptyView : public WindowClient {
public:
  EmptyView() : _View(std::make_shared<TextView>(TextView::Align::Center, "Empty View")) {}
  ~EmptyView() override = default;

  std::shared_ptr<View> GetView() override { return _View; }
  bool OnKey(TermKeyCode key) override { return false; }

private:
  std::shared_ptr<TextView> _View;
};

} // namespace frontend::curses
