#include "InterfaceManager.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <linux/rtnetlink.h>
#include <memory>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "../../../utils/Error.hpp"
#include "../../../utils/Lazy.hpp"
#include "Interface.hpp"
#include "Utils.hpp"

namespace backend::network::interface {

InterfaceManager::InterfaceManager(events::EventLoop& loop, InterfaceCallback& callback)
    : events::EventHandle(loop, PosixE(socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)), true), _Callback(callback) {
  struct sockaddr_nl sa;
  std::memset(&sa, 0, sizeof(sa));
  sa.nl_family = AF_NETLINK;
  sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
  PosixE(bind(_Fd, (struct sockaddr*)&sa, sizeof(sa)));

  InitializeList();
  ScheduleRead();
}

void InterfaceManager::OnRead() {
  std::vector<char> buf(32768);
  while (true) {
    ssize_t len = recv(_Fd, buf.data(), buf.size(), 0);
    if (len < 0) {
      if (errno != EWOULDBLOCK && errno != EAGAIN) {
        // Ignore or log error
      }
      break;
    }
    if (len == 0) {
      break;
    }

    for (struct nlmsghdr* nlh : EnumerateNlmsgs(buf.data(), len)) {
      if (nlh->nlmsg_type == NLMSG_DONE || nlh->nlmsg_type == NLMSG_ERROR) {
        break;
      }

      if (nlh->nlmsg_type == RTM_NEWLINK || nlh->nlmsg_type == RTM_DELLINK) {
        ProcessLinkMessage(nlh);
      } else if (nlh->nlmsg_type == RTM_NEWADDR || nlh->nlmsg_type == RTM_DELADDR) {
        ProcessAddressMessage(nlh);
      }
    }
  }

  ScheduleRead();
}

void InterfaceManager::OnWrite() { throw std::runtime_error("InterfaceManager::OnWrite"); }

void InterfaceManager::ProcessLinkMessage(struct nlmsghdr* nlh) {
  if (nlh->nlmsg_type == RTM_DELLINK) {
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*>(NLMSG_DATA(nlh));
    int ifIndex = ifi->ifi_index;
    auto it = _Interfaces.find(ifIndex);
    if (it != _Interfaces.end()) {
      auto iface = it->second;
      _Interfaces.erase(it);
      _Callback.OnInterfaceDeleted(iface);
    }
  } else if (nlh->nlmsg_type == RTM_NEWLINK) {
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*>(NLMSG_DATA(nlh));
    int ifIndex = ifi->ifi_index;

    char* rta = reinterpret_cast<char*>(ifi) + NLMSG_ALIGN(sizeof(struct ifinfomsg));
    int rtaLen = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));

    auto [it, ok] = _Interfaces.try_emplace(ifIndex, utils::Lazy<std::shared_ptr<Interface>>([&] {
                                              return std::make_shared<Interface>(ifi, ifIndex, rta, rtaLen);
                                            }));
    if (ok) {
      _Callback.OnInterfaceCreated(it->second);
    } else {
      it->second->UpdateFromNetlink(ifi, rta, rtaLen);
      _Callback.OnInterfaceChanged(it->second);
    }
  }
}

void InterfaceManager::ProcessAddressMessage(struct nlmsghdr* nlh) {
  if (nlh->nlmsg_type == RTM_NEWADDR || nlh->nlmsg_type == RTM_DELADDR) {
    struct ifaddrmsg* ifa = reinterpret_cast<struct ifaddrmsg*>(NLMSG_DATA(nlh));
    int ifIndex = ifa->ifa_index;

    char* rta = reinterpret_cast<char*>(ifa) + NLMSG_ALIGN(sizeof(struct ifaddrmsg));
    int rtaLen = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifaddrmsg));

    auto it = _Interfaces.find(ifIndex);
    if (it != _Interfaces.end()) {
      auto iface = it->second;
      if (nlh->nlmsg_type == RTM_NEWADDR) {
        iface->UpdateAddressFromNetlink(ifa, rta, rtaLen);
      } else {
        iface->DeleteAddressFromNetlink(ifa, rta, rtaLen);
      }
      _Callback.OnInterfaceChanged(iface);
    }
  }
}

void InterfaceManager::InitializeList() {
  int fd = GetFileDescriptor();
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

  struct {
    struct nlmsghdr n;
    struct ifinfomsg i;
  } req;

  std::memset(&req, 0, sizeof(req));
  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  req.n.nlmsg_type = RTM_GETLINK;
  req.i.ifi_family = AF_UNSPEC;

  if (send(fd, &req, req.n.nlmsg_len, 0) < 0) {
    fcntl(fd, F_SETFL, flags);
    return;
  }

  std::vector<char> buf(32768);
  bool done = false;

  while (!done) {
    ssize_t len = recv(fd, buf.data(), buf.size(), 0);
    if (len <= 0) {
      break;
    }

    for (struct nlmsghdr* nlh : EnumerateNlmsgs(buf.data(), len)) {
      if (nlh->nlmsg_type == NLMSG_DONE) {
        done = true;
        break;
      }
      if (nlh->nlmsg_type == NLMSG_ERROR) {
        done = true;
        break;
      }
      if (nlh->nlmsg_type == RTM_NEWLINK) {
        ProcessLinkMessage(nlh);
      }
    }
  }

  // Now query and dump addresses synchronously to populate IP addresses
  struct {
    struct nlmsghdr n;
    struct ifaddrmsg ip;
  } addrReq;
  std::memset(&addrReq, 0, sizeof(addrReq));
  addrReq.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
  addrReq.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  addrReq.n.nlmsg_type = RTM_GETADDR;
  addrReq.ip.ifa_family = AF_UNSPEC;

  if (send(fd, &addrReq, addrReq.n.nlmsg_len, 0) >= 0) {
    done = false;
    while (!done) {
      ssize_t len = recv(fd, buf.data(), buf.size(), 0);
      if (len <= 0) {
        break;
      }
      for (struct nlmsghdr* nlh : EnumerateNlmsgs(buf.data(), len)) {
        if (nlh->nlmsg_type == NLMSG_DONE) {
          done = true;
          break;
        }
        if (nlh->nlmsg_type == NLMSG_ERROR) {
          done = true;
          break;
        }
        if (nlh->nlmsg_type == RTM_NEWADDR) {
          ProcessAddressMessage(nlh);
        }
      }
    }
  }

  fcntl(fd, F_SETFL, flags);
}

} // namespace backend::network::interface
