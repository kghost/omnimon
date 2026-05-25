#include "SysInfo.hpp"

#include "Uptime.hpp"
#include "LoadAvg.hpp"
#include "SchedStat.hpp"
#include "CpuStat.hpp"
#include "MemInfo.hpp"
#include "VmStat.hpp"

#include <sys/sysinfo.h>
#include <unistd.h>

namespace backend::system {

SysInfo::SysInfo() = default;
SysInfo::~SysInfo() = default;

std::shared_ptr<metrics::Gauge> SysInfo::GetSystemJiffies() {
  if (!_SystemJiffies) {
    _SystemJiffies.reset(new SysConstGauge(sysconf(_SC_CLK_TCK)));
  }
  return _SystemJiffies;
}

std::shared_ptr<metrics::Gauge> SysInfo::GetTotalMem() {
  if (!_TotalMem) {
    struct sysinfo info;
    sysinfo(&info);

    long pageSize = sysconf(_SC_PAGESIZE);

    _TotalMem.reset(new SysConstGauge(info.totalram / pageSize));
  }
  return _TotalMem;
}

Uptime* SysInfo::GetUptime() {
  if (!_Uptime) _Uptime = std::make_unique<Uptime>();
  return _Uptime.get();
}

LoadAvg* SysInfo::GetLoadAvg() {
  if (!_LoadAvg) _LoadAvg = std::make_unique<LoadAvg>();
  return _LoadAvg.get();
}

SchedStat* SysInfo::GetSchedStat() {
  if (!_SchedStat) _SchedStat = std::make_unique<SchedStat>();
  return _SchedStat.get();
}

CpuStat* SysInfo::GetCpuStat() {
  if (!_CpuStat) _CpuStat = std::make_unique<CpuStat>();
  return _CpuStat.get();
}

MemInfo* SysInfo::GetMemInfo() {
  if (!_MemInfo) _MemInfo = std::make_unique<MemInfo>();
  return _MemInfo.get();
}

VmStat* SysInfo::GetVmStat() {
  if (!_VmStat) _VmStat = std::make_unique<VmStat>();
  return _VmStat.get();
}

} // namespace backend::system
