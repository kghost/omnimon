#include "Wrapper.hpp"

namespace frontend::curses {

bool Wrapper::SetLayout(Layout offset, Layout layout) {
  if (!AttrView::SetLayout(offset, layout)) {
    return false;
  }

  return _View->SetLayout(offset, layout);
}

void Wrapper::DrawPrepare(const UpdateContext& attrs) {
  _View->DrawPrepare(attrs);
  AttrView::DrawPrepare(attrs);
}

void Wrapper::DoDrawContent(const UpdateContext& my) {
  AttrView::DoDrawContent(my);
  _View->DrawContent(my);
}

} // namespace frontend::curses
