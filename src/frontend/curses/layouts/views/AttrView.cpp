#include "AttrView.hpp"

#include <cassert>

namespace frontend::curses {

bool AttrView::SetLayout(Region region) {
  if (View::SetLayout(region)) {
    if (_State == State::Shown || _State == State::Invalid) {
      _State = State::Moved;
    }
    return true;
  } else {
    return false;
  }
}

void AttrView::DrawPrepare(const DrawPrepareContext& attrs) {
  bool clear = false;

  switch (_State) {
  case State::Moved:
    clear = true;
    break;
  case State::Invalid:
  case State::Shown:
    if (!attrs.Visible || !_Visible) {
      clear = true;
    }
    break;
  case State::Hidden:
    break;
  }

  if (clear) {
    Erase(_OldRegion, attrs.Win, A_NORMAL);
    _State = State::Hidden;
  }
}

void AttrView::DrawContent(const DrawContentContext& attrs) {
  assert(_State != State::Moved);
  if (_Visible && (_State == State::Hidden || _State == State::Invalid)) {
    DoDrawContent(attrs.MergeWith(_Attrs));
    _OldRegion = _Region;
    _State = State::Shown;
  }
}

void AttrView::DoDrawContent(const DrawContentContext& my) { Erase(_Region, my.Win, my.Attrs); }

void AttrView::Erase(Region region, WINDOW* win, TermAttrs attrs) const {
  wattron(win, attrs);
  for (DisplayLength i = 0; i < region._Size.Height; ++i) {
    mvwaddstr(win, region._Offset.Height + i, region._Offset.Width, std::string(region._Size.Width, ' ').c_str());
  }
  wattroff(win, attrs);
}

} // namespace frontend::curses
