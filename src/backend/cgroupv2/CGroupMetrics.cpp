#include "CGroupMetrics.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "CGroupNode.hpp"

namespace backend::cgroupv2 {

std::optional<metrics::DataType> ReadNumericFile(const std::filesystem::path& path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    return std::nullopt;
  }

  std::string contents;
  try {
    std::getline(ifs, contents);
  } catch (const std::ios_base::failure&) {
    return std::nullopt;
  }

  if (contents.empty()) {
    return std::nullopt;
  }

  try {
    return static_cast<metrics::DataType>(std::stoull(contents));
  } catch (const std::invalid_argument&) {
    return std::nullopt;
  } catch (const std::out_of_range&) {
    return std::nullopt;
  }
}

std::map<std::string, metrics::DataType> ReadKeyValueFile(const std::filesystem::path& path) {
  std::map<std::string, metrics::DataType> result;
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    return result;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    std::istringstream stream(line);
    std::string key;
    metrics::DataType value;
    if (!(stream >> key >> value)) {
      continue;
    }
    result[key] = value;
  }

  return result;
}

struct IoStat {
  metrics::DataType rbytes = 0;
  metrics::DataType wbytes = 0;
  metrics::DataType rios = 0;
  metrics::DataType wios = 0;
  metrics::DataType dbytes = 0;
  metrics::DataType dios = 0;
};

std::string GetDiskName(const std::string& device) {
  static std::map<std::string, std::string> cache;
  if (auto it = cache.find(device); it != cache.end()) {
    return it->second;
  }

  std::string result = device;
  std::ifstream ifs("/sys/dev/block/" + device + "/uevent");
  if (ifs.is_open()) {
    std::string line;
    while (std::getline(ifs, line)) {
      if (line.starts_with("DEVNAME=")) {
        result = line.substr(8);
        break;
      }
    }
  }

  cache[device] = result;
  return result;
}

std::map<std::string, IoStat> ReadIoStatFile(const std::filesystem::path& path) {
  std::map<std::string, IoStat> result;
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    return result;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    std::istringstream stream(line);
    std::string device;
    if (!(stream >> device)) {
      continue;
    }

    IoStat stat;
    std::string token;
    while (stream >> token) {
      auto eqPos = token.find('=');
      if (eqPos == std::string::npos) {
        continue;
      }
      std::string key = token.substr(0, eqPos);
      std::string valStr = token.substr(eqPos + 1);

      try {
        metrics::DataType value = std::stoull(valStr);
        if (key == "rbytes") {
          stat.rbytes += value;
        } else if (key == "wbytes") {
          stat.wbytes += value;
        } else if (key == "rios") {
          stat.rios += value;
        } else if (key == "wios") {
          stat.wios += value;
        } else if (key == "dbytes") {
          stat.dbytes += value;
        } else if (key == "dios") {
          stat.dios += value;
        }
      } catch (...) {
      }
    }
    result[GetDiskName(device)] = stat;
  }

  return result;
}

CGroupMetrics::CGroupMetrics(CGroupNode& node)
    : _Node(node), _LastUpdate(std::chrono::steady_clock::now()),
      _MemoryCurrent(std::make_shared<metrics::SharedGauge>(*this)),
      _PidsCurrent(std::make_shared<metrics::SharedGauge>(*this)),
      _CpuUsageUsec(std::make_shared<metrics::SharedPublisher<std::chrono::microseconds>>(*this)),
      _CpuUserUsec(std::make_shared<metrics::SharedPublisher<std::chrono::microseconds>>(*this)),
      _CpuSystemUsec(std::make_shared<metrics::SharedPublisher<std::chrono::microseconds>>(*this)) {
  ReadFromDirectory();
}

void CGroupMetrics::ReadFromDirectory() {
  _LastUpdate = std::chrono::steady_clock::now();
  const auto path = _Node.GetPath();
  if (auto value = ReadNumericFile(path / "memory.current")) {
    _MemoryCurrent->SetValue(value.value());
  }
  if (auto value = ReadNumericFile(path / "pids.current")) {
    _PidsCurrent->SetValue(value.value());
  }

  auto cpuStatValues = ReadKeyValueFile(path / "cpu.stat");
  if (auto value = cpuStatValues.find("usage_usec"); value != cpuStatValues.end()) {
    _CpuUsageUsec->SetValue(std::chrono::microseconds(value->second));
  }
  if (auto value = cpuStatValues.find("user_usec"); value != cpuStatValues.end()) {
    _CpuUserUsec->SetValue(std::chrono::microseconds(value->second));
  }
  if (auto value = cpuStatValues.find("system_usec"); value != cpuStatValues.end()) {
    _CpuSystemUsec->SetValue(std::chrono::microseconds(value->second));
  }

  for (const auto& [disk, stat] : ReadIoStatFile(path / "io.stat")) {
    class LazyIoStatGauges {
    public:
      explicit LazyIoStatGauges(CGroupMetrics& parent) : _Parent(parent) {}
      operator IoStatGauges() const {
        return IoStatGauges{
            std::make_shared<metrics::SharedGauge>(_Parent), std::make_shared<metrics::SharedGauge>(_Parent),
            std::make_shared<metrics::SharedGauge>(_Parent), std::make_shared<metrics::SharedGauge>(_Parent),
            std::make_shared<metrics::SharedGauge>(_Parent), std::make_shared<metrics::SharedGauge>(_Parent)};
      }

    private:
      CGroupMetrics& _Parent;
    };
    auto [it, isNew] = _IoStats.try_emplace(disk, LazyIoStatGauges(*this));
    IoStatGauges& gauges = it->second;
    gauges.ReadBytes->SetValue(stat.rbytes);
    gauges.WriteBytes->SetValue(stat.wbytes);
    gauges.ReadCalls->SetValue(stat.rios);
    gauges.WriteCalls->SetValue(stat.wios);
    gauges.DiscardBytes->SetValue(stat.dbytes);
    gauges.DiscardCalls->SetValue(stat.dios);
  }
}

} // namespace backend::cgroupv2
