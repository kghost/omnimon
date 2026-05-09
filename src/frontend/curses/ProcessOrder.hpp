#pragma once

#include <list>
#include <memory>
#include <unordered_map>

#include "../../backend/process/Process.hpp"
#include "../../backend/process/ProcessListing.hpp"
#include "../../utils/StringUtils.hpp"

namespace frontend::ftxui {

class Process;

class ProcessCollection : public backend::process::ProcessListingCallback {
public:
  explicit ProcessCollection() : _Listing(*this) {}
  ~ProcessCollection() = default;

  std::shared_ptr<Process> GetProcess(backend::process::PidType pid) const;

  void UpdateList();

  std::shared_ptr<Process> MoveCursor(std::shared_ptr<Process> current, ssize_t offset);

  std::shared_ptr<Process> GetValidAncestor(std::shared_ptr<Process> process);
  std::vector<std::shared_ptr<Process>> GetTopK(ssize_t k);
  std::vector<std::shared_ptr<Process>> GetAround(std::shared_ptr<Process> process, ssize_t index, ssize_t max);

  void operator()(backend::process::PidType pid, const std::filesystem::path& dir) override;

private:
  backend::process::ProcessListing _Listing;
  std::unordered_map<backend::process::PidType, std::shared_ptr<Process>> _ProcessCache;
};

} // namespace frontend::ftxui
