#include "Binding.hpp"

namespace backend::metrics {

void Publisher::Notify() {
  for (auto& subscriber : _Subscribers) {
    subscriber.get().OnUpdate();
  }
}

} // namespace backend::metrics
