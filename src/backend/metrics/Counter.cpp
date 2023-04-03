#include "Counter.hpp"

namespace backend::metrics {

void CounterSlice::OnUpdate() {
  std::chrono::steady_clock::time_point now = _Target->GetLastUpdate();
  if (now <= _LastTime) {
    return;
  }

  std::chrono::steady_clock::time_point lastCut = CutPoint(_LastTime);
  std::chrono::steady_clock::time_point curCut = CutPoint(now);
  if (curCut <= lastCut) {
    return;
  }

  std::chrono::milliseconds before = std::chrono::duration_cast<std::chrono::milliseconds>(curCut - _LastTime);
  std::chrono::milliseconds total = std::chrono::duration_cast<std::chrono::milliseconds>(now - _LastTime);
  double ratioBefore = static_cast<double>(before.count()) / total.count();

  DataType curValue = _Target->GetValue();
  DataType beforeValue = (curValue - _LastValue) * ratioBefore;
  DataType afterValue = curValue - _LastValue - beforeValue;

  size_t ticks = (curCut - lastCut) / _Period;
  _CurrentValue = (beforeValue + _RemainingValue) / ticks;

  _LastTime = now;
  _LastValue = curValue;
  _RemainingValue = afterValue;

  Notify();
}

std::chrono::steady_clock::time_point CounterSlice::CutPoint(const std::chrono::steady_clock::time_point& time) {
  int64_t timeMilli = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
  int64_t durationMilli = std::chrono::duration_cast<std::chrono::milliseconds>(_Period).count();
  return std::chrono::steady_clock::time_point(std::chrono::milliseconds(timeMilli / durationMilli * durationMilli));
}

} // namespace backend::metrics
