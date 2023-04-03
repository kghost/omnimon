#pragma once

#include <list>
#include <map>
#include <memory>

#include "../../backend/process/Process.hpp"

namespace frontend::curses {

class Process : public backend::process::Process {
public:
  explicit Process(const std::filesystem::path& dir);
  ~Process();

  std::shared_ptr<Process> GetParent() const { return _Parent; }
  void SetParent(std::shared_ptr<Process> parent) { _Parent = parent; }
  void AddChild(std::shared_ptr<Process> child) { _Children[child->GetPid()] = child; }
  void RemoveChild(backend::process::PidType pid) { _Children.erase(pid); }
  enum class ChildPosition { NotLast, Last };
  static std::list<ChildPosition> GetTreePosition(std::shared_ptr<Process> me);
  ChildPosition GetChildPosition(std::shared_ptr<const Process> child) const;
  static std::list<std::shared_ptr<Process>> GetAncestors(std::shared_ptr<Process> p);

private:
  class ProcessWeakPtrOrder {
  public:
    bool operator()(const std::weak_ptr<Process>& a, const std::weak_ptr<Process>& b) const {
      return a.lock()->GetPid() < b.lock()->GetPid();
    }
  };

  std::shared_ptr<Process> _Parent;
  std::map<backend::process::PidType, std::weak_ptr<Process>> _Children;
};

} // namespace frontend::curses
