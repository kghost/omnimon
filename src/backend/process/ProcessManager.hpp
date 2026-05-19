#pragma once

#include <filesystem>
#include <memory>
#include <ranges>
#include <unordered_map>

#include "../../utils/BackendTree.hpp"
#include "Types.hpp"

namespace backend::process {

class Process;

class ProcessManager : public utils::TreeManagerMixin<ProcessManager, Process> {
public:
  static const std::filesystem::path PROC_PATH;

  explicit ProcessManager();
  ~ProcessManager() = default;

  std::optional<std::reference_wrapper<Process>> GetProcess(PidType pid);

  void UpdateList();
  auto GetAllNodes() { return _ProcessCache | std::views::values; }

private:
  std::unordered_map<PidType, std::unique_ptr<Process>> _ProcessCache;
};

} // namespace backend::process
