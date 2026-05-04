#pragma once

#include "Binding.hpp"

namespace backend::metrics {

template <typename T> class SimplePublisher : public Publisher {
public:
  explicit SimplePublisher() = default;
  explicit SimplePublisher(T value) : _Value(value) {}
  ~SimplePublisher() = default;

  void Update(T value) {
    if (_Value != value) {
      _Value = value;
      Notify();
    }
  }

  T GetValue() const { return _Value; }

protected:
  T _Value;
};

} // namespace backend::metrics