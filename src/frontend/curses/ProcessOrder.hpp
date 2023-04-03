#pragma once

#include <list>
#include <memory>
#include <unordered_map>

#include "../../backend/process/Process.hpp"
#include "../../backend/process/ProcessListing.hpp"
#include "../../utils/StringUtils.hpp"

namespace frontend::curses {

class Process;

class ProcessCollection : public backend::process::ProcessListingCallback {
public:
  explicit ProcessCollection() : _Listing(*this) {}
  ~ProcessCollection() = default;

  std::shared_ptr<Process> GetProcess(backend::process::PidType pid) const;

  void UpdateList();

  std::shared_ptr<Process> MoveCursor(std::shared_ptr<Process> current, DisplayLength offset);

  std::vector<std::shared_ptr<Process>> GetTopK(size_t k);
  std::vector<std::shared_ptr<Process>> GetAround(std::shared_ptr<Process> process, DisplayLength& cursor,
                                                  DisplayLength max, bool update);

  void operator()(backend::process::PidType pid, const std::filesystem::path& dir) override;

private:
  backend::process::ProcessListing _Listing;
  std::unordered_map<backend::process::PidType, std::shared_ptr<Process>> _ProcessCache;
};

} // namespace frontend::curses
