#include "Arithmetic.hpp"

namespace backend::metrics {

std::chrono::steady_clock::time_point ArithmeticBase::GetLastUpdate() const {
  return std::min(_Operand1->GetLastUpdate(), _Operand2->GetLastUpdate());
}

DataType Plus::GetValue() const { return _Operand1->GetValue() + _Operand2->GetValue(); }
DataType Minus::GetValue() const { return _Operand1->GetValue() - _Operand2->GetValue(); }

DataType Ratio::GetValue() const {
  auto numerator = _Operand1->GetValue();
  auto denominator = _Operand2->GetValue();
  if (denominator == 0) {
    return 0;
  }
  return static_cast<double>(numerator) / denominator * 10000;
}

} // namespace backend::metrics
