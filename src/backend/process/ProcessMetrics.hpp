#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

#include "ProcessGauge.hpp"
#include "Types.hpp"

namespace backend::process {

class Process;

class ProcessMetrics final : public ProcessGaugeOwner {
public:
  explicit ProcessMetrics(std::shared_ptr<Process> process);
  ~ProcessMetrics() override = default;

  void UpdateMetrics();
  std::shared_ptr<Process> GetProcess() const { return _Process; }

  GaugePtr GetReadBytes() const { return _ReadBytes; }
  GaugePtr GetWriteBytes() const { return _WriteBytes; }
  GaugePtr GetReadCalls() const { return _ReadCalls; }
  GaugePtr GetWriteCalls() const { return _WriteCalls; }
  GaugePtr GetDiskReadBytes() const { return _DiskReadBytes; }
  GaugePtr GetDiskWriteBytes() const { return _DiskWriteBytes; }
  GaugePtr GetDiskCancelledWriteBytes() const { return _DiskCancelledWriteBytes; }
  std::string GetCommandLine() const;

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

private:
  std::shared_ptr<Process> _Process;
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<ProcessGauge> _ReadBytes;
  std::shared_ptr<ProcessGauge> _WriteBytes;
  std::shared_ptr<ProcessGauge> _ReadCalls;
  std::shared_ptr<ProcessGauge> _WriteCalls;
  std::shared_ptr<ProcessGauge> _DiskReadBytes;
  std::shared_ptr<ProcessGauge> _DiskWriteBytes;
  std::shared_ptr<ProcessGauge> _DiskCancelledWriteBytes;
};

} // namespace backend::process
