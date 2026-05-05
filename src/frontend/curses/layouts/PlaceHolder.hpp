#pragma once

#include "View.hpp"

namespace frontend::curses {

class PlaceHolder : public View {
public:
  explicit PlaceHolder() = default;

  bool OnKey(TermKeyCode key) override { return false; }
  void DrawPrepare(const DrawContentContext& attrs) override final {}
  void DrawContent(const DrawContentContext& attrs) override final {}
};

} // namespace frontend::curses
