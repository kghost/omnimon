#pragma once

#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <string>

#include "../metrics/Gauge.hpp"
#include "Types.hpp"

namespace backend::process {

class Process {
public:
  static const std::chrono::steady_clock::time_point EPOCH;

  explicit Process(const std::filesystem::path& dir);
  ~Process();

  void Update();

  // Following APIs are only available after Update
  bool Exists() const { return _Exists; }
  PidType GetPid() const { return _Info.pid; }
  PidType GetPPid() const { return _Info.ppid; }
  std::string GetCommand() const { return _Info.comm; }
  std::chrono::steady_clock::time_point GetStartTime() const { return _StartTime; }
  std::string GetCommandLine() const;
  std::string GetUser() const;

  using GaugePtr = std::shared_ptr<backend::metrics::Gauge>;
  GaugePtr GetState() { return _State; }
  GaugePtr GetMem() { return _Mem; }
  GaugePtr GetUserTime() { return _UserTime; }
  GaugePtr GetSystemTime() { return _SystemTime; }
  GaugePtr GetReadBytes() { return _ReadBytes; }
  GaugePtr GetWriteBytes() { return _WriteBytes; }
  GaugePtr GetReadCalls() { return _ReadCalls; }
  GaugePtr GetWriteCalls() { return _WriteCalls; }
  GaugePtr GetDiskReadBytes() { return _DiskReadBytes; }
  GaugePtr GetDiskWriteBytes() { return _DiskWriteBytes; }
  GaugePtr GetDiskCancelledWriteBytes() { return _DiskCancelledWriteBytes; }

  std::shared_ptr<Process> GetParent() const { return _Parent; }
  void SetParent(std::shared_ptr<Process> parent) { _Parent = parent; }
  void AddChild(std::shared_ptr<Process> child) { _Children[child->GetPid()] = child; }
  void RemoveChild(PidType pid) { _Children.erase(pid); }
  enum class ChildPosition { NotLast, Last };
  static std::list<ChildPosition> GetTreePosition(std::shared_ptr<Process> me);
  ChildPosition GetChildPosition(std::shared_ptr<const Process> child) const;
  static std::list<std::shared_ptr<Process>> GetAncestors(std::shared_ptr<Process> p);

private:
  struct ProcessInfo {
    int pid = 0;
    int ppid;
    std::string comm;
    uid_t uid;
  };

  void ParseStatFile();
  void ParseIoFile();

  class ProcessGauge : public metrics::Gauge {
  public:
    explicit ProcessGauge(Process& owner) : _Owner(owner), _Value(0) {}

    void SetValue(metrics::DataType value) {
      _Value = value;
      Notify();
    }

    std::chrono::steady_clock::time_point GetLastUpdate() const override { return _Owner._LastUpdate; }
    metrics::DataType GetValue() const override { return _Value; }

  private:
    friend class Process;
    Process& _Owner;
    metrics::DataType _Value;
  };

  const std::filesystem::path _ProcDirPath;
  std::chrono::steady_clock::time_point _LastUpdate;
  bool _Exists = false;

  ProcessInfo _Info;
  std::chrono::steady_clock::time_point _StartTime;

  std::shared_ptr<Process> _Parent;
  std::map<PidType, std::weak_ptr<Process>> _Children;

  // Metrics from stat file
  std::shared_ptr<ProcessGauge> _State;
  std::shared_ptr<ProcessGauge> _Mem;
  std::shared_ptr<ProcessGauge> _UserTime;
  std::shared_ptr<ProcessGauge> _SystemTime;

  // Metrics from io file
  std::shared_ptr<ProcessGauge> _ReadBytes;
  std::shared_ptr<ProcessGauge> _WriteBytes;
  std::shared_ptr<ProcessGauge> _ReadCalls;
  std::shared_ptr<ProcessGauge> _WriteCalls;
  std::shared_ptr<ProcessGauge> _DiskReadBytes;
  std::shared_ptr<ProcessGauge> _DiskWriteBytes;
  std::shared_ptr<ProcessGauge> _DiskCancelledWriteBytes;
};

} // namespace backend::process