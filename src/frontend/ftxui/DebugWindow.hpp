#pragma once

#include <deque>
#include <string>
#include <vector>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::ftxui {

template <typename T> class CircularBuffer {
public:
  explicit CircularBuffer(size_t capacity) : _Capacity(capacity) {}

  void Push(T msg) {
    if (_Messages.size() >= _Capacity) {
      _Messages.pop_front();
    }
    _Messages.push_back(std::move(msg));
  }

  size_t Size() const { return _Messages.size(); }
  void Clear() { _Messages.clear(); }
  bool Empty() const { return _Messages.empty(); }

  std::vector<T> ToVector() const { return std::vector<T>(_Messages.begin(), _Messages.end()); }

private:
  size_t _Capacity;
  std::deque<T> _Messages;
};

class OmniMonInterface;

class DebugWindow {
public:
  enum class DebugLevel { Info, Warning, Error, Debug };

  struct LogMessage {
    std::string Text;
    DebugLevel Level;
  };

  enum class State { Hide, Show, Floating };

  explicit DebugWindow(OmniMonInterface& interface);
  ~DebugWindow() = default;

  void Log(std::string msg, DebugLevel level = DebugLevel::Info);
  void Toggle();
  void ToggleFloating();
  bool IsVisible() const;

  bool OnEvent(::ftxui::Event event);
  ::ftxui::Element Render();

private:
  CircularBuffer<LogMessage> _Messages{1000};
  State _Visible = State::Hide;
  int _ScrollOffset = 0;
  bool _AutoScroll = true;
  OmniMonInterface& _Interface;
};

} // namespace frontend::ftxui
