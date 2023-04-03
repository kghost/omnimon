#include "Process.hpp"

namespace frontend::curses {

Process::Process(const std::filesystem::path& dir) : backend::process::Process(dir) {}

Process::~Process() {
  if (_Parent) {
    _Parent->RemoveChild(GetPid());
  }
}

std::list<Process::ChildPosition> Process::GetTreePosition(std::shared_ptr<Process> me) {
  std::list<ChildPosition> result;
  for (auto [parent, current] = std::tuple{me->_Parent, me}; parent; current = parent, parent = parent->_Parent) {
    result.push_front(parent->GetChildPosition(current));
  }
  return result;
}

Process::ChildPosition Process::GetChildPosition(std::shared_ptr<const Process> child) const {
  if (_Children.rbegin()->second.lock() == child) {
    return ChildPosition::Last;
  } else {
    return ChildPosition::NotLast;
  }
}

std::list<std::shared_ptr<Process>> Process::GetAncestors(std::shared_ptr<Process> p) {
  std::list<std::shared_ptr<Process>> ancestors;
  while (p) {
    ancestors.push_front(p);
    p = p->GetParent();
  }
  return ancestors;
}

} // namespace frontend::curses
