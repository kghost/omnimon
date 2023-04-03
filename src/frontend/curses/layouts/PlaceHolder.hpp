#pragma once

#include <memory>
#include <string>

#include "View.hpp"

namespace frontend::curses {

class PlaceHolder : public View {
public:
  explicit PlaceHolder() = default;

  bool OnKey(TermKeyCode key) override { return false; }
  void DrawPrepare(const UpdateContext& attrs) override final {}
  void DrawContent(const UpdateContext& attrs) override final {}
};

} // namespace frontend::curses
