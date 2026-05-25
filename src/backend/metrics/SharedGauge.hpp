#pragma once

#include <chrono>

#include "Binding.hpp"
#include "Gauge.hpp"

namespace backend::metrics {

class LastUpdateOwner {
public:
  virtual ~LastUpdateOwner() = default;
  virtual std::chrono::steady_clock::time_point GetLastUpdate() const = 0;
};

class SharedLastUpdate {
public:
  explicit SharedLastUpdate(LastUpdateOwner& owner) : _Owner(owner) {}
  virtual ~SharedLastUpdate() = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const { return _Owner.GetLastUpdate(); }

private:
  LastUpdateOwner& _Owner;
};

template <typename DataType> class SharedPublisher final : public SharedLastUpdate, public Publisher {
public:
  template <typename... Args>
  explicit SharedPublisher(LastUpdateOwner& owner, Args&&... args)
      : SharedLastUpdate(owner), _Metrics(std::forward<Args>(args)...) {}

  void SetValue(DataType value) {
    _Metrics = value;
    Notify();
  }
  DataType GetValue() const { return _Metrics; }

private:
  DataType _Metrics;
};

class SharedGauge : public SharedLastUpdate, public backend::metrics::Gauge {
public:
  explicit SharedGauge(LastUpdateOwner& owner) : SharedLastUpdate(owner), _Value(0) {}
  ~SharedGauge() override = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return SharedLastUpdate::GetLastUpdate(); }
  metrics::DataType GetValue() const override { return _Value; }
  void SetValue(metrics::DataType value) {
    _Value = value;
    Notify();
  }

private:
  metrics::DataType _Value;
};

} // namespace backend::metrics
