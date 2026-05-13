#pragma once

#include <filesystem>
#include <map>
#include <memory>

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
  std::shared_ptr<CGroupNode> GetRoot() const { return _Root; }

  void OnNodeAdded(CGroupNode& node);
  void OnNodePreRemove(CGroupNode& node);

private:
  events::EventLoop& _EventLoop;
  events::DirectoryWatcher _Watcher;
  std::map<std::filesystem::path, std::reference_wrapper<CGroupNode>> _NodeMap;
  std::shared_ptr<CGroupNode> _Root;
};

} // namespace backend::cgroupv2
