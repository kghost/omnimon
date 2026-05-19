#include "ProcessOrder.hpp"

#include <algorithm>
#include <cassert>

#include "Process.hpp"

namespace backend::process {

bool ProcessOrderByPid::operator()(const Process& a, const Process& b) const {
  assert(a.GetParent() == b.GetParent());
  return a.GetPid() < b.GetPid();
}

bool ProcessOrderByHierarchical::operator()(const Process& a, const Process& b) const {
  return std::ranges::lexicographical_compare(a.GetAncestorsAndSelf(), b.GetAncestorsAndSelf(), _SameLevelOrder);
}

} // namespace backend::process
