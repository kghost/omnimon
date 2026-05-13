#include "CGroupManager.hpp"

#include <cassert>

#include "CGroupNode.hpp"

namespace backend::cgroupv2 {

CGroupManager::CGroupManager(events::EventLoop& loop)
    : _EventLoop(loop), _Watcher(loop), _Root(std::make_shared<CGroupNode>(*this)) {}

CGroupManager::~CGroupManager() { _Root->TreeDestruct(); }

void CGroupManager::OnNodeAdded(CGroupNode& node) {
  assert(!_NodeMap.contains(node.GetPath()) && "Node with the same path already exists in the manager");
  _NodeMap.emplace(node.GetPath(), node);
}

void CGroupManager::OnNodePreRemove(CGroupNode& node) {
  assert(_NodeMap.contains(node.GetPath()) && "Node does not exist in the manager");
  _NodeMap.erase(node.GetPath());
}

} // namespace backend::cgroupv2
