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
}

} // namespace backend::cgroupv2
