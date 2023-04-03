#pragma once

#include <filesystem>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Types.hpp"

namespace backend::process {

class Process;

class ProcessListingCallback {
public:
  virtual ~ProcessListingCallback() = default;
  virtual void operator()(PidType pid, const std::filesystem::path& dir) = 0;
};

class ProcessListing {
public:
  explicit ProcessListing(ProcessListingCallback& callback) : _Callback(callback) {}
  ~ProcessListing() = default;

  void DoIterate();

private:
  static PidType PeekPid(const std::filesystem::path& path);
  static bool Skip(const std::filesystem::directory_entry& entry);
  static const std::filesystem::path _ProcPath;

  ProcessListingCallback& _Callback;
};

} // namespace backend::process
