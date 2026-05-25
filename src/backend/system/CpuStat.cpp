#include "CpuStat.hpp"
#include <fstream>
#include <sstream>

namespace backend::system {

CpuStat::CpuStat() : _Total(InitCpuData("cpu")) {}

CpuStat::CpuData CpuStat::InitCpuData(const std::string& name) {
  return {
      .name = name,
      .user = std::make_shared<metrics::SharedGauge>(*this),
      .nice = std::make_shared<metrics::SharedGauge>(*this),
      .system = std::make_shared<metrics::SharedGauge>(*this),
      .idle = std::make_shared<metrics::SharedGauge>(*this),
      .iowait = std::make_shared<metrics::SharedGauge>(*this),
      .irq = std::make_shared<metrics::SharedGauge>(*this),
      .softirq = std::make_shared<metrics::SharedGauge>(*this),
      .steal = std::make_shared<metrics::SharedGauge>(*this),
      .guest = std::make_shared<metrics::SharedGauge>(*this),
      .guest_nice = std::make_shared<metrics::SharedGauge>(*this),
  };
}

void CpuStat::Update() {
  std::ifstream file("/proc/stat");
  std::string line;
  int cpuIdx = 0;
  while (std::getline(file, line)) {
    if (line.compare(0, 3, "cpu") == 0) {
      std::istringstream iss(line);
      std::string name;
      iss >> name;

      uint64_t u = 0, n = 0, s = 0, i = 0, iw = 0, ir = 0, sir = 0, st = 0, g = 0, gn = 0;
      iss >> u >> n >> s >> i >> iw >> ir >> sir >> st >> g >> gn;

      CpuData* dataPtr = nullptr;
      if (name == "cpu") {
        dataPtr = &_Total;
      } else {
        if (cpuIdx >= _Cpus.size()) {
          _Cpus.emplace_back(InitCpuData(name));
        }
        dataPtr = &_Cpus[cpuIdx];
        cpuIdx++;
      }

      dataPtr->user->SetValue(u);
      dataPtr->nice->SetValue(n);
      dataPtr->system->SetValue(s);
      dataPtr->idle->SetValue(i);
      dataPtr->iowait->SetValue(iw);
      dataPtr->irq->SetValue(ir);
      dataPtr->softirq->SetValue(sir);
      dataPtr->steal->SetValue(st);
      dataPtr->guest->SetValue(g);
      dataPtr->guest_nice->SetValue(gn);
    }
  }
  _LastUpdate = std::chrono::steady_clock::now();
}

} // namespace backend::system
