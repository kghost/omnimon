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
  _TextInvalid = true;
}

void TextView::DoDrawContent(const UpdateContext& my) {
  AttrView::DoDrawContent(my);

  if (!my.Visible || (!_TextInvalid && !my.ForceRedraw)) {
    return;
  }

  attron(my.attrs);

  if (utils::StringIsAsciiPrintable(_Text)) {
    switch (_Align) {
    case Align::Left:
      mvwaddstr(my.Win, _Offset.Height, _Offset.Width, std::format("{0:<{1}.{1}s}", _Text, _Layout.Width).c_str());
      break;
    case Align::Center:
      mvwaddstr(my.Win, _Offset.Height, _Offset.Width, std::format("{0:^{1}.{1}s}", _Text, _Layout.Width).c_str());
      break;
    case Align::Right:
      mvwaddstr(my.Win, _Offset.Height, _Offset.Width, std::format("{0:>{1}.{1}s}", _Text, _Layout.Width).c_str());
      break;
    }
  } else {
    std::string text = _Text;
    DisplayLength width = utils::StringDisplayTruncate(text, _Layout.Width);
    assert(width <= _Layout.Width);
    if (width < _Layout.Width) {
      DisplayLength missing = _Layout.Width - width;
      switch (_Align) {
      case Align::Left:
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width, text.c_str());
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width + width, std::string(missing, ' ').c_str());
        break;
      case Align::Center:
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width, std::string(missing / 2, ' ').c_str());
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width + missing / 2, text.c_str());
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width + missing / 2 + width,
                  std::string(missing - missing / 2, ' ').c_str());
        break;
      case Align::Right:
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width, std::string(missing, ' ').c_str());
        mvwaddstr(my.Win, _Offset.Height, _Offset.Width + missing, text.c_str());
        break;
      }
    } else {
      mvwaddstr(my.Win, _Offset.Height, _Offset.Width, text.c_str());
    }
  }

  attroff(my.attrs);

  _TextInvalid = false;
}

} // namespace frontend::curses
