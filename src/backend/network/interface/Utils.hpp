#pragma once

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace backend::network::interface {

class RtaIterator {
public:
  RtaIterator(struct rtattr* rta, int len) : _Rta(rta), _Len(len) {
    if (!_Rta || !RTA_OK(_Rta, _Len)) {
      _Rta = nullptr;
      _Len = 0;
    }
  }

  struct rtattr* operator*() const { return _Rta; }

  RtaIterator& operator++() {
    if (_Rta && RTA_OK(_Rta, _Len)) {
      _Rta = RTA_NEXT(_Rta, _Len);
    }
    if (!_Rta || !RTA_OK(_Rta, _Len)) {
      _Rta = nullptr;
      _Len = 0;
    }
    return *this;
  }

  bool operator==(const RtaIterator& other) const { return _Rta == other._Rta; }

  bool operator!=(const RtaIterator& other) const { return !(*this == other); }

private:
  struct rtattr* _Rta;
  int _Len;
};

class RtaRange {
public:
  RtaRange(struct rtattr* rta, int len) : _Rta(rta), _Len(len) {}

  RtaIterator begin() const { return RtaIterator(_Rta, _Len); }
  RtaIterator end() const { return RtaIterator(nullptr, 0); }

private:
  struct rtattr* _Rta;
  int _Len;
};

inline RtaRange EnumerateRtas(char* rta, int len) { return RtaRange(reinterpret_cast<struct rtattr*>(rta), len); }

class NlmsgIterator {
public:
  NlmsgIterator(struct nlmsghdr* nlh, int len) : _Nlh(nlh), _Len(len) {
    if (!_Nlh || !NLMSG_OK(_Nlh, _Len)) {
      _Nlh = nullptr;
      _Len = 0;
    }
  }

  struct nlmsghdr* operator*() const { return _Nlh; }

  NlmsgIterator& operator++() {
    if (_Nlh && NLMSG_OK(_Nlh, _Len)) {
      _Nlh = NLMSG_NEXT(_Nlh, _Len);
    }
    if (!_Nlh || !NLMSG_OK(_Nlh, _Len)) {
      _Nlh = nullptr;
      _Len = 0;
    }
    return *this;
  }

  bool operator==(const NlmsgIterator& other) const { return _Nlh == other._Nlh; }

  bool operator!=(const NlmsgIterator& other) const { return !(*this == other); }

private:
  struct nlmsghdr* _Nlh;
  int _Len;
};

class NlmsgRange {
public:
  NlmsgRange(struct nlmsghdr* nlh, int len) : _Nlh(nlh), _Len(len) {}

  NlmsgIterator begin() const { return NlmsgIterator(_Nlh, _Len); }
  NlmsgIterator end() const { return NlmsgIterator(nullptr, 0); }

private:
  struct nlmsghdr* _Nlh;
  int _Len;
};

inline NlmsgRange EnumerateNlmsgs(char* nlh, int len) {
  return NlmsgRange(reinterpret_cast<struct nlmsghdr*>(nlh), len);
}

} // namespace backend::network::interface