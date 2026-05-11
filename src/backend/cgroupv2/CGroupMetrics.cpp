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

CGroupMetrics::CGroupMetrics(std::shared_ptr<CGroupNode> node) : _Node(std::move(node)) { ReadFromDirectory(); }

const std::shared_ptr<CGroupNode>& CGroupMetrics::GetNode() const { return _Node; }

const std::optional<metrics::DataType>& CGroupMetrics::GetMemoryCurrent() const { return _MemoryCurrent; }

const std::optional<metrics::DataType>& CGroupMetrics::GetPidsCurrent() const { return _PidsCurrent; }

const std::optional<metrics::DataType>& CGroupMetrics::GetCpuUsageUsec() const { return _CpuUsageUsec; }

const std::optional<metrics::DataType>& CGroupMetrics::GetCpuUserUsec() const { return _CpuUserUsec; }

const std::optional<metrics::DataType>& CGroupMetrics::GetCpuSystemUsec() const { return _CpuSystemUsec; }

void CGroupMetrics::SetMemoryCurrent(std::optional<metrics::DataType> value) { _MemoryCurrent = std::move(value); }

void CGroupMetrics::SetPidsCurrent(std::optional<metrics::DataType> value) { _PidsCurrent = std::move(value); }

void CGroupMetrics::SetCpuUsageUsec(std::optional<metrics::DataType> value) { _CpuUsageUsec = std::move(value); }

void CGroupMetrics::SetCpuUserUsec(std::optional<metrics::DataType> value) { _CpuUserUsec = std::move(value); }

void CGroupMetrics::SetCpuSystemUsec(std::optional<metrics::DataType> value) { _CpuSystemUsec = std::move(value); }

void CGroupMetrics::ReadFromDirectory() {
  const auto path = _Node->GetPath();
  SetMemoryCurrent(ReadNumericFile(path / "memory.current"));
  SetPidsCurrent(ReadNumericFile(path / "pids.current"));

  auto cpuStatValues = ReadKeyValueFile(path / "cpu.stat");
  if (cpuStatValues.contains("usage_usec")) {
    SetCpuUsageUsec(cpuStatValues["usage_usec"]);
  }
  if (cpuStatValues.contains("user_usec")) {
    SetCpuUserUsec(cpuStatValues["user_usec"]);
  }
  if (cpuStatValues.contains("system_usec")) {
    SetCpuSystemUsec(cpuStatValues["system_usec"]);
  }
}

} // namespace backend::cgroupv2
