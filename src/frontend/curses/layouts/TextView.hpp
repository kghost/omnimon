#pragma once

#include <memory>
#include <string>

#include "AttrView.hpp"

namespace frontend::curses {

class TextView : public AttrView {
public:
  enum class Align { Left, Center, Right };

  explicit TextView(Align align = Align::Left) : _Align(align) {}
  ~TextView() override = default;

  bool OnKey(TermKeyCode key) override { return false; }
  void SetText(const std::string& text);
  void DoDrawContent(const UpdateContext& my) override;

protected:
  std::string _Text;
  Align _Align;
  bool _TextInvalid = true;
};

} // namespace frontend::curses
