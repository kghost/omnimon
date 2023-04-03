#include "Container.hpp"

#include <algorithm>
#include <cassert>

namespace frontend::curses {

bool Container::SetLayout(Layout offset, Layout layout) {
  if (!View::SetLayout(offset, layout)) {
    return false;
  }

  CalculateLayout();
  return true;
}

bool Container::OnKey(TermKeyCode key) {
  for (auto& [view, context] : _Children) {
    if (view->OnKey(key)) {
      return true;
    }
  }

  return false;
}

void Container::DrawPrepare(const UpdateContext& attrs) {
  auto my = attrs.MergeWith(_Visible);
  for (auto& [view, context] : _Children) {
    view->DrawPrepare(my);
  }
}

void Container::DrawContent(const UpdateContext& attrs) {
  auto my = attrs.MergeWith(_Visible);
  for (auto& [view, context] : _Children) {
    view->DrawContent(my);
  }
}

void Container::AppendChild(std::shared_ptr<View> view, std::shared_ptr<const Context> context) {
  // Append a new child to the container
  _Children.emplace_back(view, context);
  CalculateChildLayout(view, context);
}

void Container::CalculateLayout() {
  _AllcatedForward = 0;
  _AllcatedBackward = 0;
  for (auto& [context, view] : _Children) {
    CalculateChildLayout(context, view);
  }
}

void Container::MakeSimilarTo(const Container& other) {
  if (this == &other) {
    return;
  }

  assert(Growth == other.Growth);
  assert(_Children.size() == other._Children.size());

  for (int i = 0; i < _Children.size(); i++) {
    auto& [myView, myContext] = _Children[i];
    auto& [yourView, yourContext] = other._Children[i];
    assert(myContext->GetArrangement() == yourContext->GetArrangement());
    assert(myContext->GetMarginBefore() == yourContext->GetMarginBefore());
    assert(myContext->GetSize() == yourContext->GetSize());
    assert(myContext->GetMarginAfter() == yourContext->GetMarginAfter());
    myView->SetLayout(yourView->GetOffset() - other.GetOffset() + GetOffset(), yourView->GetLayout());
  }
}

void Container::CalculateChildLayout(std::shared_ptr<View> view, std::shared_ptr<const Context> context) {
  DisplayLength max = ParrelGrowth(_Layout);
  Layout childSize = _Layout;
  Layout childOffset = _Offset;

  auto size = context->GetSize();
  auto marginBefore = context->GetMarginBefore();
  auto marginAfter = context->GetMarginAfter();
  auto arrangement = context->GetArrangement();

  if (size <= 0 || (marginBefore + size + marginAfter + _AllcatedForward + _AllcatedBackward > max)) {
    view->SetLayout({0, 0}, {0, 0});
    return;
  }

  switch (arrangement) {
  case ArrangementType::Forward:
    // Allocate space at the begin of the container
    ParrelGrowth(childSize) = size;
    ParrelGrowth(childOffset) += _AllcatedForward + marginBefore;
    _AllcatedForward += marginBefore + size + marginAfter;

    view->SetLayout(childOffset, childSize);
    break;
  case ArrangementType::Backward:
    // Allocate space at the end of the container
    ParrelGrowth(childSize) = size;
    ParrelGrowth(childOffset) += max - (_AllcatedBackward + size + marginBefore);
    _AllcatedBackward += marginBefore + size + marginAfter;

    view->SetLayout(childOffset, childSize);
    break;
  case ArrangementType::FillRest:
    // Allocate rest for the last child of the container
    ParrelGrowth(childSize) = max - (_AllcatedForward + _AllcatedBackward + marginBefore + marginAfter);
    ParrelGrowth(childOffset) += _AllcatedForward + marginBefore;
    _AllcatedForward = max - _AllcatedBackward;

    view->SetLayout(childOffset, childSize);
    break;
  }
}

} // namespace frontend::curses
