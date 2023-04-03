#pragma once

#include <bpf/bpf.h>
#include <cstddef>
#include <format>
#include <iterator>
#include <stdexcept>

namespace backend::bpf {

template <typename KeyType, typename ValueType> class BpfMapIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::tuple<KeyType, ValueType>;
  using pointer = value_type*;
  using reference = value_type&;

  explicit BpfMapIterator(int fd, bool end) : _MapFd(fd), _End(true) {}

  explicit BpfMapIterator(int fd) : _MapFd(fd), _End(false) { Advance(nullptr); }

  value_type operator*() const {
    ValueType value;
    int err = bpf_map_lookup_elem(_MapFd, &_CurrentKey, &value);
    return {_CurrentKey, value};
  }

  BpfMapIterator& operator++() {
    Advance(&_CurrentKey);
    return *this;
  }

  BpfMapIterator operator++(int) {
    BpfMapIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const BpfMapIterator& a, const BpfMapIterator& b) {
    return a._MapFd == b._MapFd && ((a._End && b._End) || (a._CurrentKey == b._CurrentKey));
  }

  friend bool operator!=(const BpfMapIterator& a, const BpfMapIterator& b) { return !(a == b); }

private:
  void Advance(const KeyType* key) {
    int err = bpf_map_get_next_key(_MapFd, key, &_CurrentKey);
    if (err) {
      if (err == -ENOENT) {
        _End = true;
      } else {
        throw std::runtime_error(std::format("bpf_map_get_next_key err: {}", strerror(err)));
      }
    }
  }

  int _MapFd;
  KeyType _CurrentKey;
  bool _End = false;
};

template <typename KeyType, typename ValueType> class BpfMapRef {
public:
  explicit BpfMapRef(struct bpf_map* map) : _Map(map) {}
  ~BpfMapRef() {}

  BpfMapIterator<KeyType, ValueType> begin() { return BpfMapIterator<KeyType, ValueType>(bpf_map__fd(_Map)); }
  BpfMapIterator<KeyType, ValueType> end() { return BpfMapIterator<KeyType, ValueType>(bpf_map__fd(_Map), true); }

  void Remove(BpfMapIterator<KeyType, ValueType>& iter) {
    int err = bpf_map_delete_elem(bpf_map__fd(_Map), &iter._CurrentKey);
    if (err) {
      throw std::runtime_error(std::format("bpf_map_delete_elem failed: {}", strerror(errno)));
    }
  }

private:
  struct bpf_map* _Map;
};

} // namespace backend::bpf
