#pragma once

#include <chrono>

#include "Gauge.hpp"

namespace backend::metrics {

class ArithmeticBase : public Gauge, public SubscriberBase {
public:
  explicit ArithmeticBase(std::shared_ptr<Gauge> operand1, std::shared_ptr<Gauge> operand2)
      : _Operand1(operand1), _Operand2(operand2) {
    _Operand1->AddSubscribe(*this);
    _Operand2->AddSubscribe(*this);
  }
  ~ArithmeticBase() override {
    _Operand2->RemoveSubscribe(*this);
    _Operand1->RemoveSubscribe(*this);
  }

  void OnUpdate() override { Notify(); }

  std::chrono::steady_clock::time_point GetLastUpdate() const override;

protected:
  const std::shared_ptr<Gauge> _Operand1;
  const std::shared_ptr<Gauge> _Operand2;
};

class Plus : public ArithmeticBase {
public:
  explicit Plus(std::shared_ptr<Gauge> operand1, std::shared_ptr<Gauge> operand2) : ArithmeticBase(operand1, operand2) {
    Plus::OnUpdate();
  }
  ~Plus() override = default;

  DataType GetValue() const override;
};

class Minus : public ArithmeticBase {
public:
  explicit Minus(std::shared_ptr<Gauge> operand1, std::shared_ptr<Gauge> operand2) : ArithmeticBase(operand1, operand2) {
    Minus::OnUpdate();
  }
  ~Minus() override = default;

  DataType GetValue() const override;
};

class Ratio : public ArithmeticBase {
public:
  explicit Ratio(std::shared_ptr<Gauge> numerator, std::shared_ptr<Gauge> denominator)
      : ArithmeticBase(numerator, denominator) {
    Ratio::OnUpdate();
  }
  ~Ratio() override = default;

  DataType GetValue() const override;
};

} // namespace backend::metrics
