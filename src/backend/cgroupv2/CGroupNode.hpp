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
};

class CGroupNode : public CGroupNodeBase, public events::DirectoryWatchDescriptor {
public:
  explicit CGroupNode(CGroupManager& manager);
  explicit CGroupNode(CGroupManager& manager, CGroupNode* parent, std::string name);
  ~CGroupNode() override;

  const std::filesystem::file_time_type& GetCreateTime() const { return _CreateTime; }

  auto GetChildren() const { return _Children | std::views::values; }

  void OnDirectoryCreateChild(const std::string& name) override;
  void OnDirectoryDeleteChild(const std::string& name) override;

private:
  void InitializeChildren();

  std::filesystem::file_time_type _CreateTime;
  std::map<std::string, std::shared_ptr<CGroupNode>> _Children;

  std::filesystem::file_time_type ReadCreateTime() const;
};

} // namespace backend::cgroupv2
