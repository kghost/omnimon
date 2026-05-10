#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "FtxuiView.hpp"

namespace backend::metrics {
template <typename T> class SimplePublisher;
}

namespace frontend::ftxui {

constexpr const char* kProcessTreeTabName = "Process Tree";
constexpr const char* kPlaceholderTabName = "Placeholder";

class FtxuiTabView : public FtxuiView {
public:
  ~FtxuiTabView() override = default;
  virtual std::string GetTabName() const = 0;
};

class TabChoice {
public:
  virtual ~TabChoice() = default;
  virtual std::string GetName() const = 0;
  virtual std::shared_ptr<FtxuiTabView> CreateView(std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                                                   std::function<void()> refresh) const = 0;
};

class TabSelector : public FtxuiView {
public:
  using OnTabSelected = std::function<void(std::shared_ptr<TabChoice>)>;
  using OnTabSelectorClosed = std::function<void()>;

  static std::shared_ptr<TabChoice> GetDefaultTab();

  explicit TabSelector(OnTabSelected onTabSelected, OnTabSelectorClosed onTabSelectorClosed,
                       const std::string& currentTabName = "");
  ~TabSelector() override = default;

  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;

private:
  OnTabSelected _OnTabSelected;
  OnTabSelectorClosed _OnTabSelectorClosed;
  std::vector<std::shared_ptr<TabChoice>> _Tabs;
  std::vector<std::shared_ptr<TabChoice>>::iterator _Cursor;
};

} // namespace frontend::ftxui
