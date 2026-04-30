#include "Container.hpp"

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
  for (auto& child : _Children) {
    if (child->GetView().OnKey(key)) {
      return true;
    }
  }

  return false;
}

void Container::DrawPrepare(const UpdateContext& attrs) {
  auto my = attrs.MergeWith(_Visible);
  for (auto& child : _Children) {
    child->GetView().DrawPrepare(my);
  }
}

void Container::DrawContent(const UpdateContext& attrs) {
  auto my = attrs.MergeWith(_Visible);
  for (auto& child : _Children) {
    child->GetView().DrawContent(my);
  }
}

void Container::AppendChild(std::shared_ptr<Child> child) {
  // Append a new child to the container
  _Children.emplace_back(child);
  CalculateChildLayout(child);
}

void Container::CalculateLayout() {
  _AllcatedForward = 0;
  _AllcatedBackward = 0;
  for (auto& child : _Children) {
    CalculateChildLayout(child);
  }
}

void Container::MakeSimilarTo(const Container& other) {
  if (this == &other) {
    return;
  }

  assert(_Growth == other._Growth);
  assert(_Children.size() == other._Children.size());

  for (int i = 0; i < _Children.size(); i++) {
    auto& myChild = _Children[i];
    auto& yourChild = other._Children[i];
    assert(myChild->GetArrangement() == yourChild->GetArrangement());
    assert(myChild->GetMarginBefore() == yourChild->GetMarginBefore());
    assert(myChild->GetSize() == yourChild->GetSize());
    assert(myChild->GetMarginAfter() == yourChild->GetMarginAfter());
    myChild->GetView().SetLayout(yourChild->GetView().GetOffset() - other.GetOffset() + GetOffset(),
                                 yourChild->GetView().GetLayout());
  }
}

void Container::CalculateChildLayout(std::shared_ptr<Child> child) {
  DisplayLength max = ParrelGrowth(_Growth, _Layout);
  Layout childSize = _Layout;
  Layout childOffset = _Offset;

  auto size = child->GetSize();
  auto marginBefore = child->GetMarginBefore();
  auto marginAfter = child->GetMarginAfter();
  auto arrangement = child->GetArrangement();

  if (size <= 0 || (marginBefore + size + marginAfter + _AllcatedForward + _AllcatedBackward > max)) {
    child->GetView().SetLayout({0, 0}, {0, 0});
    return;
  }

  switch (arrangement) {
  case ChildArrangement::ArrangementType::Forward:
    // Allocate space at the begin of the container
    ParrelGrowth(_Growth, childSize) = size;
    ParrelGrowth(_Growth, childOffset) += _AllcatedForward + marginBefore;
    _AllcatedForward += marginBefore + size + marginAfter;

    child->GetView().SetLayout(childOffset, childSize);
    break;
  case ChildArrangement::ArrangementType::Backward:
    // Allocate space at the end of the container
    ParrelGrowth(_Growth, childSize) = size;
    ParrelGrowth(_Growth, childOffset) += max - (_AllcatedBackward + size + marginBefore);
    _AllcatedBackward += marginBefore + size + marginAfter;

    child->GetView().SetLayout(childOffset, childSize);
    break;
  case ChildArrangement::ArrangementType::FillRest:
    // Allocate rest for the last child of the container
    ParrelGrowth(_Growth, childSize) = max - (_AllcatedForward + _AllcatedBackward + marginBefore + marginAfter);
    ParrelGrowth(_Growth, childOffset) += _AllcatedForward + marginBefore;
    _AllcatedForward = max - _AllcatedBackward;

    child->GetView().SetLayout(childOffset, childSize);
    break;
  }
}

} // namespace frontend::curses
