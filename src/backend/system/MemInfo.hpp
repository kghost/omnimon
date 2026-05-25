#pragma once

#include <chrono>
#include <memory>

#include "../metrics/SharedGauge.hpp"

namespace backend::system {

class MemInfo : public metrics::LastUpdateOwner {
public:
  MemInfo();
  ~MemInfo() override = default;

  void Update();

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  std::shared_ptr<metrics::SharedGauge> GetMemTotal() const { return _MemTotal; }
  std::shared_ptr<metrics::SharedGauge> GetMemFree() const { return _MemFree; }
  std::shared_ptr<metrics::SharedGauge> GetMemAvailable() const { return _MemAvailable; }
  std::shared_ptr<metrics::SharedGauge> GetBuffers() const { return _Buffers; }
  std::shared_ptr<metrics::SharedGauge> GetCached() const { return _Cached; }
  std::shared_ptr<metrics::SharedGauge> GetSwapCached() const { return _SwapCached; }
  std::shared_ptr<metrics::SharedGauge> GetActive() const { return _Active; }
  std::shared_ptr<metrics::SharedGauge> GetInactive() const { return _Inactive; }
  std::shared_ptr<metrics::SharedGauge> GetActiveAnon() const { return _ActiveAnon; }
  std::shared_ptr<metrics::SharedGauge> GetInactiveAnon() const { return _InactiveAnon; }
  std::shared_ptr<metrics::SharedGauge> GetActiveFile() const { return _ActiveFile; }
  std::shared_ptr<metrics::SharedGauge> GetInactiveFile() const { return _InactiveFile; }
  std::shared_ptr<metrics::SharedGauge> GetUnevictable() const { return _Unevictable; }
  std::shared_ptr<metrics::SharedGauge> GetMlocked() const { return _Mlocked; }
  std::shared_ptr<metrics::SharedGauge> GetSwapTotal() const { return _SwapTotal; }
  std::shared_ptr<metrics::SharedGauge> GetSwapFree() const { return _SwapFree; }
  std::shared_ptr<metrics::SharedGauge> GetDirty() const { return _Dirty; }
  std::shared_ptr<metrics::SharedGauge> GetWriteback() const { return _Writeback; }
  std::shared_ptr<metrics::SharedGauge> GetAnonPages() const { return _AnonPages; }
  std::shared_ptr<metrics::SharedGauge> GetMapped() const { return _Mapped; }
  std::shared_ptr<metrics::SharedGauge> GetShmem() const { return _Shmem; }
  std::shared_ptr<metrics::SharedGauge> GetSlab() const { return _Slab; }
  std::shared_ptr<metrics::SharedGauge> GetSReclaimable() const { return _SReclaimable; }
  std::shared_ptr<metrics::SharedGauge> GetSUnreclaim() const { return _SUnreclaim; }
  std::shared_ptr<metrics::SharedGauge> GetKernelStack() const { return _KernelStack; }
  std::shared_ptr<metrics::SharedGauge> GetPageTables() const { return _PageTables; }
  std::shared_ptr<metrics::SharedGauge> GetCommittedAs() const { return _CommittedAs; }
  std::shared_ptr<metrics::SharedGauge> GetVmallocTotal() const { return _VmallocTotal; }
  std::shared_ptr<metrics::SharedGauge> GetVmallocUsed() const { return _VmallocUsed; }
  std::shared_ptr<metrics::SharedGauge> GetVmallocChunk() const { return _VmallocChunk; }

private:
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<metrics::SharedGauge> _MemTotal;
  std::shared_ptr<metrics::SharedGauge> _MemFree;
  std::shared_ptr<metrics::SharedGauge> _MemAvailable;
  std::shared_ptr<metrics::SharedGauge> _Buffers;
  std::shared_ptr<metrics::SharedGauge> _Cached;
  std::shared_ptr<metrics::SharedGauge> _SwapCached;
  std::shared_ptr<metrics::SharedGauge> _Active;
  std::shared_ptr<metrics::SharedGauge> _Inactive;
  std::shared_ptr<metrics::SharedGauge> _ActiveAnon;
  std::shared_ptr<metrics::SharedGauge> _InactiveAnon;
  std::shared_ptr<metrics::SharedGauge> _ActiveFile;
  std::shared_ptr<metrics::SharedGauge> _InactiveFile;
  std::shared_ptr<metrics::SharedGauge> _Unevictable;
  std::shared_ptr<metrics::SharedGauge> _Mlocked;
  std::shared_ptr<metrics::SharedGauge> _SwapTotal;
  std::shared_ptr<metrics::SharedGauge> _SwapFree;
  std::shared_ptr<metrics::SharedGauge> _Dirty;
  std::shared_ptr<metrics::SharedGauge> _Writeback;
  std::shared_ptr<metrics::SharedGauge> _AnonPages;
  std::shared_ptr<metrics::SharedGauge> _Mapped;
  std::shared_ptr<metrics::SharedGauge> _Shmem;
  std::shared_ptr<metrics::SharedGauge> _Slab;
  std::shared_ptr<metrics::SharedGauge> _SReclaimable;
  std::shared_ptr<metrics::SharedGauge> _SUnreclaim;
  std::shared_ptr<metrics::SharedGauge> _KernelStack;
  std::shared_ptr<metrics::SharedGauge> _PageTables;
  std::shared_ptr<metrics::SharedGauge> _CommittedAs;
  std::shared_ptr<metrics::SharedGauge> _VmallocTotal;
  std::shared_ptr<metrics::SharedGauge> _VmallocUsed;
  std::shared_ptr<metrics::SharedGauge> _VmallocChunk;
};

} // namespace backend::system
