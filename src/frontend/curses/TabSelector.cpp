#include "TabSelector.hpp"

#include <memory>

#include "EmptyView.hpp"
#include "OmniMon.hpp"
#include "ProcessTree.hpp"

namespace frontend::curses {

TabSelector::TabSelector(OmniMon& omniMon)
    : _OmniMon(omniMon), _Selection(std::make_shared<Selection>(GetTabs(), *this)) {}

void TabSelector::OnSelect(std::shared_ptr<SelectionChoice> choice) {
  _OmniMon.SelectTab(std::dynamic_pointer_cast<TabChoice>(choice));
}
void TabSelector::OnClose() { _OmniMon.CloseTabSelector(); }

bool TabSelector::OnKey(TermKeyCode key) { return _Selection->OnKey(key); }

template <std::size_t N> struct StringLiteral {
  char value[N];
  constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }
};

template <StringLiteral Lable, typename Content> class TabChoiceImpl : public TabChoice {
public:
  explicit TabChoiceImpl() = default;
  ~TabChoiceImpl() override = default;

  std::string GetLabel() const override { return Lable.value; }
  bool IsDefault() const override { return false; }

  std::shared_ptr<WindowClient>
  GetContent(std::shared_ptr<backend::metrics::SimplePublisher<int>> tick) const override {
    return std::make_shared<Content>(tick);
  }
};

std::vector<std::shared_ptr<SelectionChoice>> TabSelector::GetTabs() {
  std::vector<std::shared_ptr<SelectionChoice>> tabs;
  tabs.push_back(std::make_shared<TabChoiceImpl<"Process Monitor", ProcessTree>>());
  return tabs;
}

} // namespace frontend::curses