#include "MemInfo.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace backend::system {

MemInfo::MemInfo() {
  _MemTotal = std::make_shared<metrics::SharedGauge>(*this);
  _MemFree = std::make_shared<metrics::SharedGauge>(*this);
  _MemAvailable = std::make_shared<metrics::SharedGauge>(*this);
  _Buffers = std::make_shared<metrics::SharedGauge>(*this);
  _Cached = std::make_shared<metrics::SharedGauge>(*this);
  _SwapCached = std::make_shared<metrics::SharedGauge>(*this);
  _Active = std::make_shared<metrics::SharedGauge>(*this);
  _Inactive = std::make_shared<metrics::SharedGauge>(*this);
  _ActiveAnon = std::make_shared<metrics::SharedGauge>(*this);
  _InactiveAnon = std::make_shared<metrics::SharedGauge>(*this);
  _ActiveFile = std::make_shared<metrics::SharedGauge>(*this);
  _InactiveFile = std::make_shared<metrics::SharedGauge>(*this);
  _Unevictable = std::make_shared<metrics::SharedGauge>(*this);
  _Mlocked = std::make_shared<metrics::SharedGauge>(*this);
  _SwapTotal = std::make_shared<metrics::SharedGauge>(*this);
  _SwapFree = std::make_shared<metrics::SharedGauge>(*this);
  _Dirty = std::make_shared<metrics::SharedGauge>(*this);
  _Writeback = std::make_shared<metrics::SharedGauge>(*this);
  _AnonPages = std::make_shared<metrics::SharedGauge>(*this);
  _Mapped = std::make_shared<metrics::SharedGauge>(*this);
  _Shmem = std::make_shared<metrics::SharedGauge>(*this);
  _Slab = std::make_shared<metrics::SharedGauge>(*this);
  _SReclaimable = std::make_shared<metrics::SharedGauge>(*this);
  _SUnreclaim = std::make_shared<metrics::SharedGauge>(*this);
  _KernelStack = std::make_shared<metrics::SharedGauge>(*this);
  _PageTables = std::make_shared<metrics::SharedGauge>(*this);
  _CommittedAs = std::make_shared<metrics::SharedGauge>(*this);
  _VmallocTotal = std::make_shared<metrics::SharedGauge>(*this);
  _VmallocUsed = std::make_shared<metrics::SharedGauge>(*this);
  _VmallocChunk = std::make_shared<metrics::SharedGauge>(*this);
}

void MemInfo::Update() {
  std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    return;
  }

  std::string line;
  while (std::getline(file, line)) {
    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
      std::string key = line.substr(0, colonPos);
      uint64_t value = 0;
      std::string rest = line.substr(colonPos + 1);
      std::istringstream iss(rest);
      iss >> value;
      
      if (key == "MemTotal") {
        _MemTotal->SetValue(value);
      } else if (key == "MemFree") {
        _MemFree->SetValue(value);
      } else if (key == "MemAvailable") {
        _MemAvailable->SetValue(value);
      } else if (key == "Buffers") {
        _Buffers->SetValue(value);
      } else if (key == "Cached") {
        _Cached->SetValue(value);
      } else if (key == "SwapCached") {
        _SwapCached->SetValue(value);
      } else if (key == "Active") {
        _Active->SetValue(value);
      } else if (key == "Inactive") {
        _Inactive->SetValue(value);
      } else if (key == "Active(anon)") {
        _ActiveAnon->SetValue(value);
      } else if (key == "Inactive(anon)") {
        _InactiveAnon->SetValue(value);
      } else if (key == "Active(file)") {
        _ActiveFile->SetValue(value);
      } else if (key == "Inactive(file)") {
        _InactiveFile->SetValue(value);
      } else if (key == "Unevictable") {
        _Unevictable->SetValue(value);
      } else if (key == "Mlocked") {
        _Mlocked->SetValue(value);
      } else if (key == "SwapTotal") {
        _SwapTotal->SetValue(value);
      } else if (key == "SwapFree") {
        _SwapFree->SetValue(value);
      } else if (key == "Dirty") {
        _Dirty->SetValue(value);
      } else if (key == "Writeback") {
        _Writeback->SetValue(value);
      } else if (key == "AnonPages") {
        _AnonPages->SetValue(value);
      } else if (key == "Mapped") {
        _Mapped->SetValue(value);
      } else if (key == "Shmem") {
        _Shmem->SetValue(value);
      } else if (key == "Slab") {
        _Slab->SetValue(value);
      } else if (key == "SReclaimable") {
        _SReclaimable->SetValue(value);
      } else if (key == "SUnreclaim") {
        _SUnreclaim->SetValue(value);
      } else if (key == "KernelStack") {
        _KernelStack->SetValue(value);
      } else if (key == "PageTables") {
        _PageTables->SetValue(value);
      } else if (key == "Committed_AS") {
        _CommittedAs->SetValue(value);
      } else if (key == "VmallocTotal") {
        _VmallocTotal->SetValue(value);
      } else if (key == "VmallocUsed") {
        _VmallocUsed->SetValue(value);
      } else if (key == "VmallocChunk") {
        _VmallocChunk->SetValue(value);
      }
    }
  }
  _LastUpdate = std::chrono::steady_clock::now();
}

} // namespace backend::system
