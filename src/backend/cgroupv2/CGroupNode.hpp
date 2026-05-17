#pragma once

#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <utility>

#include "../../utils/TreeString.hpp"
#include "../events/DirectoryWatcher.hpp"
#include "../metrics/SimplePublisher.hpp"

namespace backend::cgroupv2 {

class CGroupManager;

class CGroupNode;

class CGroupNodeBase {
public:
  explicit CGroupNodeBase(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                          std::string name)
      : _Manager(manager), _Parent(parent), _Name(std::move(name)),
        _RemovingPublisher(std::make_shared<backend::metrics::SimplePublisher<bool>>(false)) {}
  ~CGroupNodeBase() = default;

  CGroupNodeBase(const CGroupNodeBase&) = delete;
  CGroupNodeBase(CGroupNodeBase&&) = delete;
  CGroupNodeBase& operator=(const CGroupNodeBase&) = delete;
  CGroupNodeBase& operator=(CGroupNodeBase&&) = delete;

  const std::string& GetName() const { return _Name; }
  std::filesystem::path GetPath() const;

  template <typename Callback>
  auto OnNodeRemoving(Callback&& callback) const -> std::shared_ptr<backend::metrics::SubscriberBase> {
    return backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<bool>>(_RemovingPublisher,
                                                                                     std::forward<Callback>(callback));
  }

protected:
  CGroupManager& _Manager;
  std::optional<std::reference_wrapper<CGroupNode>> _Parent; // Parent always exists, except for the root node
  const std::string _Name;
  std::shared_ptr<backend::metrics::SimplePublisher<bool>> _RemovingPublisher;
};

class CGroupNode : public CGroupNodeBase, public events::DirectoryWatchDescriptor {
  // Extract CGroupNodeBase class because DirectoryWatchDescriptor must be initialized after _Parent and _Name.
public:
  explicit CGroupNode(CGroupManager& manager);
  explicit CGroupNode(CGroupManager& manager, std::optional<std::reference_wrapper<CGroupNode>> parent,
                      std::string name);
  ~CGroupNode() override;
  void InitializeChildren();

  friend bool operator==(const CGroupNode& a, const CGroupNode& b) { return std::addressof(a) == std::addressof(b); }
  friend bool operator!=(const CGroupNode& a, const CGroupNode& b) { return std::addressof(a) != std::addressof(b); }

  void OnDirectoryCreateChild(const std::string& name) override;
  void OnDirectoryDeleteChild(const std::string& name) override;

  void TreeDestruct(); // Free the whole tree. Should only be called from the root node.

  const std::filesystem::file_time_type& GetCreateTime() const { return _CreateTime; }
  std::optional<std::reference_wrapper<CGroupNode>> GetParent() const { return _Parent; }

  auto GetChildren() const { return _Children | std::views::values; }

  std::list<std::reference_wrapper<const CGroupNode>> GetAncestors() const;
  std::list<std::reference_wrapper<const CGroupNode>> GetAncestorsAndSelf() const;
  std::list<utils::TreeStringPosition> GetTreePosition() const;
  utils::TreeStringPosition GetChildPosition(const CGroupNode& child) const;

  static constexpr const std::string ROOT_NAME = "root";

private:
  std::filesystem::file_time_type _CreateTime;
  std::map<const std::string, CGroupNode&> _Children;

  std::filesystem::file_time_type ReadCreateTime() const;
};

} // namespace backend::cgroupv2
