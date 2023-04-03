#include "ProcessListing.hpp"

#include <ranges>

#include "Process.hpp"

namespace backend::process {

const std::filesystem::path ProcessListing::_ProcPath{"/proc"};

void ProcessListing::DoIterate() {
  for (auto& dir : std::filesystem::directory_iterator(_ProcPath)) {
    if (Skip(dir)) {
      continue;
    }

    _Callback(PeekPid(dir), dir);
  }
}

PidType ProcessListing::PeekPid(const std::filesystem::path& path) { return std::stoi(path.filename().string()); }

bool ProcessListing::Skip(const std::filesystem::directory_entry& entry) {
  if (!entry.is_directory()) {
    return true;
  }

  std::string filename = entry.path().filename();
  if (!std::all_of(filename.begin(), filename.end(), ::isdigit)) {
    return true;
  }

  return false;
}

} // namespace backend::process