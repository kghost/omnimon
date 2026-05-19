
#pragma once

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <ranges>

#include "../backend/metrics/SimplePublisher.hpp"
#include "TreeString.hpp"

namespace utils {

template <typename TreeManager, typename TreeNode> class TreeManagerMixin {
protected:
  TreeManagerMixin(std::function<bool(const TreeNode&, const TreeNode&)> order) : _Order(std::move(order)) {}
  ~TreeManagerMixin() = default;

public:
  TreeManagerMixin(const TreeManagerMixin&) = delete;
  TreeManagerMixin(TreeManagerMixin&&) = delete;
  TreeManagerMixin& operator=(const TreeManagerMixin&) = delete;
  TreeManagerMixin& operator=(TreeManagerMixin&&) = delete;

  static auto UniquePtrToRef() {
    return std::views::transform([](const std::unique_ptr<TreeNode>& p) -> TreeNode& { return *p; });
  }
  static auto UniquePtrToRefWrapper() {
    return std::views::transform(
        [](const std::unique_ptr<TreeNode>& p) -> std::reference_wrapper<TreeNode> { return *p; });
  }

  template <typename Self> TreeNode& MoveCursor(this Self& self, TreeNode& node, ssize_t offset) {
    if (offset == 0) {
      return node;
    } else if (offset > 0) {
      std::vector<std::reference_wrapper<TreeNode>> result(offset + 1, node);
      auto [_, end] =
          std::ranges::partial_sort_copy(self.GetAllNodes() | UniquePtrToRef() | std::views::filter([&](const auto& p) {
                                           return std::not_fn(self._Order)(p, node);
                                         }),
                                         result, self._Order);
      return end == result.begin() ? node : (end - 1)->get();
    } else {
      std::vector<std::reference_wrapper<TreeNode>> result(-offset, node);
      auto [_, end] =
          std::ranges::partial_sort_copy(self.GetAllNodes() | UniquePtrToRef() |
                                             std::views::filter([&](const auto& p) { return self._Order(p, node); }),
                                         result, std::not_fn(self._Order));
      return end == result.begin() ? node : (end - 1)->get();
    }
  }

  template <typename Self> std::vector<std::reference_wrapper<TreeNode>> GetTopK(this Self& self, ssize_t k) {
    auto allNodes = self.GetAllNodes() | UniquePtrToRef();
    if (std::ranges::empty(allNodes)) {
      return {};
    }
    auto& dummy = *std::ranges::begin(allNodes);
    std::vector<std::reference_wrapper<TreeNode>> result(k, dummy);
    auto [_, end] = std::ranges::partial_sort_copy(allNodes, result, self._Order);
    result.erase(end, result.end());
    return result;
  }

  template <typename Self>
  std::vector<std::reference_wrapper<TreeNode>> GetAround(this Self& self, TreeNode& node, ssize_t index, ssize_t max) {
    if (self.GetAllNodes().size() <= static_cast<size_t>(max)) {
      std::vector<std::reference_wrapper<TreeNode>> result =
          self.GetAllNodes() | UniquePtrToRefWrapper() | std::ranges::to<std::vector>();
      std::ranges::sort(result, self._Order);

      return result;
    } else {
      ssize_t countBefore = std::ranges::count_if(self.GetAllNodes() | UniquePtrToRef(),
                                                  [&](TreeNode& p) { return self._Order(p, node); });
      ssize_t countAfter = static_cast<ssize_t>(self.GetAllNodes().size()) - countBefore - 1;

      auto before = std::min(countBefore, std::max(index, max - countAfter - 1));
      auto after = std::min(countAfter, std::max(max - index - 1, max - countBefore - 1));

      std::vector<std::reference_wrapper<TreeNode>> result(before + 1 + after, node);

      std::ranges::partial_sort_copy(self.GetAllNodes() | UniquePtrToRef() |
                                         std::views::filter([&](const auto& p) { return self._Order(p, node); }),
                                     std::span(result.begin(), result.begin() + before) | std::views::reverse,
                                     std::not_fn(self._Order));

      std::ranges::partial_sort_copy(self.GetAllNodes() | UniquePtrToRef() | std::views::filter([&](const auto& p) {
                                       return std::not_fn(self._Order)(p, node);
                                     }),
                                     std::span(result.begin() + before, result.end()), self._Order);

      return result;
    }
  }

  std::function<bool(const TreeNode&, const TreeNode&)> GetCurrentOrder() const { return _Order; }
  void SetCurrentOrder(std::function<bool(const TreeNode&, const TreeNode&)> order) { _Order = std::move(order); }

protected:
  std::function<bool(const TreeNode&, const TreeNode&)> _Order;
};

template <typename TreeManager, typename TreeNodeKey, typename TreeNode> class TreeNodeMixin {
public:
  explicit TreeNodeMixin(TreeManager& manager, std::optional<std::reference_wrapper<TreeNode>> parent = std::nullopt)
      : _Manager(manager), _Parent(parent),
        _RemovingPublisher(std::make_shared<backend::metrics::SimplePublisher<bool>>(false)) {}
  ~TreeNodeMixin() {
    assert(_RemovingPublisher->GetValue() && "TreeNode should be removed from the tree before it is destructed");
    assert(!_Parent.has_value());
    assert(_Children.empty());
  }

  std::optional<std::reference_wrapper<TreeNode>> GetParent() const { return _Parent; }

  template <typename Self> void SetParent(this Self& self, TreeNode& parent) {
    assert(!self._Parent.has_value());
    self._Parent = parent;
    parent._Children.emplace(self.operator TreeNodeKey(), self);
  }

  template <typename Self> void ClearParent(this Self& self) {
    assert(self._Parent.has_value());
    assert(self._Parent->get()._Children.contains(self.operator TreeNodeKey()));
    self._Parent->get()._Children.erase(self.operator TreeNodeKey());
    self._Parent = std::nullopt;
  }

  void AddChild(TreeNode& child) {
    assert(!child._Parent.has_value() && "Child is already added to another parent");
    child._Parent = *this;
    _Children.emplace(child.operator TreeNodeKey(), child);
  }

  void RemoveChild(const TreeNodeKey& key) {
    auto it = _Children.find(key);
    assert(it != _Children.end() && "Child is not a child of this node");
    it->second.get()._Parent = std::nullopt;
    _Children.erase(it);
  }

  void RemoveChild(TreeNode& child) {
    auto it = _Children.find(child.operator TreeNodeKey());
    assert(it != _Children.end() && "Child is not a child of this node");
    it->second.get()._Parent = std::nullopt;
    _Children.erase(it);
  }

  std::list<std::reference_wrapper<const TreeNode>> GetAncestors() const {
    std::list<std::reference_wrapper<const TreeNode>> result;
    auto currOpt = GetParent();
    while (currOpt.has_value()) {
      result.push_front(currOpt->get());
      currOpt = currOpt->get().GetParent();
    }
    return result;
  }

  template <typename Self>
  std::list<std::reference_wrapper<const TreeNode>> GetAncestorsAndSelf(this const Self& self) {
    auto result = self.GetAncestors();
    result.push_back(self);
    return result;
  }

  template <typename Self> std::list<utils::TreeStringPosition> GetTreePosition(this const Self& self) {
    auto line = self.GetAncestorsAndSelf();
    return line | std::views::pairwise_transform([&](const auto& parent, const auto& child) {
             return parent.get().GetChildPosition(child.get());
           }) |
           std::ranges::to<std::list>();
  }

  utils::TreeStringPosition GetChildPosition(const TreeNode& child) const {
    return std::ranges::none_of(_Children | std::views::values,
                                [&](const auto& c) { return _Manager.GetCurrentOrder()(child, c); })
               ? utils::TreeStringPosition::Last
               : utils::TreeStringPosition::NotLast;
  }

  template <typename Callback>
  auto OnNodeRemoving(Callback&& callback) const -> std::shared_ptr<backend::metrics::SubscriberBase> {
    return backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<bool>>(_RemovingPublisher,
                                                                                     std::forward<Callback>(callback));
  }

protected:
  TreeManager& _Manager;
  std::shared_ptr<backend::metrics::SimplePublisher<bool>> _RemovingPublisher;
  std::optional<std::reference_wrapper<TreeNode>> _Parent;
  std::map<TreeNodeKey, TreeNode&> _Children;
};

} // namespace utils
