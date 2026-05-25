#pragma once
#include "../metrics/SharedGauge.hpp"
#include <memory>
#include <string>
#include <vector>

namespace backend::system {

class CpuStat : public metrics::LastUpdateOwner {
public:
  struct CpuData {
    std::string name;
    std::shared_ptr<metrics::SharedGauge> user;
    std::shared_ptr<metrics::SharedGauge> nice;
    std::shared_ptr<metrics::SharedGauge> system;
    std::shared_ptr<metrics::SharedGauge> idle;
    std::shared_ptr<metrics::SharedGauge> iowait;
    std::shared_ptr<metrics::SharedGauge> irq;
    std::shared_ptr<metrics::SharedGauge> softirq;
    std::shared_ptr<metrics::SharedGauge> steal;
    std::shared_ptr<metrics::SharedGauge> guest;
    std::shared_ptr<metrics::SharedGauge> guest_nice;
  };

  CpuStat();
  ~CpuStat() override = default;

  void Update();

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  const CpuData& GetTotal() const { return _Total; }
  const std::vector<CpuData>& GetCpus() const { return _Cpus; }

private:
  CpuData InitCpuData(const std::string& name);

  std::chrono::steady_clock::time_point _LastUpdate;
  CpuData _Total;
  std::vector<CpuData> _Cpus;
};

} // namespace backend::system
