#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "../metrics/SharedGauge.hpp"

namespace backend::cgroupv2 {

class CGroupNode;

class CGroupMetrics : public metrics::LastUpdateOwner {
public:
  CGroupMetrics(const CGroupNode& node);
  ~CGroupMetrics() = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  struct IoStatGauges {
    std::shared_ptr<metrics::SharedGauge> ReadBytes;
    std::shared_ptr<metrics::SharedGauge> WriteBytes;
    std::shared_ptr<metrics::SharedGauge> ReadCalls;
    std::shared_ptr<metrics::SharedGauge> WriteCalls;
    std::shared_ptr<metrics::SharedGauge> DiscardBytes;
    std::shared_ptr<metrics::SharedGauge> DiscardCalls;
  };

  std::shared_ptr<metrics::SharedGauge> GetMemoryCurrent() const { return _MemoryCurrent; }
  std::shared_ptr<metrics::SharedGauge> GetPidsCurrent() const { return _PidsCurrent; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> GetCpuUsageUsec() const { return _CpuUsageUsec; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> GetCpuUserUsec() const { return _CpuUserUsec; }
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> GetCpuSystemUsec() const {
    return _CpuSystemUsec;
  }
  std::map<std::string, IoStatGauges>& GetIoStats() { return _IoStats; }

  void ReadFromDirectory(const CGroupNode& node);

private:
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<metrics::SharedGauge> _PidsCurrent;
  std::shared_ptr<metrics::SharedGauge> _MemoryCurrent;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> _CpuUsageUsec;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> _CpuUserUsec;
  std::shared_ptr<metrics::SharedPublisher<std::chrono::microseconds>> _CpuSystemUsec;
  std::map<std::string, IoStatGauges> _IoStats;
};

} // namespace backend::cgroupv2
