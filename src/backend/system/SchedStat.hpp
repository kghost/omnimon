#pragma once
#include "../metrics/SharedGauge.hpp"
#include <memory>
#include <vector>

namespace backend::system {

class SchedStat : public metrics::LastUpdateOwner {
public:
  struct CpuSchedStat {
    std::shared_ptr<metrics::SharedGauge> yld_count;
    std::shared_ptr<metrics::SharedGauge> yld_exp;
    std::shared_ptr<metrics::SharedGauge> sched_count;
    std::shared_ptr<metrics::SharedGauge> sched_goidle;
    std::shared_ptr<metrics::SharedGauge> ttwu_count;
    std::shared_ptr<metrics::SharedGauge> ttwu_local;
    std::shared_ptr<metrics::SharedGauge> rq_cpu_time;
    std::shared_ptr<metrics::SharedGauge> run_delay;
    std::shared_ptr<metrics::SharedGauge> pcpu_run_sched_info;
  };

  SchedStat() = default;
  ~SchedStat() override = default;

  void Update();

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  const std::vector<CpuSchedStat>& GetCpuStats() const { return _CpuStats; }

private:
  std::chrono::steady_clock::time_point _LastUpdate;
  std::vector<CpuSchedStat> _CpuStats;
};

} // namespace backend::system
