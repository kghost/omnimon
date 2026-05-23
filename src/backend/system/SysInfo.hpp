#pragma once

#include <memory>

#include "../metrics/Gauge.hpp"

namespace backend::system {

class SysInfo {
public:
  explicit SysInfo() = default;
  ~SysInfo() = default;

  SysInfo(const SysInfo&) = delete;
  SysInfo& operator=(const SysInfo&) = delete;
  SysInfo(SysInfo&&) = delete;
  SysInfo& operator=(SysInfo&&) = delete;

  std::shared_ptr<metrics::Gauge> GetSystemJiffies();
  std::shared_ptr<metrics::Gauge> GetTotalMem();

private:
  class SysConstGauge : public metrics::ConstGauge {
  public:
    explicit SysConstGauge(metrics::DataType value) : metrics::ConstGauge(value) {}
    ~SysConstGauge() override = default;
  };

  std::shared_ptr<SysConstGauge> _SystemJiffies;
  std::shared_ptr<SysConstGauge> _TotalMem;
};

} // namespace backend::system
