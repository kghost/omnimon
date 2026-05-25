#pragma once

#include <chrono>

#include "Binding.hpp"
#include "DateType.hpp"
#include "Gauge.hpp"

namespace backend::metrics {

class CounterSlice : public Gauge, public SubscriberBase {
public:
  explicit CounterSlice(std::shared_ptr<Gauge> target, std::chrono::steady_clock::duration period)
      : _Target(target), _Period(period), _LastTime(target->GetLastUpdate()), _LastValue(target->GetValue()) {
    _Target->AddSubscribe(*this);
    CounterSlice::OnUpdate();
  }
  ~CounterSlice() { _Target->RemoveSubscribe(*this); }

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _Target->GetLastUpdate(); }
  DataType GetValue() const override { return _CurrentValue; }

  void OnUpdate() override;
  std::chrono::steady_clock::time_point CutPoint(const std::chrono::steady_clock::time_point& time);

private:
  const std::shared_ptr<Gauge> _Target;
  const std::chrono::steady_clock::duration _Period;
  DataType _CurrentValue = 0;

  std::chrono::steady_clock::time_point _LastTime;
  DataType _LastValue = 0;
  DataType _RemainingValue = 0;
};

} // namespace backend::metrics
