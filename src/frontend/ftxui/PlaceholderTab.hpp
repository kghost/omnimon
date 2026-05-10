#pragma once

#include "TabSelector.hpp"

namespace frontend::ftxui {

class PlaceholderTab : public FtxuiTabView {
public:
  PlaceholderTab() = default;
  ~PlaceholderTab() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;
};

} // namespace frontend::ftxui
