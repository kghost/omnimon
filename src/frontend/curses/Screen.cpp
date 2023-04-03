#include "Screen.hpp"

#include "ProcessTree.hpp"

#include "OmniMon.hpp"

namespace frontend::curses {

frontend::curses::Screen::Screen(OmniMon& mon, std::shared_ptr<ProcessTree> ps)
    : Container(Container::GrowthType::TopDown), _OmniMon(mon) {
  AppendChild(ps->GetView(), std::make_shared<frontend::curses::Container::SimpleContext>(
                                 frontend::curses::Container::ArrangementType::FillRest, 1));
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
