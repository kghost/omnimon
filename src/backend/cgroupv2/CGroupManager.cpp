#include "CGroupManager.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <span>

#include "CGroupNode.hpp"
#include "CGroupNodeOrder.hpp"

namespace backend::cgroupv2 {

CGroupManager::CGroupManager(events::EventLoop& loop)
    : _EventLoop(loop), _Watcher(loop), _NodeMap(), _Root(CreateNode(std::nullopt, CGroupNode::ROOT_NAME)),
      _Order(CGroupNodeOrderHierarchical(CGroupNodeOrderByTime())) {}

CGroupManager::~CGroupManager() { _Root.TreeDestruct(); }

CGroupNode& CGroupManager::CreateNode(std::optional<std::reference_wrapper<CGroupNode>> parent, std::string name) {
  auto node = std::make_unique<CGroupNode>(*this, parent, std::move(name));
  auto path = node->GetPath();
  auto [it, inserted] = _NodeMap.emplace(path, std::move(node));
  assert(inserted && "Node with the same path already exists in the manager");

  it->second->InitializeChildren();
  return *it->second;
}

void CGroupManager::OnNodePreRemove(const std::filesystem::path& path) {
  assert(_NodeMap.contains(path) && "Node does not exist in the manager");
  _NodeMap.erase(path);
}

auto UniquePtrToRef() {
  return std::views::transform([](std::unique_ptr<CGroupNode>& p) -> CGroupNode& { return *p; });
}
auto UniquePtrToRefWrapper() {
  return std::views::transform([](std::unique_ptr<CGroupNode>& p) -> std::reference_wrapper<CGroupNode> { return *p; });
}

CGroupNode& CGroupManager::MoveCursor(CGroupNode& node, ssize_t offset) {
  if (offset == 0) {
    return node;
  } else if (offset > 0) {
    std::vector<std::reference_wrapper<CGroupNode>> result(offset + 1, _Root);
    auto [_, end] = std::ranges::partial_sort_copy(
        _NodeMap | std::views::values | UniquePtrToRef() |
            std::views::filter([&](const auto& p) { return std::not_fn(_Order)(p, node); }),
        result, _Order);
    return end == result.begin() ? node : (end - 1)->get();
  } else {
    std::vector<std::reference_wrapper<CGroupNode>> result(-offset, _Root);
    auto [_, end] =
        std::ranges::partial_sort_copy(_NodeMap | std::views::values | UniquePtrToRef() |
                                           std::views::filter([&](const auto& p) { return _Order(p, node); }),
                                       result, std::not_fn(_Order));
    return end == result.begin() ? node : (end - 1)->get();
  }
}

std::vector<std::reference_wrapper<CGroupNode>> CGroupManager::GetTopK(ssize_t k) {
  std::vector<std::reference_wrapper<CGroupNode>> result(k, _Root);
  auto [_, end] = std::ranges::partial_sort_copy(_NodeMap | std::views::values | UniquePtrToRef(), result, _Order);
  result.erase(end, result.end());
  return result;
}

std::vector<std::reference_wrapper<CGroupNode>> CGroupManager::GetAround(CGroupNode& node, ssize_t index, ssize_t max) {
  if (_NodeMap.size() <= static_cast<size_t>(max)) {
    std::vector<std::reference_wrapper<CGroupNode>> result =
        _NodeMap | std::views::values | UniquePtrToRefWrapper() | std::ranges::to<std::vector>();
    std::ranges::sort(result, _Order);

    return result;
  } else {
    ssize_t countBefore = std::ranges::count_if(_NodeMap | std::views::values | UniquePtrToRef(),
                                                [&](CGroupNode& p) { return _Order(p, node); });
    ssize_t countAfter = static_cast<ssize_t>(_NodeMap.size()) - countBefore - 1;

    auto before = std::min(countBefore, std::max(index, max - countAfter - 1));
    auto after = std::min(countAfter, std::max(max - index - 1, max - countBefore - 1));

    std::vector<std::reference_wrapper<CGroupNode>> result(before + 1 + after, _Root);

    std::ranges::partial_sort_copy(_NodeMap | std::views::values | UniquePtrToRef() |
                                       std::views::filter([&](const auto& p) { return _Order(p, node); }),
                                   std::span(result.begin(), result.begin() + before) | std::views::reverse,
                                   std::not_fn(_Order));

    std::ranges::partial_sort_copy(_NodeMap | std::views::values | UniquePtrToRef() |
                                       std::views::filter([&](const auto& p) { return std::not_fn(_Order)(p, node); }),
                                   std::span(result.begin() + before, result.end()), _Order);

    return result;
  }
}

std::function<bool(const CGroupNode&, const CGroupNode&)> CGroupManager::GetCurrentOrder() const { return _Order; }
void CGroupManager::SetCurrentOrder(std::function<bool(const CGroupNode&, const CGroupNode&)> order) { _Order = order; }

} // namespace backend::cgroupv2
