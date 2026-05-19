#pragma once

#include <functional>

namespace backend::process {

class Process;

class ProcessOrderByPid {
public:
  bool operator()(const Process& a, const Process& b) const;
};

class ProcessOrderByHierarchical {
public:
  explicit ProcessOrderByHierarchical(std::function<bool(const Process&, const Process&)> order)
      : _SameLevelOrder(order) {}

  bool operator()(const Process& a, const Process& b) const;

private:
  std::function<bool(const Process&, const Process&)> _SameLevelOrder;
};

} // namespace backend::process
