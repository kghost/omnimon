#pragma once

#include <chrono>
#include <memory>

#include "../metrics/Gauge.hpp"

namespace backend::process {

class ProcessGaugeOwner {
public:
  virtual ~ProcessGaugeOwner() = default;
  virtual std::chrono::steady_clock::time_point GetLastUpdate() const = 0;
};

using GaugePtr = std::shared_ptr<backend::metrics::Gauge>;

class ProcessGauge final : public backend::metrics::Gauge {
public:
  explicit ProcessGauge(ProcessGaugeOwner& owner);

  void SetValue(metrics::DataType value);

  std::chrono::steady_clock::time_point GetLastUpdate() const override;
  metrics::DataType GetValue() const override;

private:
  ProcessGaugeOwner& _Owner;
  metrics::DataType _Value;
};

} // namespace backend::process
