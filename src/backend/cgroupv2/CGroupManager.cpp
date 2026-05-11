#include "CGroupManager.hpp"

#include <cassert>

#include "CGroupNode.hpp"

namespace backend::cgroupv2 {

CGroupManager::CGroupManager(events::EventLoop& loop)
    : _EventLoop(loop), _Watcher(loop), _Root(std::make_shared<CGroupNode>(*this)) {}

void CGroupManager::OnNodeAdded(CGroupNode& node) {
  assert(_NodeMap.find(node.GetPath()) == _NodeMap.end());
  _NodeMap.emplace(node.GetPath(), node);
}

void CGroupManager::OnNodePreRemove(CGroupNode& node) {
  assert(_NodeMap.find(node.GetPath()) != _NodeMap.end());
  _NodeMap.erase(node.GetPath());
}

} // namespace backend::cgroupv2
