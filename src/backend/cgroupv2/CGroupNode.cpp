#include "CGroupNode.hpp"

#include <cassert>

#include "CGroupManager.hpp"

namespace backend::cgroupv2 {

std::filesystem::path CGroupNodeBase::GetPath() const {
  assert(!_RemovedFromTree && "GetPath should not be called after the node is removed from the tree");
  if (_Parent != nullptr) {
    return _Parent->GetPath() / _Name;
  } else {
    return std::filesystem::path("/sys/fs/cgroup");
  }
}

CGroupNode::CGroupNode(CGroupManager& manager) : CGroupNode(manager, nullptr, "root") {}

CGroupNode::CGroupNode(CGroupManager& manager, CGroupNode* parent, std::string name)
    : CGroupNodeBase(manager, parent, std::move(name)), DirectoryWatchDescriptor(manager.GetWatcher(), GetPath()),
      _CreateTime(ReadCreateTime()) {
  _Manager.OnNodeAdded(*this);
  InitializeChildren();
}

CGroupNode::~CGroupNode() {
  assert(_RemovedFromTree && "CGroupNode should be removed from the tree before it is destructed");
}

void CGroupNode::OnDirectoryCreateChild(const std::string& name) {
  assert(!_Children.contains(name));
  _Children.emplace(name, std::make_shared<CGroupNode>(_Manager, this, name));
}

void CGroupNode::OnDirectoryDeleteChild(const std::string& name) {
  assert(_Children.contains(name));
  _Children.erase(name);
}

void CGroupNode::InitializeChildren() {
  std::filesystem::path path = GetPath();

  try {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      if (!entry.is_directory()) {
        continue;
      }

      auto name = entry.path().filename().string();
      auto existing = _Children.find(name);
      if (existing != _Children.end()) {
        _Children.emplace(name, existing->second);
      } else {
        _Children.emplace(name, std::make_shared<CGroupNode>(_Manager, this, name));
      }
    }
  } catch (const std::filesystem::filesystem_error&) {
    // If the directory cannot be enumerated, leave children empty.
  }
}

void CGroupNode::NodeRemovedFromTree() {
  assert(!_RemovedFromTree && "NodeRemovedFromTree should only be called once for each node");
  assert(_Children.empty() && "CGroup node should have no children when it is removed from the tree");
  _Manager.OnNodePreRemove(*this);
  _RemovedFromTree = true;
}

void CGroupNode::TreeDestruct() {
  assert(!_RemovedFromTree && "TreeDestruct should only be called when manager destructs the whole tree");
  for (auto& [name, child] : _Children) {
    child->TreeDestruct();
  }
  _Children.clear();
  NodeRemovedFromTree();
}

std::filesystem::file_time_type CGroupNode::ReadCreateTime() const {
  auto path = GetPath();
  try {
    return std::filesystem::last_write_time(path);
  } catch (const std::filesystem::filesystem_error&) {
    return std::filesystem::file_time_type::min();
  }
}

} // namespace backend::cgroupv2
