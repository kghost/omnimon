#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <ranges>
#include <string>

#include "../../utils/BackendTree.hpp"
#include "../events/DirectoryWatcher.hpp"

namespace backend::cgroupv2 {

class CGroupManager;

class CGroupNode;

class CGroupNodeBase : public utils::TreeNodeMixin<CGroupManager, const std::string, CGroupNode> {
public:
  explicit CGroupNodeBase(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                          const std::string& name);
  ~CGroupNodeBase() = default;

  CGroupNodeBase(const CGroupNodeBase&) = delete;
  CGroupNodeBase(CGroupNodeBase&&) = delete;
  CGroupNodeBase& operator=(const CGroupNodeBase&) = delete;
  CGroupNodeBase& operator=(CGroupNodeBase&&) = delete;

  operator const std::string() const { return _Name; }
  const std::string& GetName() const { return _Name; }
  std::filesystem::path GetPath() const;

protected:
  const std::string _Name;
};

class CGroupNode : public CGroupNodeBase, public events::DirectoryWatchDescriptor {
  // Extract CGroupNodeBase class because DirectoryWatchDescriptor must be initialized after _Parent and _Name.
public:
  explicit CGroupNode(CGroupManager& manager);
  explicit CGroupNode(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                      const std::string& name);
  ~CGroupNode() override;
  void InitializeChildren();

  friend bool operator==(const CGroupNode& a, const CGroupNode& b) { return std::addressof(a) == std::addressof(b); }
  friend bool operator!=(const CGroupNode& a, const CGroupNode& b) { return std::addressof(a) != std::addressof(b); }

  void OnDirectoryCreateChild(const std::string& name) override;
  void OnDirectoryDeleteChild(const std::string& name) override;

  void TreeDestruct(); // Free the whole tree. Should only be called from the root node.
  void NodeDetach();   // Detach the node from the tree. this will delete the node

  std::chrono::system_clock::time_point GetCreateTime() const { return _CreateTime; }

  auto GetChildren() const { return _Children | std::views::values; }

  static constexpr const std::string ROOT_NAME = "root";

private:
  std::chrono::system_clock::time_point _CreateTime;
  std::chrono::system_clock::time_point ReadCreateTime() const;
};

} // namespace backend::cgroupv2
