#pragma once

#include <type_traits>
#include <utility>

namespace utils {

template <typename Target, typename Fn>
  requires std::is_invocable_r_v<Target, Fn>
class LazyInitializer {
public:
  explicit LazyInitializer(Fn&& fn) : _Fn(std::forward<Fn>(fn)) {}
  operator Target() const { return _Fn(); }

private:
  Fn _Fn;
};

template <typename Target, typename Fn>
  requires std::is_invocable_r_v<Target, Fn>
auto Lazy(Fn&& fn) {
  return LazyInitializer<Target, Fn>(std::forward<Fn>(fn));
}

} // namespace utils
