#include "OmniMon.hpp"

#include <cassert>

#include "ProcessTree.hpp"
#include "TabSelector.hpp"
#include "layouts/Base.hpp"

namespace frontend::curses {

void Timer::OnTimer() { _OmniMon.Update(); }

void SigWinChange::OnSignal(SigNumType signum) { _OmniMon.HandleWinChangeSignal(); }

void IO::OnRead() {
  _OmniMon.OnStdInRead();
  ScheduleRead();
}

OmniMon& OmniMon::GetInstance() {
  static OmniMon instance;
  return instance;
}

OmniMon::OmniMon()
    : _Loop(), _SigInt(_Loop), _SigWinChange(_Loop, *this), _StdIO(_Loop, *this), _Curses(*this),
      _Tick(std::make_shared<backend::metrics::SimplePublisher<int>>()), _Timer(_Loop, *this) {
  _Curses.SetWindow(std::make_shared<ProcessTree>(_Tick));
}

void OmniMon::ScheduleDraw() { _Curses.ScheduleDraw(); }

void OmniMon::HandleWinChangeSignal() { _Curses.HandleWinChangeSignal(); }

void OmniMon::OnStdInRead() { _Curses.HandleInput(); }

bool OmniMon::OnKey(TermKeyCode key) {
  if (key == 'q') {
    Stop();
    return true;
  }
  if (key == '\t') {
    if (!_TabSelectorShown) {
      ShowTabSelector();
    } else {
      CloseTabSelector();
    }
    return true;
  }
  return false;
}

void OmniMon::SelectTab(std::shared_ptr<TabChoice> choice) { _Curses.SetWindow(choice->GetContent(_Tick)); }

void OmniMon::ShowTabSelector() {
  assert(!_TabSelectorShown);
  _Curses.SetPopupWindow(std::make_shared<TabSelector>(*this));
  _TabSelectorShown = true;
}

void OmniMon::CloseTabSelector() {
  assert(_TabSelectorShown);
  _Curses.ClosePopupWindow();
  _TabSelectorShown = false;
}

void OmniMon::Update() {
  _Tick->Update(_Tick->GetValue() + 1);
  _Curses.Update();
}

} // namespace frontend::curses
