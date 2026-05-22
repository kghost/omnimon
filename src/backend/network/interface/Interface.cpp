#include "Interface.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Utils.hpp"

namespace backend::network::interface {

Interface::Interface(struct ifinfomsg* ifi, int ifIndex, char* rta, int rtaLen) : _IfIndex(ifIndex) {
  UpdateFromNetlink(ifi, rta, rtaLen);
}

void Interface::UpdateFromNetlink(struct ifinfomsg* ifi, char* rta, int rtaLen) {
  _IfType = ifi->ifi_type;
  for (struct rtattr* rta : EnumerateRtas(rta, rtaLen)) {
    switch (rta->rta_type) {
    case IFLA_IFNAME: {
      _Name = reinterpret_cast<char*>(RTA_DATA(rta));
      break;
    }
    case IFLA_ADDRESS: {
      int len = RTA_PAYLOAD(rta);
      char* mac = reinterpret_cast<char*>(RTA_DATA(rta));
      _MacAddress.assign(mac, mac + len);
      break;
    }
    case IFLA_MTU: {
      _Mtu = *reinterpret_cast<uint32_t*>(RTA_DATA(rta));
      break;
    }
    case IFLA_OPERSTATE: {
      unsigned char state = *reinterpret_cast<unsigned char*>(RTA_DATA(rta));
      _OperState = static_cast<OperState>(state);
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
    if (rta->rta_type == IFA_LOCAL || (rta->rta_type == IFA_ADDRESS && (!_PrimaryIpV4 || !_PrimaryIpV6))) {
      if (ifa->ifa_family == AF_INET) {
        ipV4Type& ip = _PrimaryIpV4.emplace();
        std::memcpy(ip.data(), RTA_DATA(rta), sizeof(ip));
      } else if (ifa->ifa_family == AF_INET6) {
        ipV6Type& ip = _PrimaryIpV6.emplace();
        std::memcpy(ip.data(), RTA_DATA(rta), sizeof(ip));
      }
    }
  }
}

void Interface::DeleteAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen) {
  for (struct rtattr* rta : EnumerateRtas(rta, rtaLen)) {
    if (rta->rta_type == IFA_LOCAL || rta->rta_type == IFA_ADDRESS) {
      if (ifa->ifa_family == AF_INET) {
        if (_PrimaryIpV4) {
          ipV4Type delIp;
          std::memcpy(delIp.data(), RTA_DATA(rta), 4);
          if (_PrimaryIpV4 == delIp) {
            _PrimaryIpV4 = std::nullopt;
          }
        }
      } else if (ifa->ifa_family == AF_INET6) {
        if (_PrimaryIpV6) {
          ipV6Type delIp;
          std::memcpy(delIp.data(), RTA_DATA(rta), 16);
          if (_PrimaryIpV6 == delIp) {
            _PrimaryIpV6 = std::nullopt;
          }
        }
      }
    }
  }
}

void Interface::UpdateSpeedDuplex() {
  _Speed = -1;
  _Duplex = DuplexType::Unknown;

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
      _Duplex = DuplexType::Half;
    } else if (ecmd->duplex == DUPLEX_FULL) {
      _Duplex = DuplexType::Full;
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
        _Duplex = DuplexType::Half;
      } else if (ep.duplex == DUPLEX_FULL) {
        _Duplex = DuplexType::Full;
      }
    }
  }
  close(fd);
}

std::string ToString(Interface::OperState state) {
  switch (state) {
  case Interface::OperState::Unknown:
    return "unknown";
  case Interface::OperState::NotPresent:
    return "notpresent";
  case Interface::OperState::Down:
    return "down";
  case Interface::OperState::LowerLayerDown:
    return "lowerlayerdown";
  case Interface::OperState::Testing:
    return "testing";
  case Interface::OperState::Dormant:
    return "dormant";
  case Interface::OperState::Up:
    return "up";
  }
  return "unknown";
}

std::string ToString(Interface::DuplexType duplex) {
  switch (duplex) {
  case Interface::DuplexType::Unknown:
    return "unknown";
  case Interface::DuplexType::Half:
    return "half";
  case Interface::DuplexType::Full:
    return "full";
  }
  return "unknown";
}

std::string ToString(const std::vector<char>& mac, unsigned short ifType) {
  if (mac.empty()) {
    return "00:00:00:00:00:00";
  }
  std::array<char, 64> buf;
  const unsigned char* uMac = reinterpret_cast<const unsigned char*>(mac.data());

  if (ifType == ARPHRD_ETHER || ifType == ARPHRD_LOOPBACK) {
    if (mac.size() >= 6) {
      std::snprintf(buf.data(), buf.size(), "%02x:%02x:%02x:%02x:%02x:%02x", uMac[0], uMac[1], uMac[2], uMac[3],
                    uMac[4], uMac[5]);
      return std::string(buf.data());
    }
  } else if (ifType == ARPHRD_INFINIBAND) {
    std::string res;
    for (size_t i = 0; i < std::min(mac.size(), size_t(20)); ++i) {
      if (i > 0) {
        res += ":";
      }
      std::snprintf(buf.data(), buf.size(), "%02x", uMac[i]);
      res += buf.data();
    }
    return res;
  }

  std::string res;
  for (size_t i = 0; i < mac.size(); ++i) {
    if (i > 0) {
      res += ":";
    }
    std::snprintf(buf.data(), buf.size(), "%02x", uMac[i]);
    res += buf.data();
  }
  return res;
}

std::string ToString(const std::optional<Interface::ipV4Type>& ip) {
  if (!ip) {
    return "none";
  }
  char buf[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, ip->data(), buf, sizeof(buf))) {
    return buf;
  }
  return "none";
}

std::string ToString(const std::optional<Interface::ipV6Type>& ip) {
  if (!ip) {
    return "none";
  }
  char buf[INET6_ADDRSTRLEN];
  if (inet_ntop(AF_INET6, ip->data(), buf, sizeof(buf))) {
    return buf;
  }
  return "none";
}

} // namespace backend::network::interface
