#pragma once

#include <functional>

namespace backend::cgroupv2 {

class CGroupNode;

class CGroupNodeOrderByTime {
public:
  bool operator()(const CGroupNode& a, const CGroupNode& b) const;
};

class CGroupNodeOrderHierarchical {
public:
  explicit CGroupNodeOrderHierarchical(std::function<bool(const CGroupNode&, const CGroupNode&)> order)
      : _SameLevelOrder(order) {}

  bool operator()(const CGroupNode& a, const CGroupNode& b) const;

private:
  std::function<bool(const CGroupNode&, const CGroupNode&)> _SameLevelOrder;
};

} // namespace backend::cgroupv2
