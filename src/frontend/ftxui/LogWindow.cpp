#include "LogWindow.hpp"

#include <algorithm>

#include "OmniMonInterface.hpp"

namespace frontend::ftxui {

LogWindow::LogWindow(OmniMonInterface& omniMon) : _OmniMon(omniMon) {}

void LogWindow::Log(std::string msg, LogWindow::LogLevel level) {
  std::string prefix;
  switch (level) {
  case LogLevel::Info:
    prefix = "[INFO] ";
    break;
  case LogLevel::Warning:
    prefix = "[WARN] ";
    break;
  case LogLevel::Error:
    prefix = "[ERROR] ";
    break;
  case LogLevel::Log:
    prefix = "[Log] ";
    break;
  }
  _Messages.Push(LogWindow::LogMessage{prefix + std::move(msg), level});
  _OmniMon.ScheduleRefresh();
}

void LogWindow::Toggle() {
  if (_Visible == State::Show) {
    _Visible = State::Hide;
  } else {
    _Visible = State::Show;
    _ScrollOffset = 0;
    _AutoScroll = true;
  }
}

void LogWindow::ToggleFloating() {
  if (_Visible == State::Floating) {
    _Visible = State::Hide;
  } else {
    _Visible = State::Floating;
    _ScrollOffset = 0;
    _AutoScroll = true;
  }
}

bool LogWindow::IsVisible() const { return _Visible != State::Hide; }

bool LogWindow::OnEvent(::ftxui::Event event) {
  using namespace ::ftxui;

  if (event == Event::Character('`')) {
    Toggle();
    return true;
  }

  if (event == Event::Character('~')) {
    ToggleFloating();
    return true;
  }

  if (_Visible == State::Hide) {
    return false;
  }

  if (event == Event::Escape) {
    _Visible = State::Hide;
    return true;
  }

  if (_Visible == State::Floating) {
    return false;
  }

  if (event == Event::ArrowUp || event == Event::Character('k')) {
    _AutoScroll = false;
    if (_ScrollOffset > 0) {
      _ScrollOffset--;
    }
    return true;
  } else if (event == Event::ArrowDown || event == Event::Character('j')) {
    _ScrollOffset++;
    int max_scroll = std::max(0, (int)_Messages.Size() - 15);
    if (_ScrollOffset >= max_scroll) {
      _ScrollOffset = max_scroll;
      _AutoScroll = true;
    }
    return true;
  } else if (event == Event::PageUp) {
    _AutoScroll = false;
    _ScrollOffset = std::max(0, _ScrollOffset - 10);
    return true;
  } else if (event == Event::PageDown) {
    _ScrollOffset += 10;
    int max_scroll = std::max(0, (int)_Messages.Size() - 15);
    if (_ScrollOffset >= max_scroll) {
      _ScrollOffset = max_scroll;
      _AutoScroll = true;
    }
    return true;
  } else if (event == Event::Character('c') || event == Event::Character('C')) {
    _Messages.Clear();
    _ScrollOffset = 0;
    _AutoScroll = true;
    return true;
  }

  return true; // Consume other events in Log window when visible
}

::ftxui::Element LogWindow::Render() {
  using namespace ::ftxui;

  std::vector<LogWindow::LogMessage> msgs = _Messages.ToVector();

  int visible_height = (_Visible == State::Floating) ? 5 : 15;
  int max_scroll = std::max(0, (int)msgs.size() - visible_height);
  if (_Visible == State::Floating) {
    _ScrollOffset = max_scroll;
  } else {
    if (_AutoScroll) {
      _ScrollOffset = max_scroll;
    } else if (_ScrollOffset > max_scroll) {
      _ScrollOffset = max_scroll;
    }
    if (_ScrollOffset < 0) {
      _ScrollOffset = 0;
    }
  }

  Elements message_elements;
  int start_idx = _ScrollOffset;
  int end_idx = std::min((int)msgs.size(), start_idx + visible_height);

  for (int i = start_idx; i < end_idx; ++i) {
    const auto& msg = msgs[i];
    Element msg_el = text(msg.Text);
    switch (msg.Level) {
    case LogLevel::Error:
      msg_el = msg_el | color(Color::Red);
      break;
    case LogLevel::Warning:
      msg_el = msg_el | color(Color::Yellow);
      break;
    case LogLevel::Info:
      msg_el = msg_el | color(Color::Green);
      break;
    case LogLevel::Log:
      msg_el = msg_el | color(Color::Cyan);
      break;
    }
    message_elements.push_back(msg_el);
  }

  if (message_elements.empty()) {
    message_elements.push_back(text("No Log messages collected.") | dim | center);
  }

  while ((int)message_elements.size() < visible_height) {
    message_elements.push_back(text(""));
  }

  if (_Visible == State::Floating) {
    Element title = hbox({text(" Log (Realtime) ") | bold | color(Color::Cyan)});
    Element float_el = window(title, vbox(std::move(message_elements))) | size(WIDTH, EQUAL, 55) |
                       size(HEIGHT, EQUAL, 7) | clear_under;
    return vbox({filler(), hbox({filler(), float_el})});
  } else {
    std::string scroll_status = "";
    if (!msgs.empty()) {
      scroll_status = " [Line " + std::to_string(_ScrollOffset + 1) + "-" + std::to_string(end_idx) + "/" +
                      std::to_string(msgs.size()) + "]";
      if (_AutoScroll) {
        scroll_status += " (Auto-Scroll)";
      }
    }

    Element title = hbox({text(" OmniMon Log Console ") | bold | color(Color::Yellow), text(scroll_status) | dim});

    Element footer = hbox({text(" [`/Esc] Close ") | bold | color(Color::Green), separatorLight(),
                           text(" [↑/↓] Scroll ") | bold | color(Color::Green), separatorLight(),
                           text(" [c] Clear ") | bold | color(Color::Green)});

    return window(title, vbox({vbox(std::move(message_elements)), separatorLight(), footer})) | size(WIDTH, EQUAL, 80) |
           size(HEIGHT, EQUAL, 19) | clear_under | center;
  }
}

} // namespace frontend::ftxui
