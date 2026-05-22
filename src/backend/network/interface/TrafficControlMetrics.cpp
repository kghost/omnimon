#include "TrafficControlMetrics.hpp"

#include <cstdint>
#include <cstring>
#include <linux/pkt_sched.h>
#include <linux/rtnetlink.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "Interface.hpp"
#include "Utils.hpp"

namespace backend::network::interface {

namespace {

std::string FormatTcHandle(uint32_t handle) {
  if (handle == TC_H_ROOT) {
    return "root";
  }
  if (handle == TC_H_UNSPEC) {
    return "none";
  }
  uint32_t major = (handle & 0xFFFF0000) >> 16;
  uint32_t minor = handle & 0x0000FFFF;
  char buf[64];
  if (minor == 0) {
    snprintf(buf, sizeof(buf), "%x:", major);
  } else {
    snprintf(buf, sizeof(buf), "%x:%x", major, minor);
  }
  return buf;
}

} // namespace

TrafficControlMetrics::TrafficControlMetrics(const Interface& interface) : _Interface(interface) {}

void TrafficControlMetrics::Update() {
  unsigned int ifIndex = _Interface.GetIfIndex();
  _TcClasses.clear();
  if (ifIndex == 0) {
    return;
  }

  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (fd < 0) {
    return;
  }

  struct {
    struct nlmsghdr n;
    struct tcmsg t;
  } req;

  memset(&req, 0, sizeof(req));
  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  req.n.nlmsg_type = RTM_GETTCLASS;
  req.t.tcm_family = AF_UNSPEC;
  req.t.tcm_ifindex = static_cast<int>(ifIndex);

  if (send(fd, &req, req.n.nlmsg_len, 0) < 0) {
    close(fd);
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
      if (nlh->nlmsg_type == RTM_NEWTCLASS) {
        struct tcmsg* tcm = reinterpret_cast<struct tcmsg*>(NLMSG_DATA(nlh));
        if (tcm->tcm_ifindex != static_cast<int>(ifIndex)) {
          continue;
        }

        std::string kind = "";
        char* rta = reinterpret_cast<char*>(tcm) + NLMSG_ALIGN(sizeof(struct tcmsg));
        int rtaLen = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(struct tcmsg));

        for (struct rtattr* rtaItem : EnumerateRtas(rta, rtaLen)) {
          if (rtaItem->rta_type == TCA_KIND) {
            kind = reinterpret_cast<char*>(RTA_DATA(rtaItem));
          }
        }

        TcClassInfo info;
        info.Handle = FormatTcHandle(tcm->tcm_handle);
        info.Parent = FormatTcHandle(tcm->tcm_parent);
        info.Kind = kind.empty() ? "unknown" : kind;
        _TcClasses.push_back(std::move(info));
      }
    }
  }
  close(fd);
}

} // namespace backend::network::interface
