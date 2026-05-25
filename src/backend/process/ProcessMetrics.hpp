#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "../metrics/Gauge.hpp"
#include "../metrics/SharedGauge.hpp"

namespace backend::process {

using GaugePtr = std::shared_ptr<backend::metrics::Gauge>;

class Process;

class ProcessMetrics final : public metrics::LastUpdateOwner {
public:
  explicit ProcessMetrics(Process& process);
  ~ProcessMetrics() override = default;

  void UpdateMetrics(Process& process);
  std::string GetCommandLine(Process& process) const;

  GaugePtr GetReadBytes() const { return _ReadBytes; }
  GaugePtr GetWriteBytes() const { return _WriteBytes; }
  GaugePtr GetReadCalls() const { return _ReadCalls; }
  GaugePtr GetWriteCalls() const { return _WriteCalls; }
  GaugePtr GetDiskReadBytes() const { return _DiskReadBytes; }
  GaugePtr GetDiskWriteBytes() const { return _DiskWriteBytes; }
  GaugePtr GetDiskCancelledWriteBytes() const { return _DiskCancelledWriteBytes; }

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

private:
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<metrics::SharedGauge> _ReadBytes;
  std::shared_ptr<metrics::SharedGauge> _WriteBytes;
  std::shared_ptr<metrics::SharedGauge> _ReadCalls;
  std::shared_ptr<metrics::SharedGauge> _WriteCalls;
  std::shared_ptr<metrics::SharedGauge> _DiskReadBytes;
  std::shared_ptr<metrics::SharedGauge> _DiskWriteBytes;
  std::shared_ptr<metrics::SharedGauge> _DiskCancelledWriteBytes;
};

} // namespace backend::process
