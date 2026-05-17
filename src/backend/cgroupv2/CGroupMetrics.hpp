#pragma once

#include <chrono>
#include <memory>

#include "../metrics/SharedGauge.hpp"

namespace backend::cgroupv2 {

class CGroupNode;

class CGroupMetrics : public metrics::SharedPublisherOwner {
public:
  CGroupMetrics(CGroupNode& node);
  ~CGroupMetrics() = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  std::shared_ptr<metrics::SharedGauge> GetMemoryCurrent() const { return _MemoryCurrent; }
  std::shared_ptr<metrics::SharedGauge> GetPidsCurrent() const { return _PidsCurrent; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> GetCpuUsageUsec() const { return _CpuUsageUsec; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> GetCpuUserUsec() const { return _CpuUserUsec; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> GetCpuSystemUsec() const {
    return _CpuSystemUsec;
  }

  void ReadFromDirectory();

private:
  CGroupNode& _Node;
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<metrics::SharedGauge> _MemoryCurrent;
  std::shared_ptr<metrics::SharedGauge> _PidsCurrent;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> _CpuUsageUsec;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> _CpuUserUsec;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> _CpuSystemUsec;
};

} // namespace backend::cgroupv2
