#include "Screen.hpp"

#include "ProcessTree.hpp"

#include "OmniMon.hpp"

namespace frontend::curses {

class ScreenUserContent : public Container::Child {
public:
  ScreenUserContent(std::shared_ptr<View> view) : _View(view) {}

  Container::ChildArrangement::ArrangementType GetArrangement() const override {
    return Container::ChildArrangement::ArrangementType::FillRest;
  }
  DisplayLength GetSize() const override { return 1; }
  DisplayLength GetMarginBefore() const override { return 0; }
  DisplayLength GetMarginAfter() const override { return 0; }
  View& GetView() override { return *_View; }

private:
  std::shared_ptr<View> _View;
};

frontend::curses::Screen::Screen(OmniMon& mon, std::shared_ptr<ProcessTree> ps)
    : Container(Container::GrowthType::Vertical), _OmniMon(mon) {
  AppendChild(std::make_shared<ScreenUserContent>(ps->GetView()));
}

bool Screen::OnKey(TermKeyCode key) {
  if (Container::OnKey(key)) {
    return true;
  }

  if (key == 'q') {
    _OmniMon.Stop();
    return true;
  }

  return false;
}

} // namespace frontend::curses
