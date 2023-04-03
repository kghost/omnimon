#include "AttrView.hpp"

namespace frontend::curses {

bool AttrView::SetLayout(Layout offset, Layout layout) {
  if (View::SetLayout(offset, layout)) {
    _Invalid = true;
    return true;
  } else {
    return false;
  }
}

void AttrView::DrawPrepare(const UpdateContext& attrs) {
  if (!_Shown) {
    return;
  }

  if (attrs.ForceRedraw || _Invalid) {
    Erase(attrs, _OldOffset, _OldLayout);
    _OldOffset = _Offset;
    _OldLayout = _Layout;

    _ForceRedraw = true;
    _Invalid = false;
    _Shown = false;
  }
}

void AttrView::DrawContent(const UpdateContext& attrs) {
  auto my = attrs.MergeWith(_Visible, _Attrs);
  my.ForceRedraw |= _ForceRedraw;
  _ForceRedraw = false;

  DoDrawContent(my);
}

void AttrView::DoDrawContent(const UpdateContext& my) {
  if (!my.Visible || (_Shown && !my.ForceRedraw)) {
    return;
  }

  Erase(my, _Offset, _Layout);
  _OldOffset = _Offset;
  _OldLayout = _Layout;

  _Shown = true;
}

void AttrView::Erase(const UpdateContext& attrs, Layout offset, Layout layout) const {
  attron(attrs.attrs);
  for (DisplayLength i = 0; i < layout.Height; ++i) {
    mvwaddstr(attrs.Win, offset.Height + i, offset.Width, std::string(layout.Width, ' ').c_str());
  }
  attroff(attrs.attrs);
}

} // namespace frontend::curses
