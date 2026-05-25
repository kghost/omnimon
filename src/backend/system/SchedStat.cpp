#include "SchedStat.hpp"
#include <fstream>
#include <string>
#include <sstream>

namespace backend::system {

void SchedStat::Update() {
  std::ifstream file("/proc/schedstat");
  std::string line;
  int cpuIdx = 0;
  while (std::getline(file, line)) {
    if (line.compare(0, 3, "cpu") == 0) {
      std::istringstream iss(line);
      std::string cpuName;
      iss >> cpuName;
      
      if (cpuIdx >= _CpuStats.size()) {
        CpuSchedStat stat;
        stat.yld_count = std::make_shared<metrics::SharedGauge>(*this);
        stat.yld_exp = std::make_shared<metrics::SharedGauge>(*this);
        stat.sched_count = std::make_shared<metrics::SharedGauge>(*this);
        stat.sched_goidle = std::make_shared<metrics::SharedGauge>(*this);
        stat.ttwu_count = std::make_shared<metrics::SharedGauge>(*this);
        stat.ttwu_local = std::make_shared<metrics::SharedGauge>(*this);
        stat.rq_cpu_time = std::make_shared<metrics::SharedGauge>(*this);
        stat.run_delay = std::make_shared<metrics::SharedGauge>(*this);
        stat.pcpu_run_sched_info = std::make_shared<metrics::SharedGauge>(*this);
        _CpuStats.push_back(stat);
      }
      
      uint64_t yld, yldExp, sched, goidle, ttwu, ttwuLocal, rqCpuTime, runDelay, pcpuRunSchedInfo;
      iss >> yld >> yldExp >> sched >> goidle >> ttwu >> ttwuLocal >> rqCpuTime >> runDelay >> pcpuRunSchedInfo;
      
      _CpuStats[cpuIdx].yld_count->SetValue(yld);
      _CpuStats[cpuIdx].yld_exp->SetValue(yldExp);
      _CpuStats[cpuIdx].sched_count->SetValue(sched);
      _CpuStats[cpuIdx].sched_goidle->SetValue(goidle);
      _CpuStats[cpuIdx].ttwu_count->SetValue(ttwu);
      _CpuStats[cpuIdx].ttwu_local->SetValue(ttwuLocal);
      _CpuStats[cpuIdx].rq_cpu_time->SetValue(rqCpuTime);
      _CpuStats[cpuIdx].run_delay->SetValue(runDelay);
      _CpuStats[cpuIdx].pcpu_run_sched_info->SetValue(pcpuRunSchedInfo);
      
      cpuIdx++;
    }
  }
  _LastUpdate = std::chrono::steady_clock::now();
}

} // namespace backend::system
