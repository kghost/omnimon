#pragma once

#include <filesystem>
#include <string>

#include "../metrics/Gauge.hpp"
#include "Types.hpp"

namespace backend::process {

class Process {
public:
  explicit Process(const std::filesystem::path& dir) : _ProcDirPath(dir) {}
  ~Process() = default;

  using GaugePtr = std::shared_ptr<backend::metrics::Gauge>;
  static GaugePtr GetState(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_State); }
  static GaugePtr GetMem(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_Mem); }
  static GaugePtr GetUserTime(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_UserTime); }
  static GaugePtr GetSystemTime(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_SystemTime); }
  static GaugePtr GetReadBytes(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_ReadBytes); }
  static GaugePtr GetWriteBytes(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_WriteBytes); }
  static GaugePtr GetReadCalls(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_ReadCalls); }
  static GaugePtr GetWriteCalls(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_WriteCalls); }
  static GaugePtr GetDiskReadBytes(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_DiskReadBytes); }
  static GaugePtr GetDiskWriteBytes(std::shared_ptr<Process> me) { return me->GetGauge(me, &Process::_DiskWriteBytes); }
  static GaugePtr GetDiskCancelledWriteBytes(std::shared_ptr<Process> me) {
    return me->GetGauge(me, &Process::_DiskCancelledWriteBytes);
  }

  void Update();

  // Following APIs are only available after Update
  bool Exists() const { return _Exists; }
  PidType GetPid() const { return _Info.pid; }
  PidType GetPPid() const { return _Info.ppid; }
  std::string GetCommand() const { return _Info.comm; }
  std::chrono::steady_clock::time_point GetStartTime() const { return _StartTime; }
  std::string GetCommandLine() const;

private:
  struct ProcessInfo {
    int pid = 0;
    int ppid;
    std::string comm;
  };

  void ParseStatFile();
  void ParseIoFile();

  class ProcessGauge : public metrics::Gauge {
  public:
    explicit ProcessGauge(std::shared_ptr<Process> owner) : _Owner(owner), _Value(0), _First(true) {}

    void SetValue(metrics::DataType value) {
      _Value = value;
      _First = false;
      Notify();
    }

    std::chrono::steady_clock::time_point GetLastUpdate() const override {
      if (!_First) {
        return _Owner->_LastUpdate;
      } else {
        return _Owner->_StartTime;
      }
    }
    metrics::DataType GetValue() const override { return _Value; }

  private:
    friend class Process;
    std::shared_ptr<Process> _Owner;
    metrics::DataType _Value;
    bool _First;
  };

  using GaugeMember = std::weak_ptr<ProcessGauge> Process::*;
  static std::shared_ptr<metrics::Gauge> GetGauge(std::shared_ptr<Process> me, GaugeMember member);

  const std::filesystem::path _ProcDirPath;
  std::chrono::steady_clock::time_point _LastUpdate;
  bool _Exists = false;

  ProcessInfo _Info;
  std::chrono::steady_clock::time_point _StartTime;

  // Metrics from stat file
  std::weak_ptr<ProcessGauge> _State;
  std::weak_ptr<ProcessGauge> _Mem;
  std::weak_ptr<ProcessGauge> _UserTime;
  std::weak_ptr<ProcessGauge> _SystemTime;

  // Metrics from io file
  std::weak_ptr<ProcessGauge> _ReadBytes;
  std::weak_ptr<ProcessGauge> _WriteBytes;
  std::weak_ptr<ProcessGauge> _ReadCalls;
  std::weak_ptr<ProcessGauge> _WriteCalls;
  std::weak_ptr<ProcessGauge> _DiskReadBytes;
  std::weak_ptr<ProcessGauge> _DiskWriteBytes;
  std::weak_ptr<ProcessGauge> _DiskCancelledWriteBytes;
};

} // namespace backend::process