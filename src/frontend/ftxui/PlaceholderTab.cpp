#include "PlaceholderTab.hpp"

#include <ftxui/dom/elements.hpp>

namespace frontend::ftxui {

std::string PlaceholderTab::GetTabName() const { return kPlaceholderTabName; }

bool PlaceholderTab::OnEvent(::ftxui::Event event) { return false; }

::ftxui::Element PlaceholderTab::Render() {
  using namespace ::ftxui;
  return vbox({
             text("Placeholder Tab"),
             text(""),
             text("Press Tab to switch to another tab"),
         }) |
         center | border;
}

} // namespace frontend::ftxui
