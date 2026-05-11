#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "../metrics/DateType.hpp"

namespace backend::cgroupv2 {

class CGroupNode;

class CGroupMetrics {
public:
  CGroupMetrics(std::shared_ptr<CGroupNode> node);
  ~CGroupMetrics() = default;

  const std::shared_ptr<CGroupNode>& GetNode() const;

  const std::optional<metrics::DataType>& GetMemoryCurrent() const;
  const std::optional<metrics::DataType>& GetPidsCurrent() const;
  const std::optional<metrics::DataType>& GetCpuUsageUsec() const;
  const std::optional<metrics::DataType>& GetCpuUserUsec() const;
  const std::optional<metrics::DataType>& GetCpuSystemUsec() const;

  void SetMemoryCurrent(std::optional<metrics::DataType> value);
  void SetPidsCurrent(std::optional<metrics::DataType> value);
  void SetCpuUsageUsec(std::optional<metrics::DataType> value);
  void SetCpuUserUsec(std::optional<metrics::DataType> value);
  void SetCpuSystemUsec(std::optional<metrics::DataType> value);

  void ReadFromDirectory();

private:
  std::shared_ptr<CGroupNode> _Node;
  std::optional<metrics::DataType> _MemoryCurrent;
  std::optional<metrics::DataType> _PidsCurrent;
  std::optional<metrics::DataType> _CpuUsageUsec;
  std::optional<metrics::DataType> _CpuUserUsec;
  std::optional<metrics::DataType> _CpuSystemUsec;
};

} // namespace backend::cgroupv2
