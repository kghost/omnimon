#include "ProcessGauge.hpp"

namespace backend::process {

ProcessGauge::ProcessGauge(ProcessGaugeOwner& owner) : _Owner(owner), _Value(0) {}

void ProcessGauge::SetValue(metrics::DataType value) {
  _Value = value;
  Notify();
}

std::chrono::steady_clock::time_point ProcessGauge::GetLastUpdate() const { return _Owner.GetLastUpdate(); }

metrics::DataType ProcessGauge::GetValue() const { return _Value; }

} // namespace backend::process
