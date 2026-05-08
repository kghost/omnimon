#pragma once

#include <memory>
#include <vector>

#include "../../../../backend/metrics/SimplePublisher.hpp"
#include "Container.hpp"
#include "TextView.hpp"

namespace frontend::curses {

class SelectionChoice {
public:
  virtual ~SelectionChoice() = default;
  virtual std::string GetLabel() const = 0;
  virtual bool IsDefault() const = 0;
};

class SelectionCallbacks {
public:
  virtual ~SelectionCallbacks() = default;
  virtual void OnSelect(std::shared_ptr<SelectionChoice> choice) = 0;
  virtual void OnClose() = 0;
};

class Selection : public Container {
public:
  explicit Selection(std::vector<std::shared_ptr<SelectionChoice>> choices, SelectionCallbacks& callbacks);
  ~Selection() override = default;

  bool OnKey(TermKeyCode key) override;

  class ChoiceView : public TextView, public Container::Child {
  public:
    explicit ChoiceView(
        std::shared_ptr<SelectionChoice> choice,
        std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>
            cursorPublisher);
    ~ChoiceView() override = default;

    ChildArrangement::ArrangementType GetArrangement() const override {
      return ChildArrangement::ArrangementType::Forward;
    }
    DisplayLength GetSize() const override { return 1; }
    DisplayLength GetMarginBefore() const override { return 0; }
    DisplayLength GetMarginAfter() const override { return 0; }
    View& GetView() override { return *this; }

  private:
    std::shared_ptr<SelectionChoice> _Choice;
    std::shared_ptr<backend::metrics::SubscriberBase> _CursorSubscriber;

    bool IsSelected(
        std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>
            cursor) const;
    std::shared_ptr<backend::metrics::SubscriberBase> SetupCursorSubscripber(
        std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>
            cursor);
  };

private:
  std::vector<std::shared_ptr<SelectionChoice>> _Choices;
  std::shared_ptr<backend::metrics::SimplePublisher<std::vector<std::shared_ptr<SelectionChoice>>::iterator>>
      _CursorPublisher;
  SelectionCallbacks& _Callbacks;

  std::vector<std::shared_ptr<SelectionChoice>>::iterator GetInitialSelection();
};

} // namespace frontend::curses
