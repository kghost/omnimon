#include "SysInfo.hpp"

#include <sys/sysinfo.h>
#include <unistd.h>

namespace backend::system {

std::shared_ptr<SysInfo> SysInfo::GetInstance() {
  static std::weak_ptr<SysInfo> weak;
  if (auto instance = weak.lock()) {
    return instance;
  }

  auto instance = std::make_shared<SysInfo>();
  weak = instance;
  return instance;
}

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

} // namespace backend::system
