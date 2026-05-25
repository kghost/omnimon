#pragma once

#include <chrono>
#include <memory>

#include "../metrics/SharedGauge.hpp"

namespace backend::system {

class VmStat : public metrics::LastUpdateOwner {
public:
  VmStat();
  ~VmStat() override = default;

  void Update();

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  std::shared_ptr<metrics::SharedGauge> GetNrFreePages() const { return _NrFreePages; }
  std::shared_ptr<metrics::SharedGauge> GetNrInactiveAnon() const { return _NrInactiveAnon; }
  std::shared_ptr<metrics::SharedGauge> GetNrActiveAnon() const { return _NrActiveAnon; }
  std::shared_ptr<metrics::SharedGauge> GetNrInactiveFile() const { return _NrInactiveFile; }
  std::shared_ptr<metrics::SharedGauge> GetNrActiveFile() const { return _NrActiveFile; }
  std::shared_ptr<metrics::SharedGauge> GetNrUnevictable() const { return _NrUnevictable; }
  std::shared_ptr<metrics::SharedGauge> GetNrDirty() const { return _NrDirty; }
  std::shared_ptr<metrics::SharedGauge> GetNrWriteback() const { return _NrWriteback; }
  std::shared_ptr<metrics::SharedGauge> GetNrAnonPages() const { return _NrAnonPages; }
  std::shared_ptr<metrics::SharedGauge> GetNrMapped() const { return _NrMapped; }
  std::shared_ptr<metrics::SharedGauge> GetNrFilePages() const { return _NrFilePages; }
  std::shared_ptr<metrics::SharedGauge> GetNrSlabReclaimable() const { return _NrSlabReclaimable; }
  std::shared_ptr<metrics::SharedGauge> GetNrSlabUnreclaimable() const { return _NrSlabUnreclaimable; }
  std::shared_ptr<metrics::SharedGauge> GetPgpgin() const { return _Pgpgin; }
  std::shared_ptr<metrics::SharedGauge> GetPgpgout() const { return _Pgpgout; }
  std::shared_ptr<metrics::SharedGauge> GetPswpin() const { return _Pswpin; }
  std::shared_ptr<metrics::SharedGauge> GetPswpout() const { return _Pswpout; }
  std::shared_ptr<metrics::SharedGauge> GetPgfault() const { return _Pgfault; }
  std::shared_ptr<metrics::SharedGauge> GetPgmajfault() const { return _Pgmajfault; }

private:
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<metrics::SharedGauge> _NrFreePages;
  std::shared_ptr<metrics::SharedGauge> _NrInactiveAnon;
  std::shared_ptr<metrics::SharedGauge> _NrActiveAnon;
  std::shared_ptr<metrics::SharedGauge> _NrInactiveFile;
  std::shared_ptr<metrics::SharedGauge> _NrActiveFile;
  std::shared_ptr<metrics::SharedGauge> _NrUnevictable;
  std::shared_ptr<metrics::SharedGauge> _NrDirty;
  std::shared_ptr<metrics::SharedGauge> _NrWriteback;
  std::shared_ptr<metrics::SharedGauge> _NrAnonPages;
  std::shared_ptr<metrics::SharedGauge> _NrMapped;
  std::shared_ptr<metrics::SharedGauge> _NrFilePages;
  std::shared_ptr<metrics::SharedGauge> _NrSlabReclaimable;
  std::shared_ptr<metrics::SharedGauge> _NrSlabUnreclaimable;
  std::shared_ptr<metrics::SharedGauge> _Pgpgin;
  std::shared_ptr<metrics::SharedGauge> _Pgpgout;
  std::shared_ptr<metrics::SharedGauge> _Pswpin;
  std::shared_ptr<metrics::SharedGauge> _Pswpout;
  std::shared_ptr<metrics::SharedGauge> _Pgfault;
  std::shared_ptr<metrics::SharedGauge> _Pgmajfault;
};

} // namespace backend::system
