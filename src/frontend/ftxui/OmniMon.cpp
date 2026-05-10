#include "OmniMon.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "TabSelector.hpp"

namespace frontend::ftxui {

void Timer::OnTimer() { _OmniMon.TimerUpdate(); }

void SigWinChange::OnSignal(SigNumType signum) { _OmniMon.HandleWinChangeSignal(); }

void IO::OnRead() {
  _OmniMon.OnStdInRead();
  ScheduleRead();
}

void DeferredRender::OnNotification(uint64_t amount) { _OmniMon.Refresh(); }

OmniMon::OmniMon()
    : _Loop(), _SigInt(_Loop), _SigWinChange(_Loop, *this), _DeferredRender(_Loop, *this), _StdIO(_Loop, *this),
      _Tick(std::make_shared<backend::metrics::SimplePublisher<int>>()), _Timer(_Loop, *this),
      _Screen(::ftxui::App::FullscreenAlternateScreen()),
      _ActiveView(TabSelector::GetDefaultTab()->CreateView(_Tick, [this] { this->ScheduleRefresh(); })) {
  _Screen.TrackMouse(false); // Disables mouse tracking
}

void OmniMon::HandleWinChangeSignal() {
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
    if (_TabSelector.has_value()) {
      return dbox({_ActiveView->Render(), (*_TabSelector)->Render()});
    }
    return _ActiveView->Render();
    return text("Loading...");
  });

  auto component = CatchEvent(renderer, [&](Event event) -> bool {
    if (event == Event::Custom) {
      return true;
    } else if (event == Event::Character('q')) {
      Stop();
      return true;
    } else if (event == Event::Tab) {
      if (_TabSelector.has_value()) {
        _TabSelector.reset();
      } else {
        _TabSelector = std::make_unique<TabSelector>(
            [this](std::shared_ptr<TabChoice> choice) {
              if (choice->GetName() != _ActiveView->GetTabName()) {
                _ActiveView = choice->CreateView(_Tick, [this] { this->ScheduleRefresh(); });
              }
              _TabSelector.reset();
            },
            [this] { _TabSelector.reset(); }, _ActiveView->GetTabName());
      }
      return true;
    } else if (_TabSelector.has_value() && (*_TabSelector)->OnEvent(event)) {
      return true;
    } else if (_ActiveView->OnEvent(event)) {
      return true;
    } else {
      return false;
    }
  });

  // Default view
  _FtxuiLoop = std::make_unique<::ftxui::Loop>(&_Screen, component);
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();
  _Loop.Run();
}

void OmniMon::Stop() { _Loop.Stop(); }

void OmniMon::ScheduleRefresh() { _DeferredRender.Notify(1); }

void OmniMon::TimerUpdate() {
  _Tick->Update(_Tick->GetValue() + 1);
  Refresh();
}

void OmniMon::Refresh() {
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();
}

} // namespace frontend::ftxui
