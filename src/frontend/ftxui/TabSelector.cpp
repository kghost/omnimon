#include "TabSelector.hpp"

#include <cassert>
#include <memory>
#include <vector>

#include "CGroupTree.hpp"
#include "OmniMonInterface.hpp"
#include "PlaceholderTab.hpp"
#include "ProcessTree.hpp"

namespace frontend::ftxui {

class ProcessTreeTabChoice : public TabChoice {
public:
  std::string GetName() const override { return kProcessTreeTabName; }

  std::unique_ptr<FtxuiTabView> CreateView(OmniMonInterface& interface) const override {
    return std::make_unique<ProcessTree>(interface);
  }
};

class CGroupTabChoice : public TabChoice {
public:
  std::string GetName() const override { return kCGroupV2TabName; }

  std::unique_ptr<FtxuiTabView> CreateView(OmniMonInterface& interface) const override {
    return std::make_unique<CGroupTree>(interface);
  }
};

class PlaceholderTabChoice : public TabChoice {
public:
  std::string GetName() const override { return kPlaceholderTabName; }

  std::unique_ptr<FtxuiTabView> CreateView(OmniMonInterface& interface) const override {
    return std::make_unique<PlaceholderTab>();
  }
};

TabSelector::TabSelector(TabSelector::OnTabSelected onTabSelected, TabSelector::OnTabSelectorClosed onTabSelectorClosed,
                         const std::string& currentTabName)
    : _OnTabSelected(std::move(onTabSelected)), _OnTabSelectorClosed(std::move(onTabSelectorClosed)) {
  _Tabs.push_back(std::make_shared<ProcessTreeTabChoice>());
  _Tabs.push_back(std::make_shared<CGroupTabChoice>());
  _Tabs.push_back(std::make_shared<PlaceholderTabChoice>());

  _Cursor = std::find_if(_Tabs.begin(), _Tabs.end(),
                         [&](const std::shared_ptr<TabChoice>& tab) { return tab->GetName() == currentTabName; });
  assert(_Cursor != _Tabs.end());
}

std::shared_ptr<TabChoice> TabSelector::GetDefaultTab() { return std::make_shared<CGroupTabChoice>(); }

bool TabSelector::OnEvent(::ftxui::Event event) {
  using namespace ::ftxui;

  if (event == Event::ArrowUp) {
    if (_Cursor != _Tabs.begin()) {
      _Cursor = std::prev(_Cursor);
    }
    return true;
  } else if (event == Event::ArrowDown) {
    if (_Cursor != std::prev(_Tabs.end())) {
      ++_Cursor;
    }
    return true;
  } else if (event == Event::Return) {
    _OnTabSelected((_Cursor == _Tabs.end()) ? nullptr : *_Cursor);
    return true;
  } else if (event == Event::Escape) {
    _OnTabSelectorClosed();
    return true;
  } else {
    return false;
  }
}

::ftxui::Element TabSelector::Render() {
  using namespace ::ftxui;

  Elements tabElements;
  tabElements.push_back(text(""));
  for (const auto& tab : _Tabs) {
    Element node = text(tab->GetName());
    if (tab == *_Cursor) {
      node = node | inverted | bold;
    }
    tabElements.push_back(node | center);
  }
  tabElements.push_back(text(""));

  Element title = hbox({text("Select Tab") | bold | color(Color::Yellow)});

  Element footer = hbox({text(" [Tab/Esc] Close ") | bold | color(Color::Green), separatorLight(),
                         text(" [↑/↓] Move ") | bold | color(Color::Green), separatorLight(),
                         text(" [Enter] Select ") | bold | color(Color::Green)});

  return window(title, vbox({vbox(std::move(tabElements)), separatorLight(), footer})) | size(WIDTH, EQUAL, 60) |
         clear_under | center;
}

} // namespace frontend::ftxui
