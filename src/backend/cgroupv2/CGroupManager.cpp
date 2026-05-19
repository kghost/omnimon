#include "CGroupManager.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <optional>

#include "CGroupNode.hpp"
#include "CGroupNodeOrder.hpp"

namespace backend::cgroupv2 {

CGroupManager::CGroupManager(events::EventLoop& loop)
    : utils::TreeManagerMixin<CGroupManager, CGroupNode>(CGroupNodeOrderHierarchical(CGroupNodeOrderByTime())),
      _EventLoop(loop), _Watcher(loop), _NodeSet(), _Root(CreateNode(std::nullopt, CGroupNode::ROOT_NAME)) {}

CGroupManager::~CGroupManager() { _Root.TreeDestruct(); }

CGroupNode& CGroupManager::CreateNode(std::optional<std::reference_wrapper<CGroupNode>> parent,
                                      const std::string& name) {
  auto node = std::make_unique<CGroupNode>(*this, parent, name);
  auto [it, inserted] = _NodeSet.emplace(std::move(node));
  assert(inserted && "Node with the same path already exists in the manager");
  (*it)->InitializeChildren();
  return **it;
}

void CGroupManager::DeleteNode(const CGroupNode& node) {
  assert(_NodeSet.contains(node) && "Node does not exist in the manager");
  // FIXME: _NodeSet.erase(node) if our stl supports heterogeneous lookup
  _NodeSet.erase(_NodeSet.find(node));
}

} // namespace backend::cgroupv2
