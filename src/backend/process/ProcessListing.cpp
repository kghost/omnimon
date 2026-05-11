#include "ProcessListing.hpp"

#include <ranges>

#include "Process.hpp"

namespace backend::process {

const std::filesystem::path ProcessListing::_ProcPath{"/proc"};

void ProcessListing::DoIterate() {
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

  for (auto& dir : std::filesystem::directory_iterator(_ProcPath)) {
    if (SkipNonePidDir(dir)) {
      continue;
    }

    _Callback(PeekPid(dir), dir);
  }
}

PidType ProcessListing::PeekPid(const std::filesystem::path& path) { return std::stoi(path.filename().string()); }

} // namespace backend::process