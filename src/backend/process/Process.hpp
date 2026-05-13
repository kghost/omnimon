#pragma once

#include <chrono>
#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <string>

#include "ProcessGauge.hpp"
#include "Types.hpp"

namespace backend::process {

class Process final : public std::enable_shared_from_this<Process>, public ProcessGaugeOwner {
public:
  static const std::chrono::steady_clock::time_point EPOCH;

  explicit Process(const std::filesystem::path& dir);
  ~Process();

  void ParseStatFile();

  const std::filesystem::path& GetProcDirPath() const { return _ProcDirPath; }
  bool Exists() const { return _Exists; }
  void SetExists(bool exists) { _Exists = exists; }

  PidType GetPid() const { return _Info.pid; }
  PidType GetPPid() const { return _Info.ppid; }
  std::string GetUser() const;
  std::chrono::steady_clock::time_point GetStartTime() const { return _StartTime; }

  GaugePtr GetState() const { return _State; }
  GaugePtr GetMem() const { return _Mem; }
  GaugePtr GetUserTime() const { return _UserTime; }
  GaugePtr GetSystemTime() const { return _SystemTime; }
  std::string GetComm() const { return _Info.comm; }

  std::shared_ptr<Process> GetParent() const { return _Parent; }
  void SetParent(std::shared_ptr<Process> parent) { _Parent = parent; }
  void AddChild(std::shared_ptr<Process> child) { _Children[child->GetPid()] = child; }
  void RemoveChild(PidType pid) { _Children.erase(pid); }
  enum class ChildPosition { NotLast, Last };
  static std::list<ChildPosition> GetTreePosition(std::shared_ptr<Process> me);
  ChildPosition GetChildPosition(std::shared_ptr<const Process> child) const;
  static std::list<std::shared_ptr<Process>> GetAncestors(std::shared_ptr<Process> p);

private:
  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  const std::filesystem::path _ProcDirPath;
  bool _Exists = false;

  struct ProcessInfo {
    int pid = 0;
    int ppid;
    std::string comm;
    uid_t uid;
  };
  ProcessInfo _Info;
  std::chrono::steady_clock::time_point _StartTime;
  std::chrono::steady_clock::time_point _LastUpdate;

  std::shared_ptr<ProcessGauge> _State;
  std::shared_ptr<ProcessGauge> _Mem;
  std::shared_ptr<ProcessGauge> _UserTime;
  std::shared_ptr<ProcessGauge> _SystemTime;

  std::shared_ptr<Process> _Parent;
  std::map<PidType, std::weak_ptr<Process>> _Children;
};

} // namespace backend::process