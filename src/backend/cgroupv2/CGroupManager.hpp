#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "../events/DirectoryWatcher.hpp"
#include "../events/Events.hpp"

namespace backend::cgroupv2 {

class CGroupNode;

struct CGroupNodePathCompare {
  using is_transparent = int;
  bool operator()(const std::unique_ptr<CGroupNode>& lhs, const std::unique_ptr<CGroupNode>& rhs) const {
    return std::to_address(lhs) < std::to_address(rhs);
  }
  bool operator()(const std::unique_ptr<CGroupNode>& lhs, const CGroupNode& rhs) const {
    return std::to_address(lhs) < std::addressof(rhs);
  }
  bool operator()(const CGroupNode& lhs, const std::unique_ptr<CGroupNode>& rhs) const {
    return std::addressof(lhs) < std::to_address(rhs);
  }
};

class CGroupManager {
public:
  explicit CGroupManager(events::EventLoop& loop);
  ~CGroupManager();

  CGroupManager(const CGroupManager&) = delete;
  CGroupManager(CGroupManager&&) = delete;
  CGroupManager& operator=(const CGroupManager&) = delete;
  CGroupManager& operator=(CGroupManager&&) = delete;

  events::DirectoryWatcher& GetWatcher() { return _Watcher; }
  CGroupNode& GetRoot() const { return _Root; }

  CGroupNode& MoveCursor(CGroupNode& current, ssize_t offset);
  std::vector<std::reference_wrapper<CGroupNode>> GetTopK(ssize_t k);
  std::vector<std::reference_wrapper<CGroupNode>> GetAround(CGroupNode& node, ssize_t index, ssize_t max);

  CGroupNode& CreateNode(std::optional<std::reference_wrapper<CGroupNode>> parent, std::string name);
  void OnNodePreRemove(const CGroupNode& node);

  std::function<bool(const CGroupNode&, const CGroupNode&)> GetCurrentOrder() const;
  void SetCurrentOrder(std::function<bool(const CGroupNode&, const CGroupNode&)> order);

private:
  events::EventLoop& _EventLoop;
  events::DirectoryWatcher _Watcher;
  std::set<std::unique_ptr<CGroupNode>, CGroupNodePathCompare> _NodeSet;
  CGroupNode& _Root;
  std::function<bool(const CGroupNode&, const CGroupNode&)> _Order;

  std::tuple<std::filesystem::path, std::unique_ptr<CGroupNode>>
  NewNode(std::optional<std::reference_wrapper<CGroupNode>> parent, std::string name);
};

} // namespace backend::cgroupv2
