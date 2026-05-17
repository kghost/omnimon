#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Types.hpp"

namespace backend::process {

class Process;

class ProcessListing {
public:
  ProcessListing() = default;
  ~ProcessListing() = default;

  void UpdateList();

  std::shared_ptr<Process> MoveCursor(std::shared_ptr<Process> current, ssize_t offset);
  std::shared_ptr<Process> GetValidAncestor(std::shared_ptr<Process> process);
  std::vector<std::shared_ptr<Process>> GetTopK(ssize_t k);
  std::vector<std::shared_ptr<Process>> GetAround(std::shared_ptr<Process> process, ssize_t index, ssize_t max);

private:
  static const std::filesystem::path _ProcPath;

  std::unordered_map<PidType, std::shared_ptr<Process>> _ProcessCache;
};

} // namespace backend::process
