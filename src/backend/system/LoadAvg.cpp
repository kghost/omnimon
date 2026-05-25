#include "LoadAvg.hpp"
#include <fstream>
#include <string>

namespace backend::system {

LoadAvg::LoadAvg() {
  _Load1 = std::make_shared<metrics::SharedPublisher<double>>(*this);
  _Load5 = std::make_shared<metrics::SharedPublisher<double>>(*this);
  _Load15 = std::make_shared<metrics::SharedPublisher<double>>(*this);
  _Runnable = std::make_shared<metrics::SharedGauge>(*this);
  _TotalTasks = std::make_shared<metrics::SharedGauge>(*this);
  _LastPid = std::make_shared<metrics::SharedGauge>(*this);
}

void LoadAvg::Update() {
  std::ifstream file("/proc/loadavg");
  if (file.is_open()) {
    double l1, l5, l15;
    file >> l1 >> l5 >> l15;
    _Load1->SetValue(l1);
    _Load5->SetValue(l5);
    _Load15->SetValue(l15);
    
    std::string tasks;
    file >> tasks;
    size_t slashPos = tasks.find('/');
    if (slashPos != std::string::npos) {
      _Runnable->SetValue(std::stoull(tasks.substr(0, slashPos)));
      _TotalTasks->SetValue(std::stoull(tasks.substr(slashPos + 1)));
    }
    
    int pid;
    file >> pid;
    _LastPid->SetValue(pid);
    
    _LastUpdate = std::chrono::steady_clock::now();
  }
}

} // namespace backend::system
