#include "Wrapper.hpp"

namespace frontend::curses {

bool Wrapper::SetLayout(Region region) {
  if (!AttrView::SetLayout(region)) {
    return false;
  }

  return _View->SetLayout(region);
}

void Wrapper::DrawPrepare(const DrawContentContext& attrs) {
  _View->DrawPrepare(attrs);
  AttrView::DrawPrepare(attrs);
}

void Wrapper::DoDrawContent(const DrawContentContext& my) {
  AttrView::DoDrawContent(my);
  _View->DrawContent(my);
}

} // namespace frontend::curses
