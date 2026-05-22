#include "InterfaceMetrics.hpp"

#include <chrono>
#include <cstring>
#include <linux/rtnetlink.h>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "Interface.hpp"
#include "Utils.hpp"

namespace backend::network::interface {

InterfaceMetrics::InterfaceMetrics(Interface& interface)
    : _Interface(interface), _LastUpdate(std::chrono::steady_clock::now()),
      _RxBytes(std::make_shared<metrics::SharedGauge>(*this)), _TxBytes(std::make_shared<metrics::SharedGauge>(*this)),
      _RxPackets(std::make_shared<metrics::SharedGauge>(*this)),
      _TxPackets(std::make_shared<metrics::SharedGauge>(*this)),
      _RxErrors(std::make_shared<metrics::SharedGauge>(*this)),
      _TxErrors(std::make_shared<metrics::SharedGauge>(*this)),
      _RxDropped(std::make_shared<metrics::SharedGauge>(*this)),
      _TxDropped(std::make_shared<metrics::SharedGauge>(*this)),
      _RxMulticast(std::make_shared<metrics::SharedGauge>(*this)),
      _Collisions(std::make_shared<metrics::SharedGauge>(*this)) {}

void InterfaceMetrics::Update() {
  _LastUpdate = std::chrono::steady_clock::now();

  unsigned int ifIndex = _Interface.GetIfIndex();
  if (ifIndex == 0) {
    return;
  }

  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (fd >= 0) {
    struct {
      struct nlmsghdr n;
      struct ifinfomsg i;
    } req;

    std::memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_GETLINK;
    req.i.ifi_family = AF_UNSPEC;
    req.i.ifi_index = static_cast<int>(ifIndex);

    if (send(fd, &req, req.n.nlmsg_len, 0) >= 0) {
      std::vector<char> buf(8192);
      ssize_t len = recv(fd, buf.data(), buf.size(), 0);
      if (len > 0) {
        for (struct nlmsghdr* nlh : EnumerateNlmsgs(buf.data(), len)) {
          if (nlh->nlmsg_type == RTM_NEWLINK) {
            struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*>(NLMSG_DATA(nlh));
            if (ifi->ifi_index == static_cast<int>(ifIndex)) {
              char* rta = reinterpret_cast<char*>(ifi) + NLMSG_ALIGN(sizeof(struct ifinfomsg));
              int rtaLen = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
              UpdateFromNetlink(ifi, rta, rtaLen);
              break;
            }
          }
        }
      }
    }
    close(fd);
  }
}

void InterfaceMetrics::UpdateFromNetlink(struct ifinfomsg* ifi, char* rta, int rtaLen) {
  _LastUpdate = std::chrono::steady_clock::now();
  _Interface.UpdateFromNetlink(rta, rtaLen);

  bool hasStats = false;
  for (struct rtattr* rtaPtr : EnumerateRtas(rta, rtaLen)) {
    switch (rtaPtr->rta_type) {
    case IFLA_STATS64: {
      struct rtnl_link_stats64* stats = reinterpret_cast<struct rtnl_link_stats64*>(RTA_DATA(rtaPtr));
      _RxBytes->SetValue(stats->rx_bytes);
      _TxBytes->SetValue(stats->tx_bytes);
      _RxPackets->SetValue(stats->rx_packets);
      _TxPackets->SetValue(stats->tx_packets);
      _RxErrors->SetValue(stats->rx_errors);
      _TxErrors->SetValue(stats->tx_errors);
      _RxDropped->SetValue(stats->rx_dropped);
      _TxDropped->SetValue(stats->tx_dropped);
      _RxMulticast->SetValue(stats->multicast);
      _Collisions->SetValue(stats->collisions);
      hasStats = true;
      break;
    }
    case IFLA_STATS: {
      if (!hasStats) {
        struct rtnl_link_stats* stats = reinterpret_cast<struct rtnl_link_stats*>(RTA_DATA(rtaPtr));
        _RxBytes->SetValue(stats->rx_bytes);
        _TxBytes->SetValue(stats->tx_bytes);
        _RxPackets->SetValue(stats->rx_packets);
        _TxPackets->SetValue(stats->tx_packets);
        _RxErrors->SetValue(stats->rx_errors);
        _TxErrors->SetValue(stats->tx_errors);
        _RxDropped->SetValue(stats->rx_dropped);
        _TxDropped->SetValue(stats->tx_dropped);
        _RxMulticast->SetValue(stats->multicast);
        _Collisions->SetValue(stats->collisions);
      }
      break;
    }
    }
  }
}

} // namespace backend::network::interface
