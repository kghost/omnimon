#include "CGroupNode.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>

#include "CGroupManager.hpp"

namespace backend::cgroupv2 {

std::filesystem::path CGroupNodeBase::GetPath() const {
  assert(!_RemovingPublisher->GetValue() && "GetPath should not be called after the node is removed from the tree");
  if (_Parent.has_value()) {
    return _Parent->get().GetPath() / _Name;
  } else {
    return std::filesystem::path("/sys/fs/cgroup");
  }
}

CGroupNode::CGroupNode(CGroupManager& manager) : CGroupNode(manager, std::nullopt, "root") {}

CGroupNode::CGroupNode(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                       std::string name)
    : CGroupNodeBase(manager, parent, std::move(name)), DirectoryWatchDescriptor(manager.GetWatcher(), GetPath()),
      _CreateTime(ReadCreateTime()) {}

CGroupNode::~CGroupNode() {
  assert(_RemovingPublisher->GetValue() && "CGroupNode should be removed from the tree before it is destructed");
}

void CGroupNode::OnDirectoryCreateChild(const std::string& name) {
  assert(!_Children.contains(name));
  _Children.emplace(name, _Manager.CreateNode(*this, name));
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
        _Children.emplace(name, _Manager.CreateNode(*this, name));
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
  auto path = GetPath();
  _RemovingPublisher->Update(true);
  _Manager.OnNodePreRemove(path);
}

std::chrono::system_clock::time_point CGroupNode::ReadCreateTime() const {
  auto path = GetPath();
  try {
    return std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(path));
  } catch (const std::filesystem::filesystem_error&) {
    return std::chrono::system_clock::time_point::min();
  }
}

std::list<std::reference_wrapper<const CGroupNode>> CGroupNode::GetAncestors() const {
  std::list<std::reference_wrapper<const CGroupNode>> result;
  auto currOpt = GetParent();
  while (currOpt.has_value()) {
    result.push_front(currOpt->get());
    currOpt = currOpt->get().GetParent();
  }
  return result;
}

std::list<std::reference_wrapper<const CGroupNode>> CGroupNode::GetAncestorsAndSelf() const {
  auto result = GetAncestors();
  result.push_back(*this);
  return result;
}

std::list<utils::TreeStringPosition> CGroupNode::GetTreePosition() const {
  auto line = GetAncestorsAndSelf();
  return line | std::views::pairwise_transform([this](const auto& parent, const auto& child) {
           return parent.get().GetChildPosition(child.get());
         }) |
         std::ranges::to<std::list>();
}

utils::TreeStringPosition CGroupNode::GetChildPosition(const CGroupNode& child) const {
  return std::ranges::none_of(_Children | std::views::values,
                              [&](const auto& c) { return _Manager.GetCurrentOrder()(child, c); })
             ? utils::TreeStringPosition::Last
             : utils::TreeStringPosition::NotLast;
}

} // namespace backend::cgroupv2
