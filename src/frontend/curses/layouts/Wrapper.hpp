#pragma once

#include <memory>

#include "AttrView.hpp"

namespace frontend::curses {

class Wrapper : public AttrView {
public:
  explicit Wrapper(std::shared_ptr<View> view) : _View(view) {}
  ~Wrapper() override = default;

  bool SetLayout(Layout offset, Layout layout) override;
  bool OnKey(TermKeyCode key) override { return _View->OnKey(key); }
  void DrawPrepare(const UpdateContext& attrs) override;
  void DoDrawContent(const UpdateContext& my) override;

private:
  std::shared_ptr<View> _View;
  bool _LocationInvalid = true;
};

} // namespace frontend::curses
