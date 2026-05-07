#include "Window.hpp"

#include <cassert>

#include "views/View.hpp"

namespace frontend::curses {

Window::Window(Region region, std::shared_ptr<WindowClient> client)
    : _Win(newwin(region._Size.Height, region._Size.Width, region._Offset.Height, region._Offset.Width)),
      _Client(client) {
  keypad(_Win, true);
  _Client->GetView()->SetLayout({{0, 0}, region._Size});
}

Window::~Window() { delwin(_Win); }

void Window::SetLayout(Region region) {
  mvwin(_Win, region._Offset.Height, region._Offset.Width);
  wresize(_Win, region._Size.Height, region._Size.Width);
  _Client->GetView()->SetLayout({{0, 0}, region._Size});
}

void Window::Draw() {
  _Client->GetView()->DrawPrepare(DrawPrepareContext(_Win));
  _Client->GetView()->DrawContent(DrawContentContext(_Win));
  wnoutrefresh(_Win);
}

bool Window::OnKey(TermKeyCode key) { return _Client->GetView()->OnKey(key); }

} // namespace frontend::curses
