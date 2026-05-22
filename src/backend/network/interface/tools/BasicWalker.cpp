#include <iostream>

#include "../InterfaceManager.hpp"

class WalkerCallback : public backend::network::interface::InterfaceCallback {
public:
  void OnInterfaceCreated(std::shared_ptr<backend::network::interface::Interface> iface) override {}
  void OnInterfaceDeleted(std::shared_ptr<backend::network::interface::Interface> iface) override {}
  void OnInterfaceChanged(std::shared_ptr<backend::network::interface::Interface> iface) override {}
};

int main() {
  std::cout << "=== Network Interface Basic Walker ===" << std::endl;
  backend::events::EventLoop loop;
  WalkerCallback callback;
  backend::network::interface::InterfaceManager manager(loop, callback);
  const auto& interfaces = manager.GetInterfaces();
  if (interfaces.empty()) {
    std::cout << "No interfaces discovered." << std::endl;
    return 0;
  }
  for (const auto& [ifIndex, iface] : interfaces) {
    std::cout << "Interface: " << iface->GetName() << std::endl;
    std::cout << "  Index:     " << iface->GetIfIndex() << std::endl;
    std::cout << "  MAC:       " << backend::network::interface::ToString(iface->GetMacAddress(), iface->GetIfType()) << std::endl;
    std::cout << "  IPv4:      " << backend::network::interface::ToString(iface->GetPrimaryIpV4()) << std::endl;
    std::cout << "  IPv6:      " << backend::network::interface::ToString(iface->GetPrimaryIpV6()) << std::endl;
    std::cout << "  OperState: " << backend::network::interface::ToString(iface->GetOperState()) << std::endl;
    std::cout << "  Duplex:    " << backend::network::interface::ToString(iface->GetDuplex()) << std::endl;
    std::cout << "  Speed:     " << iface->GetSpeed() << " Mbps" << std::endl;
    std::cout << "  MTU:       " << iface->GetMtu() << std::endl;
    std::cout << "-------------------------------------" << std::endl;
  }
  return 0;
}
