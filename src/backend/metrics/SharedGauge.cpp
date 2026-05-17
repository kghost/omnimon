#include "SharedGauge.hpp"

namespace backend::metrics {

void SharedGauge::SetValue(metrics::DataType value) {
  _Value = value;
  Notify();
}

metrics::DataType SharedGauge::GetValue() const { return _Value; }

} // namespace backend::metrics
