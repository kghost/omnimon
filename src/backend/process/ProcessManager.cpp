#include "ProcessManager.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <ranges>

#include "../../utils/Lazy.hpp"
#include "Process.hpp"
#include "ProcessOrder.hpp"
#include "Types.hpp"

namespace backend::process {

const std::filesystem::path ProcessManager::PROC_PATH{"/proc"};

ProcessManager::ProcessManager()
    : TreeManagerMixin<ProcessManager, Process>(ProcessOrderByHierarchical(ProcessOrderByPid())) {}

std::optional<std::reference_wrapper<Process>> ProcessManager::GetProcess(PidType pid) {
  auto it = _ProcessCache.find(pid);
  if (it != _ProcessCache.end()) {
    return *it->second;
  }
  return std::nullopt;
}

void ProcessManager::UpdateList() {
  auto SkipNonePidDir = [](const std::filesystem::directory_entry& entry) {
    if (!entry.is_directory()) {
      return true;
    }

    std::string filename = entry.path().filename();
    if (!std::all_of(filename.begin(), filename.end(), ::isdigit)) {
      return true;
    }

    return false;
  };

  std::set<PidType> pids;

  for (auto& dir : std::filesystem::directory_iterator(PROC_PATH)) {
    if (!SkipNonePidDir(dir)) {
      PidType pid = std::stoi(dir.path().filename().string());
      pids.emplace(pid);
      _ProcessCache.try_emplace(
          pid, utils::Lazy<std::unique_ptr<Process>>([&] { return std::make_unique<Process>(*this, pid); }));
    }
  }

  for (auto& [_, proc] : _ProcessCache) {
    proc.get()->ParseStatFile(*this);
  }

  std::erase_if(_ProcessCache, [](auto& proc) { return !proc.second->Exists(); });

  for (auto& [_, proc] : _ProcessCache) {
    auto oldParentOpt = proc->GetParent();
    auto pid = proc->GetPid();
    auto ppid = proc->GetPPid();
    if (ppid != 0 && pid != ppid) {
      auto setNewParent = [&]() {
        auto it = _ProcessCache.find(ppid);
        if (it != _ProcessCache.end()) {
          proc->SetParent(*it->second);
        }
      };

      if (oldParentOpt.has_value()) {
        auto& oldParent = oldParentOpt.value().get();
        if (oldParent.GetPid() != ppid) {
          proc->ClearParent();
          setNewParent();
        }
      } else {
        setNewParent();
      }
    } else {
      if (oldParentOpt.has_value()) {
        proc->ClearParent();
      }
    }
  }
}

} // namespace backend::process
