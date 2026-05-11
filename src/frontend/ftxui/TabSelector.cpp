#include "TabSelector.hpp"

#include <cassert>
#include <ranges>
#include <vector>

#include "CGroupTree.hpp"
#include "PlaceholderTab.hpp"
#include "ProcessTree.hpp"

namespace frontend::ftxui {

class ProcessTreeTabChoice : public TabChoice {
public:
  std::string GetName() const override { return kProcessTreeTabName; }

  std::shared_ptr<FtxuiTabView> CreateView(backend::events::EventLoop& loop,
                                           std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                                           std::function<void()> refresh) const override {
    return std::make_shared<ProcessTree>(tick, refresh);
  }
};

class CGroupTabChoice : public TabChoice {
public:
  std::string GetName() const override { return kCGroupV2TabName; }

  std::shared_ptr<FtxuiTabView> CreateView(backend::events::EventLoop& loop,
                                           std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                                           std::function<void()> refresh) const override {
    return std::make_shared<CGroupTree>(loop, tick, refresh);
  }
};

class PlaceholderTabChoice : public TabChoice {
public:
  std::string GetName() const override { return kPlaceholderTabName; }

  std::shared_ptr<FtxuiTabView> CreateView(backend::events::EventLoop& loop,
                                           std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                                           std::function<void()> refresh) const override {
    return std::make_shared<PlaceholderTab>();
  }
};

TabSelector::TabSelector(TabSelector::OnTabSelected onTabSelected, TabSelector::OnTabSelectorClosed onTabSelectorClosed,
                         const std::string& currentTabName)
    : _OnTabSelected(std::move(onTabSelected)), _OnTabSelectorClosed(std::move(onTabSelectorClosed)) {
  _Tabs.push_back(GetDefaultTab());
  _Tabs.push_back(std::make_shared<CGroupTabChoice>());
  _Tabs.push_back(std::make_shared<PlaceholderTabChoice>());

  _Cursor = std::find_if(_Tabs.begin(), _Tabs.end(),
                         [&](const std::shared_ptr<TabChoice>& tab) { return tab->GetName() == currentTabName; });
  assert(_Cursor != _Tabs.end());
}

std::shared_ptr<TabChoice> TabSelector::GetDefaultTab() { return std::make_shared<ProcessTreeTabChoice>(); }

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
  return vbox(_Tabs | std::views::transform([&](const std::shared_ptr<TabChoice>& tab) {
                Element node = text(tab->GetName());
                if (tab == *_Cursor) {
                  node = node | inverted | bold;
                }
                return node | center;
              }) |
              std::ranges::to<std::vector<Element>>()) |
         clear_under | border | center;
}

} // namespace frontend::ftxui
