#include "ColumnSelector.hpp"

#include <cassert>
#include <ftxui/dom/elements.hpp>
#include <ranges>

#include <ftxui/component/component.hpp> // For Checkbox and Container
#include <ftxui/screen/color.hpp>

namespace frontend::ftxui {

ColumnSelector::ColumnSelector(std::function<void()> onClose, std::vector<SelectorItem> columns)
    : _OnClose(std::move(onClose)),
      _Component(::ftxui::Container::Vertical(columns | std::views::transform([](const auto& c) -> ::ftxui::Component {
                                                return ::ftxui::Checkbox(c.Label, &c.IsShown);
                                              }) |
                                              std::ranges::to<::ftxui::Components>())) {};

bool ColumnSelector::OnEvent(::ftxui::Event event) {
  if (_Component->OnEvent(event)) {
    return true;
  } else if (event == ::ftxui::Event::Character('c') || event == ::ftxui::Event::Escape) {
    _OnClose();
    return true;
  } else {
    return true;
  }
}

::ftxui::Element ColumnSelector::Render() {
  using namespace ::ftxui;
  Element title = hbox({text(" Select Visible Columns ") | bold | color(Color::Yellow)});

  Element footer = hbox({text(" [Space] Toggle ") | bold | color(Color::Green), separatorLight(),
                         text(" [c/Esc] Close ") | bold | color(Color::Green)}) |
                   size(HEIGHT, EQUAL, 1);

  return window(title, vbox({_Component->Render() | yframe | flex, separatorLight(), footer})) | clear_under | center;
}

} // namespace frontend::ftxui
