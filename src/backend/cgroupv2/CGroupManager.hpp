#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <string>

#include "../../utils/BackendTree.hpp"
#include "../events/DirectoryWatcher.hpp"
#include "../events/Events.hpp"

namespace backend::cgroupv2 {

class CGroupNode;

struct CGroupNodeAddressCompare {
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

class CGroupManager : public utils::TreeManagerMixin<CGroupManager, CGroupNode> {
public:
  explicit CGroupManager(events::EventLoop& loop);
  ~CGroupManager();

  CGroupManager(const CGroupManager&) = delete;
  CGroupManager(CGroupManager&&) = delete;
  CGroupManager& operator=(const CGroupManager&) = delete;
  CGroupManager& operator=(CGroupManager&&) = delete;

  auto GetAllNodes() { return std::views::all(_NodeSet); }
  bool ContainsNode(const CGroupNode& node) const { return _NodeSet.contains(node); }

  events::DirectoryWatcher& GetWatcher() { return _Watcher; }
  CGroupNode& GetRoot() const { return _Root; }

  CGroupNode& CreateNode(std::optional<std::reference_wrapper<CGroupNode>> parent, const std::string& name);
  void DeleteNode(const CGroupNode& node);

private:
  events::EventLoop& _EventLoop;
  events::DirectoryWatcher _Watcher;
  std::set<std::unique_ptr<CGroupNode>, CGroupNodeAddressCompare> _NodeSet;
  CGroupNode& _Root;

  std::tuple<std::filesystem::path, std::unique_ptr<CGroupNode>>
  NewNode(std::optional<std::reference_wrapper<CGroupNode>> parent, std::string name);
};

} // namespace backend::cgroupv2
