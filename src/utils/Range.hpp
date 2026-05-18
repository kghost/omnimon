#include <ranges>
#include <variant>

namespace utils {

template <typename Data, typename R1, typename R2>
class ConcatRange : public std::ranges::view_interface<ConcatRange<Data, R1, R2>> {
public:
  ConcatRange(R1 r1, R2 r2) : _R1(std::move(r1)), _R2(std::move(r2)) {}

  ConcatRange(const ConcatRange&) = default;
  ConcatRange& operator=(const ConcatRange&) = default;
  ConcatRange(ConcatRange&&) = default;
  ConcatRange& operator=(ConcatRange&&) = default;

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::remove_cvref_t<Data>;
    using difference_type = typename std::ranges::range_difference_t<R1>;
    using reference = Data;

    class End {};

    explicit iterator() : _Owner(nullptr), _Iter(End{}) {}
    explicit iterator(ConcatRange<Data, R1, R2>* owner, End) : _Owner(owner), _Iter(End{}) {}
    explicit iterator(ConcatRange<Data, R1, R2>* owner) : _Owner(owner), _Iter(owner->_R1.begin()) { satisfy(); }
    ~iterator() = default;

    iterator(const iterator&) = default;
    iterator& operator=(const iterator&) = default;
    iterator(iterator&& other) = default;
    iterator& operator=(iterator&& other) = default;

    iterator& operator++() {
      if (_Iter.index() == 1) {
        auto& it1 = std::get<typename std::ranges::iterator_t<R1>>(_Iter);
        ++it1;
        satisfy();
      } else if (_Iter.index() == 2) {
        auto& it2 = std::get<typename std::ranges::iterator_t<R2>>(_Iter);
        ++it2;
        satisfy();
      } else {
        throw std::runtime_error("End reached");
      }
      return *this;
    }

    iterator operator++(int) {
      auto temp = *this;
      ++(*this);
      return temp;
    }

    operator bool() const { return _Iter.index() != 0; }

    bool operator==(const iterator& other) const
      requires std::equality_comparable<typename std::ranges::iterator_t<R1>> &&
               std::equality_comparable<typename std::ranges::iterator_t<R2>>
    {
      if (_Iter.index() != other._Iter.index()) {
        return false;
      }
      if (_Iter.index() == 0) {
        return true;
      }
      if (_Owner != other._Owner) {
        return false;
      }
      switch (_Iter.index()) {
      case 1:
        return std::get<typename std::ranges::iterator_t<R1>>(_Iter) ==
               std::get<typename std::ranges::iterator_t<R1>>(other._Iter);
      case 2:
        return std::get<typename std::ranges::iterator_t<R2>>(_Iter) ==
               std::get<typename std::ranges::iterator_t<R2>>(other._Iter);
      default:
        return true;
      }
    }

    bool operator!=(const iterator& other) const
      requires std::equality_comparable<typename std::ranges::iterator_t<R1>> &&
               std::equality_comparable<typename std::ranges::iterator_t<R2>>
    {
      return !(*this == other);
    }

    reference operator*() const {
      if (_Iter.index() == 1) {
        return *std::get<typename std::ranges::iterator_t<R1>>(_Iter);
      } else if (_Iter.index() == 2) {
        return *std::get<typename std::ranges::iterator_t<R2>>(_Iter);
      } else {
        throw std::runtime_error("End reached");
      }
    }

  private:
    void satisfy() {
      if (_Iter.index() == 1) {
        auto& it1 = std::get<typename std::ranges::iterator_t<R1>>(_Iter);
        if (it1 == _Owner->_R1.end()) {
          _Iter = _Owner->_R2.begin();
          satisfy();
        }
      } else if (_Iter.index() == 2) {
        auto& it2 = std::get<typename std::ranges::iterator_t<R2>>(_Iter);
        if (it2 == _Owner->_R2.end()) {
          _Iter = End{};
        }
      }
    }

    ConcatRange<Data, R1, R2>* _Owner;
    std::variant<End, typename std::ranges::iterator_t<R1>, typename std::ranges::iterator_t<R2>> _Iter;
  };

  iterator begin() { return iterator(this); }
  iterator end() { return iterator(this, typename iterator::End()); }

private:
  R1 _R1;
  R2 _R2;
};

template <typename Data, typename R1, typename R2>
  requires std::ranges::range<R1> && std::ranges::range<R2> &&
           std::same_as<typename std::ranges::range_value_t<R1>, typename std::ranges::range_value_t<R2>>
auto concat(R1&& r1, R2&& r2) {
  return ConcatRange<Data, std::decay_t<R1>, std::decay_t<R2>>(std::forward<R1>(r1), std::forward<R2>(r2));
}

} // namespace utils
