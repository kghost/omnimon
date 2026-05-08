#pragma once

#include <memory>
#include <vector>

#include "layouts/Window.hpp"
#include "layouts/views/Selection.hpp"

namespace frontend::curses {

class OmniMon;

class TabChoice : public SelectionChoice {
public:
  explicit TabChoice() = default;
  ~TabChoice() override = default;

  virtual std::shared_ptr<WindowClient>
  GetContent(std::shared_ptr<backend::metrics::SimplePublisher<int>> tick) const = 0;
};

class TabSelector : public SelectionCallbacks, public WindowClient {
public:
  explicit TabSelector(OmniMon& omniMon);
  ~TabSelector() override = default;

  void OnSelect(std::shared_ptr<SelectionChoice> choice) override;
  void OnClose() override;
  bool OnKey(TermKeyCode key) override;
  std::shared_ptr<View> GetView() override { return _Selection; };

private:
  OmniMon& _OmniMon;
  std::shared_ptr<Selection> _Selection;

  std::vector<std::shared_ptr<SelectionChoice>> GetTabs();
};

} // namespace frontend::curses
