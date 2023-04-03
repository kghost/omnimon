#pragma once

#include <memory>
#include <string>

#include "View.hpp"

namespace frontend::curses {

class AttrView : public View {
public:
  explicit AttrView() = default;
  ~AttrView() override = default;

  void SetAttr(Attrs attrs) { _Attrs = attrs; }

  bool SetLayout(Layout offset, Layout layout) override;
  void DrawPrepare(const UpdateContext& attrs) override;
  void DrawContent(const UpdateContext& attrs) override final;

protected:
  virtual void DoDrawContent(const UpdateContext& my);

private:
  void Erase(const UpdateContext& attrs, Layout offset, Layout layout) const;
  Attrs _Attrs;

  bool _ForceRedraw = false;
  bool _Invalid = true;
  bool _Shown = false;

  Layout _OldOffset;
  Layout _OldLayout;
};

} // namespace frontend::curses
