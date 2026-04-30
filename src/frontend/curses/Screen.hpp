#pragma once

#include "ProcessTree.hpp"
#include "layouts/Container.hpp"

namespace frontend::curses {

class OmniMon;

class Screen : public Container {
public:
  explicit Screen(OmniMon& mon, std::shared_ptr<ProcessTree> ps);
  ~Screen() override = default;

  bool OnKey(TermKeyCode key) override;

private:
  OmniMon& _OmniMon;
};

} // namespace frontend::curses
