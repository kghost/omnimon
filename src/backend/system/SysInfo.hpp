#pragma once

#include <memory>

#include "../metrics/Gauge.hpp"

namespace backend::system {

class SysInfo {
public:
  static std::shared_ptr<SysInfo> GetInstance();

  explicit SysInfo() = default;
  ~SysInfo() = default;

  std::shared_ptr<metrics::Gauge> GetSystemJiffies();
  std::shared_ptr<metrics::Gauge> GetTotalMem();

private:
  class SysConstGauge : public metrics::ConstGauge {
  public:
    explicit SysConstGauge(metrics::DataType value) : metrics::ConstGauge(value), _SysInfo(SysInfo::GetInstance()) {}
    ~SysConstGauge() override = default;

  private:
    // Hold SysInfo to prevent it from being released.
    std::shared_ptr<SysInfo> _SysInfo;
  };

  std::shared_ptr<SysConstGauge> _SystemJiffies;
  std::shared_ptr<SysConstGauge> _TotalMem;
};

} // namespace backend::system
