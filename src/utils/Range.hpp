#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
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

    explicit iterator() : _Owner(nullptr), _Iter(std::in_place_index<0>, End{}) {}
    explicit iterator(ConcatRange<Data, R1, R2>* owner, End) : _Owner(owner), _Iter(std::in_place_index<0>, End{}) {}
    explicit iterator(ConcatRange<Data, R1, R2>* owner)
        : _Owner(owner), _Iter(std::in_place_index<1>, owner->_R1.begin()) {
      satisfy();
    }
    ~iterator() = default;

    iterator(const iterator&) = default;
    iterator& operator=(const iterator&) = default;
    iterator(iterator&& other) = default;
    iterator& operator=(iterator&& other) = default;

    iterator& operator++() {
      if (_Iter.index() == 1) {
        auto& it1 = std::get<1>(_Iter);
        ++it1;
        satisfy();
      } else if (_Iter.index() == 2) {
        auto& it2 = std::get<2>(_Iter);
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
        return std::get<1>(_Iter) == std::get<1>(other._Iter);
      case 2:
        return std::get<2>(_Iter) == std::get<2>(other._Iter);
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
        return *std::get<1>(_Iter);
      } else if (_Iter.index() == 2) {
        return *std::get<2>(_Iter);
      } else {
        throw std::runtime_error("End reached");
      }
    }

  private:
    void satisfy() {
      if (_Iter.index() == 1) {
        auto& it1 = std::get<1>(_Iter);
        if (it1 == _Owner->_R1.end()) {
          _Iter.template emplace<2>(_Owner->_R2.begin());
          satisfy();
        }
      } else if (_Iter.index() == 2) {
        auto& it2 = std::get<2>(_Iter);
        if (it2 == _Owner->_R2.end()) {
          _Iter.template emplace<0>(End{});
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

template <typename Ref> class AnyIterator {
private:
  struct IteratorConcept {
    virtual ~IteratorConcept() = default;
    virtual std::unique_ptr<IteratorConcept> Clone() const = 0;
    virtual Ref Dereference() const = 0;
    virtual void Increment() = 0;
    virtual bool Equals(const IteratorConcept& other) const = 0;
    virtual bool IsEnd() const = 0;
  };

  template <typename It, typename Sent> struct IteratorModel : public IteratorConcept {
    It Current;
    Sent Sentinel;

    explicit IteratorModel(It current, Sent sentinel) : Current(std::move(current)), Sentinel(std::move(sentinel)) {}

    std::unique_ptr<IteratorConcept> Clone() const override {
      return std::make_unique<IteratorModel<It, Sent>>(Current, Sentinel);
    }

    Ref Dereference() const override { return *Current; }

    void Increment() override { ++Current; }

    bool Equals(const IteratorConcept& other) const override {
      if constexpr (std::equality_comparable<It>) {
        auto* otherModel = dynamic_cast<const IteratorModel<It, Sent>*>(&other);
        if (otherModel != nullptr) {
          return Current == otherModel->Current;
        }
      }
      return false;
    }

    bool IsEnd() const override { return Current == Sentinel; }
  };

  std::unique_ptr<IteratorConcept> _Impl;

public:
  using iterator_category = std::input_iterator_tag;
  using value_type = std::remove_cvref_t<Ref>;
  using difference_type = std::ptrdiff_t;
  using reference = Ref;

  AnyIterator() : _Impl(nullptr) {}

  template <typename It, typename Sent>
  explicit AnyIterator(It it, Sent sent)
      : _Impl(std::make_unique<IteratorModel<It, Sent>>(std::move(it), std::move(sent))) {}

  AnyIterator(const AnyIterator& other) : _Impl(other._Impl ? other._Impl->Clone() : nullptr) {}

  AnyIterator& operator=(const AnyIterator& other) {
    if (this != &other) {
      _Impl = other._Impl ? other._Impl->Clone() : nullptr;
    }
    return *this;
  }

  AnyIterator(AnyIterator&&) noexcept = default;
  AnyIterator& operator=(AnyIterator&&) noexcept = default;

  ~AnyIterator() = default;

  reference operator*() const { return _Impl->Dereference(); }

  AnyIterator& operator++() {
    _Impl->Increment();
    return *this;
  }

  AnyIterator operator++(int) {
    auto temp = *this;
    _Impl->Increment();
    return temp;
  }

  bool IsEnd() const { return _Impl == nullptr || _Impl->IsEnd(); }

  bool operator==(const AnyIterator& other) const {
    bool lhsEnd = IsEnd();
    bool rhsEnd = other.IsEnd();
    if (lhsEnd && rhsEnd) {
      return true;
    }
    if (lhsEnd || rhsEnd) {
      return false;
    }
    return _Impl->Equals(*other._Impl);
  }

  bool operator!=(const AnyIterator& other) const { return !(*this == other); }
};

template <typename Ref> class AnyView : public std::ranges::view_interface<AnyView<Ref>> {
private:
  struct ViewConcept {
    virtual ~ViewConcept() = default;
    virtual std::unique_ptr<ViewConcept> Clone() const = 0;
    virtual AnyIterator<Ref> Begin() = 0;
    virtual AnyIterator<Ref> End() = 0;
  };

  template <typename V> struct ViewModel : public ViewConcept {
    V View;

    explicit ViewModel(V view) : View(std::move(view)) {}

    std::unique_ptr<ViewConcept> Clone() const override { return std::make_unique<ViewModel<V>>(View); }

    AnyIterator<Ref> Begin() override { return AnyIterator<Ref>(std::ranges::begin(View), std::ranges::end(View)); }

    AnyIterator<Ref> End() override { return AnyIterator<Ref>(); }
  };

  std::unique_ptr<ViewConcept> _Impl;

public:
  AnyView() = default;

  template <typename R>
    requires std::ranges::range<R> && std::convertible_to<std::ranges::range_reference_t<R>, Ref> &&
             (!std::same_as<std::decay_t<R>, AnyView>)
  explicit AnyView(R&& r) {
    using V = decltype(std::views::all(std::forward<R>(r)));
    _Impl = std::make_unique<ViewModel<V>>(std::views::all(std::forward<R>(r)));
  }

  AnyView(const AnyView& other) : _Impl(other._Impl ? other._Impl->Clone() : nullptr) {}

  AnyView& operator=(const AnyView& other) {
    if (this != &other) {
      _Impl = other._Impl ? other._Impl->Clone() : nullptr;
    }
    return *this;
  }

  AnyView(AnyView&&) noexcept = default;
  AnyView& operator=(AnyView&&) noexcept = default;

  ~AnyView() = default;

  AnyIterator<Ref> begin() {
    if (_Impl == nullptr) {
      return AnyIterator<Ref>();
    }
    return _Impl->Begin();
  }

  AnyIterator<Ref> end() { return AnyIterator<Ref>(); }

  AnyIterator<Ref> begin() const {
    if (_Impl == nullptr) {
      return AnyIterator<Ref>();
    }
    return const_cast<AnyView*>(this)->_Impl->Begin();
  }

  AnyIterator<Ref> end() const { return AnyIterator<Ref>(); }
};

} // namespace utils
