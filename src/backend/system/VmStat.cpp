#include "VmStat.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace backend::system {

VmStat::VmStat() {
  _NrFreePages = std::make_shared<metrics::SharedGauge>(*this);
  _NrInactiveAnon = std::make_shared<metrics::SharedGauge>(*this);
  _NrActiveAnon = std::make_shared<metrics::SharedGauge>(*this);
  _NrInactiveFile = std::make_shared<metrics::SharedGauge>(*this);
  _NrActiveFile = std::make_shared<metrics::SharedGauge>(*this);
  _NrUnevictable = std::make_shared<metrics::SharedGauge>(*this);
  _NrDirty = std::make_shared<metrics::SharedGauge>(*this);
  _NrWriteback = std::make_shared<metrics::SharedGauge>(*this);
  _NrAnonPages = std::make_shared<metrics::SharedGauge>(*this);
  _NrMapped = std::make_shared<metrics::SharedGauge>(*this);
  _NrFilePages = std::make_shared<metrics::SharedGauge>(*this);
  _NrSlabReclaimable = std::make_shared<metrics::SharedGauge>(*this);
  _NrSlabUnreclaimable = std::make_shared<metrics::SharedGauge>(*this);
  _Pgpgin = std::make_shared<metrics::SharedGauge>(*this);
  _Pgpgout = std::make_shared<metrics::SharedGauge>(*this);
  _Pswpin = std::make_shared<metrics::SharedGauge>(*this);
  _Pswpout = std::make_shared<metrics::SharedGauge>(*this);
  _Pgfault = std::make_shared<metrics::SharedGauge>(*this);
  _Pgmajfault = std::make_shared<metrics::SharedGauge>(*this);
}

void VmStat::Update() {
  std::ifstream file("/proc/vmstat");
  if (!file.is_open()) {
    return;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string key;
    uint64_t value = 0;
    if (iss >> key >> value) {
      if (key == "nr_free_pages") {
        _NrFreePages->SetValue(value);
      } else if (key == "nr_inactive_anon") {
        _NrInactiveAnon->SetValue(value);
      } else if (key == "nr_active_anon") {
        _NrActiveAnon->SetValue(value);
      } else if (key == "nr_inactive_file") {
        _NrInactiveFile->SetValue(value);
      } else if (key == "nr_active_file") {
        _NrActiveFile->SetValue(value);
      } else if (key == "nr_unevictable") {
        _NrUnevictable->SetValue(value);
      } else if (key == "nr_dirty") {
        _NrDirty->SetValue(value);
      } else if (key == "nr_writeback") {
        _NrWriteback->SetValue(value);
      } else if (key == "nr_anon_pages") {
        _NrAnonPages->SetValue(value);
      } else if (key == "nr_mapped") {
        _NrMapped->SetValue(value);
      } else if (key == "nr_file_pages") {
        _NrFilePages->SetValue(value);
      } else if (key == "nr_slab_reclaimable") {
        _NrSlabReclaimable->SetValue(value);
      } else if (key == "nr_slab_unreclaimable") {
        _NrSlabUnreclaimable->SetValue(value);
      } else if (key == "pgpgin") {
        _Pgpgin->SetValue(value);
      } else if (key == "pgpgout") {
        _Pgpgout->SetValue(value);
      } else if (key == "pswpin") {
        _Pswpin->SetValue(value);
      } else if (key == "pswpout") {
        _Pswpout->SetValue(value);
      } else if (key == "pgfault") {
        _Pgfault->SetValue(value);
      } else if (key == "pgmajfault") {
        _Pgmajfault->SetValue(value);
      }
    }
  }
  _LastUpdate = std::chrono::steady_clock::now();
}

} // namespace backend::system
