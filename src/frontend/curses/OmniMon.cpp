#include "OmniMon.hpp"

#include "ProcessTree.hpp"

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
  return false;
}

void OmniMon::Update() {
  _Tick->Update(_Tick->GetValue() + 1);
  _Curses.Update();
}

} // namespace frontend::curses
