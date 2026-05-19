#include "CGroupNode.hpp"

#include <cassert>

#include "CGroupManager.hpp"

namespace backend::cgroupv2 {

CGroupNodeBase::CGroupNodeBase(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                               const std::string& name)
    : utils::TreeNodeMixin<CGroupManager, const std::string, CGroupNode>(manager, parent), _Name(name) {}

CGroupNode::CGroupNode(CGroupManager& manager) : CGroupNode(manager, std::nullopt, "root") {}

CGroupNode::CGroupNode(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                       const std::string& name)
    : CGroupNodeBase(manager, parent, name), DirectoryWatchDescriptor(manager.GetWatcher(), GetPath()),
      _CreateTime(ReadCreateTime()) {
  if (parent.has_value()) {
    parent.value().get()._Children.emplace(name, *this);
  }
}

CGroupNode::~CGroupNode() {}

std::filesystem::path CGroupNodeBase::GetPath() const {
  assert(!_RemovingPublisher->GetValue() && "GetPath should not be called after the node is removed from the tree");
  if (_Parent.has_value()) {
    return _Parent->get().GetPath() / _Name;
  } else {
    return std::filesystem::path("/sys/fs/cgroup");
  }
}

void CGroupNode::OnDirectoryCreateChild(const std::string& name) {
  assert(!_Children.contains(name));
  _Manager.CreateNode(*this, name);
}

void CGroupNode::OnDirectoryDeleteChild(const std::string& name) {
  auto it = _Children.find(name);
  assert(it != _Children.end());
  it->second.NodeDetach();
  _Children.erase(it);
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
      if (existing == _Children.end()) {
        _Manager.CreateNode(*this, name);
      }
    }
  } catch (const std::filesystem::filesystem_error&) {
    // If the directory cannot be enumerated, leave children empty.
  }
}

void CGroupNode::TreeDestruct() {
  assert(!_RemovingPublisher->GetValue() && "TreeDestruct should only be called when manager destructs the whole tree");
  for (auto& [name, child] : _Children) {
    child.TreeDestruct();
  }
  _Children.clear();
  NodeDetach();
}

void CGroupNode::NodeDetach() {
  assert(!_RemovingPublisher->GetValue() && "NodeDetach should only be called once for each node");
  assert(_Children.empty() && "CGroup node should have no children when it is removed from the tree");
  _RemovingPublisher->Update(true);
  if (_Parent.has_value()) {
    ClearParent();
  }
  _Manager.DeleteNode(*this);
}

std::chrono::system_clock::time_point CGroupNode::ReadCreateTime() const {
  auto path = GetPath();
  try {
    auto sys = std::chrono::file_clock::to_sys(std::filesystem::last_write_time(path));
    return std::chrono::time_point_cast<std::chrono::system_clock::duration>(sys);
  } catch (const std::filesystem::filesystem_error&) {
    return std::chrono::system_clock::time_point::min();
  }
}

} // namespace backend::cgroupv2
