#include "CGroupNodeOrder.hpp"

#include <algorithm>
#include <cassert>

#include "CGroupNode.hpp"

namespace backend::cgroupv2 {

bool CGroupNodeOrderByTime::operator()(const CGroupNode& a, const CGroupNode& b) const {
  assert(a.GetParent() == b.GetParent());
  return a.GetCreateTime() < b.GetCreateTime();
}

bool CGroupNodeOrderHierarchical::operator()(const CGroupNode& a, const CGroupNode& b) const {
  return std::ranges::lexicographical_compare(a.GetAncestorsAndSelf(), b.GetAncestorsAndSelf(), _SameLevelOrder);
}

} // namespace backend::cgroupv2
