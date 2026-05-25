#pragma once

#include <memory>

#include "../metrics/Gauge.hpp"

namespace backend::system {

class Uptime;
class LoadAvg;
class SchedStat;
class CpuStat;
class MemInfo;
class VmStat;

class SysInfo {
public:
  explicit SysInfo();
  ~SysInfo();

  SysInfo(const SysInfo&) = delete;
  SysInfo& operator=(const SysInfo&) = delete;
  SysInfo(SysInfo&&) = delete;
  SysInfo& operator=(SysInfo&&) = delete;

  std::shared_ptr<metrics::Gauge> GetSystemJiffies();
  std::shared_ptr<metrics::Gauge> GetTotalMem();

  Uptime* GetUptime();
  LoadAvg* GetLoadAvg();
  SchedStat* GetSchedStat();
  CpuStat* GetCpuStat();
  MemInfo* GetMemInfo();
  VmStat* GetVmStat();

private:
  class SysConstGauge : public metrics::ConstGauge {
  public:
    explicit SysConstGauge(metrics::DataType value) : metrics::ConstGauge(value) {}
    ~SysConstGauge() override = default;
  };

  std::shared_ptr<SysConstGauge> _SystemJiffies;
  std::shared_ptr<SysConstGauge> _TotalMem;

  std::unique_ptr<Uptime> _Uptime;
  std::unique_ptr<LoadAvg> _LoadAvg;
  std::unique_ptr<SchedStat> _SchedStat;
  std::unique_ptr<CpuStat> _CpuStat;
  std::unique_ptr<MemInfo> _MemInfo;
  std::unique_ptr<VmStat> _VmStat;
};

} // namespace backend::system
