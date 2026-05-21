#pragma once

#include <ftxui/component/component_base.hpp>
#include <string>
#include <vector>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::ftxui {

class ColumnSelector {
public:
  struct SelectorItem {
    const std::string Label;
    bool& IsShown;
  };

  explicit ColumnSelector(std::function<void()> onClose, std::vector<SelectorItem> columns);
  ~ColumnSelector() = default;

  bool OnEvent(::ftxui::Event event);
  ::ftxui::Element Render();

private:
  std::function<void()> _OnClose;
  ::ftxui::Component _Component;
};

} // namespace frontend::ftxui
