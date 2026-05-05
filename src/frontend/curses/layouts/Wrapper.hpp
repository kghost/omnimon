#pragma once

#include <memory>

#include "AttrView.hpp"

namespace frontend::curses {

class Wrapper : public AttrView {
public:
  explicit Wrapper(std::shared_ptr<View> view) : _View(view) {}
  ~Wrapper() override = default;

  bool SetLayout(Region region) override;
  bool OnKey(TermKeyCode key) override { return _View->OnKey(key); }
  void DrawPrepare(const DrawContentContext& attrs) override;
  void DoDrawContent(const DrawContentContext& my) override;

private:
  std::shared_ptr<View> _View;
};

} // namespace frontend::curses
