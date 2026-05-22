#pragma once

#include <array>
#include <cstdint>
#include <linux/if.h>
#include <optional>
#include <string>
#include <vector>

struct ifinfomsg;
struct ifaddrmsg;
struct rtattr;

namespace backend::network::interface {

class Interface {
  friend class InterfaceManager;

public:
  Interface(struct ifinfomsg* ifi, int ifIndex, char* rta, int rtaLen);
  ~Interface() = default;

  Interface(const Interface&) = delete;
  Interface& operator=(const Interface&) = delete;

  using ipV4Type = std::array<char, 4>;
  using ipV6Type = std::array<char, 16>;
  enum class DuplexType {
    Unknown,
    Half,
    Full,
  };
  enum class OperState : unsigned char {
    Unknown = IF_OPER_UNKNOWN,
    NotPresent = IF_OPER_NOTPRESENT,
    Down = IF_OPER_DOWN,
    LowerLayerDown = IF_OPER_LOWERLAYERDOWN,
    Testing = IF_OPER_TESTING,
    Dormant = IF_OPER_DORMANT,
    Up = IF_OPER_UP,
  };

  const std::string& GetName() const { return _Name; }
  int GetIfIndex() const { return _IfIndex; }
  unsigned short GetIfType() const { return _IfType; }
  const std::vector<char>& GetMacAddress() const { return _MacAddress; }
  const std::optional<ipV4Type> GetPrimaryIpV4() const { return _PrimaryIpV4; }
  const std::optional<ipV6Type> GetPrimaryIpV6() const { return _PrimaryIpV6; }
  const OperState GetOperState() const { return _OperState; }
  const DuplexType GetDuplex() const { return _Duplex; }
  const std::string& GetQdiscType() const { return _QdiscType; }
  int64_t GetSpeed() const { return _Speed; }
  uint32_t GetMtu() const { return _Mtu; }

  void UpdateFromNetlink(struct ifinfomsg* ifi, char* rta, int rtaLen);
  void UpdateAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen);
  void DeleteAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen);

private:
  std::string _Name;
  const int _IfIndex;
  unsigned short _IfType;
  OperState _OperState = OperState::Unknown;
  std::vector<char> _MacAddress;
  std::optional<ipV4Type> _PrimaryIpV4;
  std::optional<ipV6Type> _PrimaryIpV6;
  std::string _QdiscType = "none";
  uint32_t _Mtu = 0;

  // ETHTOOL data
  int64_t _Speed = -1;
  DuplexType _Duplex = DuplexType::Unknown;

  void UpdateSpeedDuplex();
};

std::string ToString(Interface::OperState state);
std::string ToString(Interface::DuplexType duplex);
std::string ToString(const std::vector<char>& mac, unsigned short ifType);
std::string ToString(const std::optional<Interface::ipV4Type>& ip);
std::string ToString(const std::optional<Interface::ipV6Type>& ip);

} // namespace backend::network::interface
