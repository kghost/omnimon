#pragma once

#include "View.hpp"

namespace frontend::curses {

// AttrView is a View that has a background attribute and can be invalidated.
//
// Each space on the screen is covered by at most one AttrView, so that when
// DrawPrepare is called for AttrView, it will overwrite the space with its
// attribute, and when DrawContent is called, it will overwrite the space with
// its content. Only leaf views can be AttrView, since any parent view will be
// drawn over it.
class AttrView : public View {
public:
  explicit AttrView() = default;
  ~AttrView() override = default;

  void SetAttr(TermAttrs attrs) { _Attrs = attrs; }

  bool SetLayout(Region region) override;
  void DrawPrepare(const DrawPrepareContext& attrs) override;
  void DrawContent(const DrawContentContext& attrs) override final;

private:
  void Erase(Region region, WINDOW* win, TermAttrs attrs) const;

protected:
  virtual void DoDrawContent(const DrawContentContext& my);

  TermAttrs _Attrs = A_NORMAL;

  enum class State {
    Hidden,  // View is hidden, erase can be skipped.
    Moved,   // View has moved, erase old region, clear new region and draw.
    Invalid, // View is invalid, erase and draw.
    Shown,   // View is shown, skip erase and skip draw.
  } _State = State::Hidden;

  Region _OldRegion = {{0, 0}, {0, 0}};
};

} // namespace frontend::curses
