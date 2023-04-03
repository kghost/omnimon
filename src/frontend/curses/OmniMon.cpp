#include "OmniMon.hpp"

namespace frontend::curses {

void Timer::OnTimer() { _OmniMon.Update(); }

OmniMon& OmniMon::GetInstance() {
  static OmniMon instance;
  return instance;
}

OmniMon::OmniMon()
    : _Loop(), _SigInt(_Loop), _Curses(_Loop), _Timer(_Loop, *this), _ProcessTree(std::make_shared<ProcessTree>()),
      _Screen(std::make_shared<Screen>(*this, _ProcessTree)) {
  _Curses.SetRoot(_Screen);
}

void OmniMon::ScheduleDraw() { _Curses.ScheduleDraw(); }

void OmniMon::Update() {
  _ProcessTree->Update();
  Draw();
}

void OmniMon::Draw() { _Curses.Update(); }

} // namespace frontend::curses
