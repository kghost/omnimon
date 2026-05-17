#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../events/DirectoryWatcher.hpp"
#include "../events/Events.hpp"

namespace backend::cgroupv2 {

class CGroupNode;

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
  void OnNodePreRemove(const std::filesystem::path& path);

  std::function<bool(const CGroupNode&, const CGroupNode&)> GetCurrentOrder() const;
  void SetCurrentOrder(std::function<bool(const CGroupNode&, const CGroupNode&)> order);

private:
  events::EventLoop& _EventLoop;
  events::DirectoryWatcher _Watcher;
  std::map<std::filesystem::path, std::unique_ptr<CGroupNode>> _NodeMap;
  CGroupNode& _Root;
  std::function<bool(const CGroupNode&, const CGroupNode&)> _Order;

  std::tuple<std::filesystem::path, std::unique_ptr<CGroupNode>>
  NewNode(std::optional<std::reference_wrapper<CGroupNode>> parent, std::string name);
};

} // namespace backend::cgroupv2
