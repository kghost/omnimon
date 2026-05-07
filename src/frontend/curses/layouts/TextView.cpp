#include "TextView.hpp"

#include <cassert>
#include <format>

#include "../../../utils/StringUtils.hpp"

namespace frontend::curses {

void TextView::SetText(const std::string& text) {
  if (_Text == text) {
    return;
  }

  _Text = text;
  if (_State == AttrView::State::Shown) {
    _State = AttrView::State::Invalid;
  }
}

void TextView::DoDrawContent(const DrawContentContext& my) {
  AttrView::DoDrawContent(my);

  wattron(my.Win, my.Attrs);

  if (utils::StringIsAsciiPrintable(_Text)) {
    switch (_Align) {
    case Align::Left:
      mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width,
                std::format("{0:<{1}.{1}s}", _Text, _Region._Size.Width).c_str());
      break;
    case Align::Center:
      mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width,
                std::format("{0:^{1}.{1}s}", _Text, _Region._Size.Width).c_str());
      break;
    case Align::Right:
      mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width,
                std::format("{0:>{1}.{1}s}", _Text, _Region._Size.Width).c_str());
      break;
    }
  } else {
    std::string text = _Text;
    DisplayLength width = utils::StringDisplayTruncate(text, _Region._Size.Width);
    assert(width <= _Region._Size.Width);
    if (width < _Region._Size.Width) {
      DisplayLength missing = _Region._Size.Width - width;
      switch (_Align) {
      case Align::Left:
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width, text.c_str());
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width + width, std::string(missing, ' ').c_str());
        break;
      case Align::Center:
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width, std::string(missing / 2, ' ').c_str());
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width + missing / 2, text.c_str());
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width + missing / 2 + width,
                  std::string(missing - missing / 2, ' ').c_str());
        break;
      case Align::Right:
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width, std::string(missing, ' ').c_str());
        mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width + missing, text.c_str());
        break;
      }
    } else {
      mvwaddstr(my.Win, _Region._Offset.Height, _Region._Offset.Width, text.c_str());
    }
  }

  wattroff(my.Win, my.Attrs);
}

} // namespace frontend::curses
