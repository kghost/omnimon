#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <ranges>
#include <string>

#include "../events/DirectoryWatcher.hpp"

namespace backend::cgroupv2 {

class CGroupManager;

class CGroupNode;

class CGroupNodeBase {
public:
  explicit CGroupNodeBase(CGroupManager& manager, CGroupNode* parent, std::string name)
      : _Manager(manager), _Parent(parent), _Name(std::move(name)) {}

  const std::string& GetName() const { return _Name; }
  std::filesystem::path GetPath() const;

protected:
  CGroupManager& _Manager;
  CGroupNode* _Parent; // Parent always exists, except for the root node where it is set to nullptr.
  const std::string _Name;
  bool _RemovedFromTree = false;
};

class CGroupNode : public CGroupNodeBase, public events::DirectoryWatchDescriptor {
  // Extract CGroupNodeBase class because DirectoryWatchDescriptor must be initialized after _Parent and _Name.
public:
  explicit CGroupNode(CGroupManager& manager);
  explicit CGroupNode(CGroupManager& manager, CGroupNode* parent, std::string name);
  ~CGroupNode() override;

  const std::filesystem::file_time_type& GetCreateTime() const { return _CreateTime; }

  auto GetChildren() const { return _Children | std::views::values; }

  void OnDirectoryCreateChild(const std::string& name) override;
  void OnDirectoryDeleteChild(const std::string& name) override;

  void TreeDestruct(); // Free the whole tree. Should only be called from the root node.

private:
  void InitializeChildren();
  void NodeRemovedFromTree();

  std::filesystem::file_time_type _CreateTime;
  std::map<const std::string, std::shared_ptr<CGroupNode>> _Children;

  std::filesystem::file_time_type ReadCreateTime() const;
};

} // namespace backend::cgroupv2
