#pragma once

#include <chrono>

#include "Binding.hpp"
#include "DateType.hpp"

namespace backend::metrics {

class Gauge : public Publisher {
protected:
  virtual ~Gauge() = default;

public:
  virtual std::chrono::steady_clock::time_point GetLastUpdate() const = 0;
  virtual DataType GetValue() const = 0;
};

template <typename T> class BasicTypeGauge : public Gauge {
public:
  explicit BasicTypeGauge() = default;
  explicit BasicTypeGauge(T value) : _Value(value), _LastUpdate(std::chrono::steady_clock::now()) {}
  ~BasicTypeGauge() = default;

  void Update(T value) {
    if (_Value != value) {
      _Value = value;
      _LastUpdate = std::chrono::steady_clock::now();
      Notify();
    }
  }

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }
  T GetValue() const override { return _Value; }

protected:
  std::chrono::steady_clock::time_point _LastUpdate;
  T _Value;
};

using SimpleGauge = BasicTypeGauge<DataType>;

class ConstGauge : public Gauge {
public:
  explicit ConstGauge(DataType value) : _Value(value) {}
  ~ConstGauge() = default;

  std::chrono::steady_clock::time_point GetLastUpdate() const override {
    return std::chrono::steady_clock::time_point::max();
  }
  DataType GetValue() const override { return _Value; }

private:
  DataType _Value;
};

} // namespace backend::metrics
