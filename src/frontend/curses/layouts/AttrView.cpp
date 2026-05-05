#include "AttrView.hpp"

namespace frontend::curses {

bool AttrView::SetLayout(Region region) {
  if (View::SetLayout(region)) {
    _Invalid = true;
    return true;
  } else {
    return false;
  }
}

void AttrView::DrawPrepare(const DrawContentContext& attrs) {
  if (!_Shown) {
    return;
  }

  if (attrs.ForceRedraw || _Invalid) {
    Erase(attrs, _OldRegion);
    _OldRegion = _Region;

    _ForceRedraw = true;
    _Invalid = false;
    _Shown = false;
  }
}

void AttrView::DrawContent(const DrawContentContext& attrs) {
  auto my = attrs.MergeWith(_Visible, _Attrs);
  my.ForceRedraw |= _ForceRedraw;
  _ForceRedraw = false;

  DoDrawContent(my);
}

void AttrView::DoDrawContent(const DrawContentContext& my) {
  if (!my.Visible || (_Shown && !my.ForceRedraw)) {
    return;
  }

  Erase(my, _Region);
  _OldRegion = _Region;

  _Shown = true;
}

void AttrView::Erase(const DrawContentContext& attrs, Region region) const {
  attron(attrs.attrs);
  for (DisplayLength i = 0; i < region._Size.Height; ++i) {
    mvwaddstr(attrs.Win, region._Offset.Height + i, region._Offset.Width, std::string(region._Size.Width, ' ').c_str());
  }
  attroff(attrs.attrs);
}

} // namespace frontend::curses
