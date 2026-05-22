#include "Interface.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "Utils.hpp"

namespace backend::network::interface {

Interface::Interface(unsigned int ifIndex, char* rta, int rtaLen) : _IfIndex(ifIndex) {
  UpdateFromNetlink(rta, rtaLen);
}

void Interface::UpdateFromNetlink(char* rta, int rtaLen) {
  for (struct rtattr* rta : EnumerateRtas(rta, rtaLen)) {
    switch (rta->rta_type) {
    case IFLA_IFNAME: {
      _Name = reinterpret_cast<char*>(RTA_DATA(rta));
      break;
    }
    case IFLA_ADDRESS: {
      int len = RTA_PAYLOAD(rta);
      unsigned char* mac = reinterpret_cast<unsigned char*>(RTA_DATA(rta));
      if (len == 6) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4],
                      mac[5]);
        _MacAddress = buf;
      } else if (len > 0) {
        _MacAddress = "00:00:00:00:00:00";
      }
      break;
    }
    case IFLA_MTU: {
      _Mtu = *reinterpret_cast<uint32_t*>(RTA_DATA(rta));
      break;
    }
    case IFLA_OPERSTATE: {
      unsigned char state = *reinterpret_cast<unsigned char*>(RTA_DATA(rta));
      switch (state) {
      case 0:
        _OperState = "unknown";
        break; // IF_OPER_UNKNOWN
      case 1:
        _OperState = "notpresent";
        break; // IF_OPER_NOTPRESENT
      case 2:
        _OperState = "down";
        break; // IF_OPER_DOWN
      case 3:
        _OperState = "lowerlayerdown";
        break; // IF_OPER_LOWERLAYERDOWN
      case 4:
        _OperState = "testing";
        break; // IF_OPER_TESTING
      case 5:
        _OperState = "dormant";
        break; // IF_OPER_DORMANT
      case 6:
        _OperState = "up";
        break; // IF_OPER_UP
      default:
        _OperState = "unknown";
        break;
      }
      break;
    }
    case IFLA_QDISC: {
      _QdiscType = reinterpret_cast<char*>(RTA_DATA(rta));
      break;
    }
    }
  }

  UpdateSpeedDuplex();
}

void Interface::UpdateAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen) {
  for (struct rtattr* rta : EnumerateRtas(rta, rtaLen)) {
    if (rta->rta_type == IFA_LOCAL ||
        (rta->rta_type == IFA_ADDRESS && (_PrimaryIpV4 == "none" || _PrimaryIpV6 == "none"))) {
      char ipStr[INET6_ADDRSTRLEN];
      if (ifa->ifa_family == AF_INET) {
        if (inet_ntop(AF_INET, RTA_DATA(rta), ipStr, sizeof(ipStr))) {
          _PrimaryIpV4 = ipStr;
        }
      } else if (ifa->ifa_family == AF_INET6) {
        if (inet_ntop(AF_INET6, RTA_DATA(rta), ipStr, sizeof(ipStr))) {
          _PrimaryIpV6 = ipStr;
        }
      }
    }
  }
}

void Interface::DeleteAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen) {
  for (struct rtattr* rta : EnumerateRtas(rta, rtaLen)) {
    if (rta->rta_type == IFA_LOCAL || rta->rta_type == IFA_ADDRESS) {
      char ipStr[INET6_ADDRSTRLEN];
      if (ifa->ifa_family == AF_INET) {
        if (inet_ntop(AF_INET, RTA_DATA(rta), ipStr, sizeof(ipStr))) {
          if (_PrimaryIpV4 == ipStr) {
            _PrimaryIpV4 = "none";
          }
        }
      } else if (ifa->ifa_family == AF_INET6) {
        if (inet_ntop(AF_INET6, RTA_DATA(rta), ipStr, sizeof(ipStr))) {
          if (_PrimaryIpV6 == ipStr) {
            _PrimaryIpV6 = "none";
          }
        }
      }
    }
  }
}

void Interface::UpdateSpeedDuplex() {
  _Speed = -1;
  _Duplex = "unknown";

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    return;
  }

  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, _Name.c_str(), IFNAMSIZ - 1);

  // Try modern ETHTOOL_GLINKSETTINGS first
  constexpr size_t kBufSize = sizeof(struct ethtool_link_settings) + 3 * 127 * sizeof(__u32);
  std::vector<char> ecmdBuf(kBufSize, 0);
  struct ethtool_link_settings* ecmd = reinterpret_cast<struct ethtool_link_settings*>(ecmdBuf.data());
  ecmd->cmd = ETHTOOL_GLINKSETTINGS;
  ifr.ifr_data = reinterpret_cast<char*>(ecmd);

  if (ioctl(fd, SIOCETHTOOL, &ifr) == 0) {
    if (ecmd->speed != static_cast<__u32>(SPEED_UNKNOWN)) {
      _Speed = ecmd->speed;
    }
    if (ecmd->duplex == DUPLEX_HALF) {
      _Duplex = "half";
    } else if (ecmd->duplex == DUPLEX_FULL) {
      _Duplex = "full";
    }
  } else {
    // Fallback to legacy ETHTOOL_GSET
    struct ethtool_cmd ep;
    std::memset(&ep, 0, sizeof(ep));
    ep.cmd = ETHTOOL_GSET;
    ifr.ifr_data = reinterpret_cast<char*>(&ep);

    if (ioctl(fd, SIOCETHTOOL, &ifr) == 0) {
      uint32_t speed = (ep.speed_hi << 16) | ep.speed;
      if (speed != static_cast<uint32_t>(SPEED_UNKNOWN)) {
        _Speed = speed;
      }
      if (ep.duplex == DUPLEX_HALF) {
        _Duplex = "half";
      } else if (ep.duplex == DUPLEX_FULL) {
        _Duplex = "full";
      }
    }
  }
  close(fd);
}

} // namespace backend::network::interface
