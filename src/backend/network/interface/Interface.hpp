#pragma once

#include <cstdint>
#include <string>

struct ifinfomsg;
struct ifaddrmsg;
struct rtattr;

namespace backend::network::interface {

class Interface {
  friend class InterfaceManager;

public:
  Interface(unsigned int ifIndex, char* rta, int rtaLen);
  ~Interface() = default;

  Interface(const Interface&) = delete;
  Interface& operator=(const Interface&) = delete;

  const std::string& GetName() const { return _Name; }
  unsigned int GetIfIndex() const { return _IfIndex; }
  const std::string& GetMacAddress() const { return _MacAddress; }
  const std::string& GetPrimaryIpV4() const { return _PrimaryIpV4; }
  const std::string& GetPrimaryIpV6() const { return _PrimaryIpV6; }
  const std::string& GetOperState() const { return _OperState; }
  const std::string& GetDuplex() const { return _Duplex; }
  const std::string& GetQdiscType() const { return _QdiscType; }
  int64_t GetSpeed() const { return _Speed; }
  uint32_t GetMtu() const { return _Mtu; }

  void UpdateFromNetlink(char* rta, int rtaLen);
  void UpdateAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen);
  void DeleteAddressFromNetlink(struct ifaddrmsg* ifa, char* rta, int rtaLen);

private:
  std::string _Name;
  const unsigned int _IfIndex;
  std::string _MacAddress = "00:00:00:00:00:00";
  std::string _PrimaryIpV4 = "none";
  std::string _PrimaryIpV6 = "none";
  std::string _OperState = "unknown";
  std::string _Duplex = "unknown";
  std::string _QdiscType = "none";
  int64_t _Speed = -1;
  uint32_t _Mtu = 0;

  void UpdateSpeedDuplex();
};

} // namespace backend::network::interface
