#include "OmniMon.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "../../backend/metrics/Binding.hpp"
#include "DebugWindow.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

void Timer::OnTimer() { _OmniMon.TimerUpdate(); }

void SigWinChange::OnSignal(backend::events::SigNumType signum) { _OmniMon.HandleWinChangeSignal(); }

void IO::OnRead() {
  _OmniMon.OnStdInRead();
  ScheduleRead();
}

void DeferredRender::OnNotification(uint64_t amount) { _OmniMon.Refresh(); }

OmniMon::OmniMon()
    : _Loop(), _SigInt(_Loop), _SigWinChange(_Loop, *this), _DeferredRender(_Loop, *this), _StdIO(_Loop, *this),
      _Tick(std::make_shared<backend::metrics::SimplePublisher<int>>()), _Timer(_Loop, *this),
      _Screen(::ftxui::App::FullscreenAlternateScreen()), _DebugWindow(std::make_unique<DebugWindow>(*this)),
      _ActiveView(TabSelector::GetDefaultTab()->CreateView(*this)) {
  _Screen.TrackMouse(false);
  _Screen.CatchSignals(false);
  Debug("OmniMon TUI started.", DebugWindow::DebugLevel::Info);
  Debug("Press ` (backtick) to toggle this debug console.", DebugWindow::DebugLevel::Info);
}

void OmniMon::HandleWinChangeSignal() { Refresh(); }
void OmniMon::OnStdInRead() { Refresh(); }
void OmniMon::TimerUpdate() {
  _Tick->Update(_Tick->GetValue() + 1);
  Refresh();
}

void OmniMon::Refresh() {
  _Screen.PostEvent(::ftxui::Event::Custom);
  _FtxuiLoop->RunOnce();
}

void OmniMon::Run() {
  using namespace ::ftxui;

  auto renderer = Renderer([&] {
    std::vector<Element> stacks;
    stacks.emplace_back(_ActiveView->Render());
    if (_TabSelector.has_value()) {
      stacks.emplace_back(_TabSelector.value()->Render());
    }
    if (_DebugWindow->IsVisible()) {
      stacks.emplace_back(_DebugWindow->Render());
    }
    return dbox(std::move(stacks));
  });

  auto component = CatchEvent(renderer, [&](Event event) -> bool {
    if (event == Event::Custom) {
      return true;
    }

    if (_DebugWindow->OnEvent(event)) {
      return true;
    }

    if (event == Event::Character('q')) {
      Stop();
      return true;
    } else if (event == Event::Tab) {
      if (_TabSelector.has_value()) {
        _TabSelector.reset();
      } else {
        _TabSelector = std::make_unique<TabSelector>(
            [this](std::shared_ptr<TabChoice> choice) {
              if (choice->GetName() != _ActiveView->GetTabName()) {
                _ActiveView = choice->CreateView(*this);
                Debug("Switched active view to: " + choice->GetName(), DebugWindow::DebugLevel::Info);
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

void OmniMon::Debug(std::string msg, DebugWindow::DebugLevel level) { _DebugWindow->Log(std::move(msg), level); }

backend::events::EventLoop& OmniMon::GetLoop() { return _Loop; }

std::shared_ptr<backend::metrics::SubscriberBase> OmniMon::OnTickUpdate(std::function<void(int)> callback) {
  return backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<int>>(_Tick, std::move(callback));
}

} // namespace frontend::ftxui
