#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::ftxui {

class FtxuiView {
public:
  virtual ~FtxuiView() = default;
  virtual bool OnEvent(::ftxui::Event event) = 0;
  virtual ::ftxui::Element Render() = 0;
};

} // namespace frontend::ftxui
