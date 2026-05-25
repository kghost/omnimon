#pragma once
#include "../metrics/SharedGauge.hpp"
#include <memory>

namespace backend::system {

class LoadAvg : public metrics::LastUpdateOwner {
public:
  LoadAvg();
  ~LoadAvg() override = default;

  void Update();

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  std::shared_ptr<metrics::SharedPublisher<double>> GetLoad1() const { return _Load1; }
  std::shared_ptr<metrics::SharedPublisher<double>> GetLoad5() const { return _Load5; }
  std::shared_ptr<metrics::SharedPublisher<double>> GetLoad15() const { return _Load15; }
  std::shared_ptr<metrics::SharedGauge> GetRunnable() const { return _Runnable; }
  std::shared_ptr<metrics::SharedGauge> GetTotalTasks() const { return _TotalTasks; }
  std::shared_ptr<metrics::SharedGauge> GetLastPid() const { return _LastPid; }

private:
  std::chrono::steady_clock::time_point _LastUpdate;
  std::shared_ptr<metrics::SharedPublisher<double>> _Load1;
  std::shared_ptr<metrics::SharedPublisher<double>> _Load5;
  std::shared_ptr<metrics::SharedPublisher<double>> _Load15;
  std::shared_ptr<metrics::SharedGauge> _Runnable;
  std::shared_ptr<metrics::SharedGauge> _TotalTasks;
  std::shared_ptr<metrics::SharedGauge> _LastPid;
};

} // namespace backend::system
