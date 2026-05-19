#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

#include "../../utils/BackendTree.hpp"
#include "../metrics/SharedGauge.hpp"
#include "ProcessManager.hpp"
#include "Types.hpp"

namespace backend::process {

using GaugePtr = std::shared_ptr<backend::metrics::Gauge>;

class ProcessManager;

class Process final : public utils::TreeNodeMixin<ProcessManager, PidType, Process>,
                      public metrics::SharedPublisherOwner {
public:
  static const std::chrono::steady_clock::time_point EPOCH;

  explicit Process(ProcessManager& manager, PidType pid);
  ~Process();

  Process(const Process&) = delete;
  Process(Process&&) = delete;
  Process& operator=(const Process&) = delete;
  Process& operator=(Process&&) = delete;

  friend bool operator==(const Process& a, const Process& b) { return std::addressof(a) == std::addressof(b); }
  friend bool operator!=(const Process& a, const Process& b) { return std::addressof(a) != std::addressof(b); }

  void ParseStatFile(ProcessManager& psm);

  std::filesystem::path GetProcDirPath() const;
  bool Exists() const { return !_RemovingPublisher->GetValue(); }
  void DetachProcess();

  explicit operator PidType() const { return _Pid; }
  // TODO: read NS status (NSpid, NSpgid, NStid, etc) from /proc/[pid]/status
  PidType GetPid() const { return _Pid; }
  PidType GetPPid() const { return _Info.ppid; }
  std::string GetUser() const;
  std::chrono::steady_clock::time_point GetStartTime() const { return _StartTime; }

  GaugePtr GetState() const { return _State; }
  GaugePtr GetMem() const { return _Mem; }
  GaugePtr GetUserTime() const { return _UserTime; }
  GaugePtr GetSystemTime() const { return _SystemTime; }
  std::string GetComm() const { return _Info.comm; }

private:
  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  const PidType _Pid;

  struct ProcessInfo {
    int pid;
    int ppid;
    std::string comm;
    uid_t uid;
  };
  ProcessInfo _Info;
  std::chrono::steady_clock::time_point _StartTime;
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<metrics::SharedGauge> _State;
  std::shared_ptr<metrics::SharedGauge> _Mem;
  std::shared_ptr<metrics::SharedGauge> _UserTime;
  std::shared_ptr<metrics::SharedGauge> _SystemTime;
};

} // namespace backend::process