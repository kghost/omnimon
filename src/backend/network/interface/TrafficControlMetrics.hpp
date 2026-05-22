#pragma once

#include <string>
#include <vector>

namespace backend::network::interface {

class Interface;

struct TcClassInfo {
  std::string Handle;
  std::string Parent;
  std::string Kind;
};

class TrafficControlMetrics {
public:
  explicit TrafficControlMetrics(const Interface& interface);
  ~TrafficControlMetrics() = default;

  TrafficControlMetrics(const TrafficControlMetrics&) = delete;
  TrafficControlMetrics& operator=(const TrafficControlMetrics&) = delete;

  void Update();
  const std::vector<TcClassInfo>& GetTcClasses() const { return _TcClasses; }

private:
  const Interface& _Interface;
  std::vector<TcClassInfo> _TcClasses;
};

} // namespace backend::network::interface
