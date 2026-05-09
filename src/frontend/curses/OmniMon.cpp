#include "OmniMon.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "ProcessTree.hpp"

namespace frontend::ftxui {

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
    : _Loop(), _SigInt(_Loop), _SigWinChange(_Loop, *this), _StdIO(_Loop, *this),
      _Tick(std::make_shared<backend::metrics::SimplePublisher<int>>()), _Timer(_Loop, *this),
      _Screen(::ftxui::App::FullscreenAlternateScreen()) {
  _Screen.TrackMouse(false); // Disables mouse tracking
}

void OmniMon::HandleWinChangeSignal() {
  // Run twice to ensure the screen is properly resized and rendered after a window size change
  // First time to recalculate layout based on new size, second time to render the updated layout
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();
  _ActiveView->OnTerminalSizeChange(); // Trigger initial size update
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();
}

void OmniMon::OnStdInRead() {
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();
}

void OmniMon::Run() {
  using namespace ::ftxui;

  auto renderer = Renderer([&] {
    if (_ActiveView) {
      return _ActiveView->Render();
    }
    return text("Loading...");
  });

  auto component = CatchEvent(renderer, [&](Event event) -> bool {
    if (event == Event::Character('q')) {
      Stop();
      return true;
    }
    if (_ActiveView && _ActiveView->OnEvent(event)) {
      _Screen.PostEvent(Event::Custom);
      return true;
    }
    return false;
  });

  // Default view
  _ActiveView = std::make_shared<ProcessTree>(_Tick);
  _FtxuiLoop = std::make_unique<::ftxui::Loop>(&_Screen, component);

  // Run twice to ensure the screen is properly initialized and rendered before entering the main loop
  _FtxuiLoop->RunOnce();
  _ActiveView->OnTerminalSizeChange(); // Trigger initial size update
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();

  _Loop.Run();

  // Cleanup
  _FtxuiLoop.reset();
}

void OmniMon::Stop() { _Loop.Stop(); }

void OmniMon::Update() {
  _Tick->Update(_Tick->GetValue() + 1);
  if (_FtxuiLoop) {
    _Screen.PostEvent(::ftxui::Event::Custom);
    _FtxuiLoop->RunOnce();
  }
}

void OmniMon::SelectTab(std::shared_ptr<TabChoice> choice) {
  // TODO: implement tab switching
}

void OmniMon::ShowTabSelector() { _TabSelectorShown = true; }

void OmniMon::CloseTabSelector() { _TabSelectorShown = false; }

} // namespace frontend::ftxui
