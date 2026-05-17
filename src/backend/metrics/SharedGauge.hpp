#pragma once

#include <chrono>

#include "Binding.hpp"
#include "Gauge.hpp"

namespace backend::metrics {

class SharedPublisherOwner {
public:
  virtual ~SharedPublisherOwner() = default;
  virtual std::chrono::steady_clock::time_point GetLastUpdate() const = 0;
};

class SharedPublisherBase : virtual public Publisher {
public:
  explicit SharedPublisherBase(SharedPublisherOwner& owner) : _Owner(owner) {}
  virtual ~SharedPublisherBase() = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const { return _Owner.GetLastUpdate(); }

private:
  SharedPublisherOwner& _Owner;
};

template <typename Metrics> class SharedPublisher : public SharedPublisherBase {
public:
  template <typename... Args>
  explicit SharedPublisher(SharedPublisherOwner& owner, Args&&... args)
      : SharedPublisherBase(owner), _Metrics(std::forward<Args>(args)...) {}

  void SetValue(Metrics value) {
    _Metrics = value;
    Notify();
  }
  Metrics GetValue() const { return _Metrics; }

private:
  Metrics _Metrics;
};

class SharedGauge : public SharedPublisherBase, public backend::metrics::Gauge {
public:
  explicit SharedGauge(SharedPublisherOwner& owner) : SharedPublisherBase(owner), _Value(0) {}
  ~SharedGauge() override = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return SharedPublisherBase::GetLastUpdate(); }
  metrics::DataType GetValue() const override;

  void SetValue(metrics::DataType value);

private:
  metrics::DataType _Value;
};

} // namespace backend::metrics
