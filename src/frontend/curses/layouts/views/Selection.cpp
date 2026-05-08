#include "Selection.hpp"

#include <algorithm>
#include <memory>

#include "Container.hpp"

namespace frontend::curses {

Selection::ChoiceView::ChoiceView(
    std::shared_ptr<SelectionChoice> choice,
    std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>> cursor)
    : _Choice(choice), _CursorSubscriber(SetupCursorSubscripber(cursor)) {
  if (IsSelected(cursor)) {
    _Attrs = A_BOLD;
  }
  SetText(_Choice->GetLabel());
}

bool Selection::ChoiceView::IsSelected(
    std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>> cursor)
    const {
  return *cursor->GetValue() == _Choice;
}

std::shared_ptr<backend::metrics::SubscriberBase> Selection::ChoiceView::SetupCursorSubscripber(
    std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>
        cursorPublisher) {
  return backend::metrics::MakeSubscriber(
      cursorPublisher,
      [this](std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>
                 cursor) {
        bool isSelected = IsSelected(cursor);
        auto oldAttrs = _Attrs;
        if (isSelected) {
          _Attrs |= A_BOLD;
        } else {
          _Attrs &= ~A_BOLD;
        }
        if (_Attrs != oldAttrs) {
          _State = AttrView::State::Invalid;
        }
      });
}

Selection::Selection(std::vector<std::shared_ptr<SelectionChoice>> choices, SelectionCallbacks& callbacks)
    : Container(Container::GrowthType::Vertical), _Choices(choices), _Callbacks(callbacks),
      _CursorPublisher(
          std::make_shared<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>(
              GetInitialSelection())) {
  for (const auto& choice : _Choices) {
    AppendChild(std::make_shared<ChoiceView>(choice, _CursorPublisher));
  }
}

bool Selection::OnKey(TermKeyCode key) {
  auto current = _CursorPublisher->GetValue();

  switch (key) {
  case KEY_UP:
    if (current == _Choices.begin()) {
      break;
    }
    _CursorPublisher->Update(std::prev(current));
    return true;
  case KEY_DOWN:
    if (current == std::prev(_Choices.end())) {
      break;
    }
    _CursorPublisher->Update(std::next(current));
    return true;
  case '\n':
  case KEY_ENTER:
    _Callbacks.OnSelect(*current);
    _Callbacks.OnClose();
    return true;
  case 27:   // ESC
  case '\t': // Also close on TAB
    _Callbacks.OnClose();
    return true;
  }
  return false;
}

std::vector<std::shared_ptr<SelectionChoice>>::iterator Selection::GetInitialSelection() {
  auto it = std::find_if(_Choices.begin(), _Choices.end(),
                         [](std::shared_ptr<SelectionChoice> choice) { return choice->IsDefault(); });
  return _Choices.begin();
}

} // namespace frontend::curses
