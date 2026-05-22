#include <iostream>

#include "../InterfaceManager.hpp"
#include "../InterfaceMetrics.hpp"

class WalkerCallback : public backend::network::interface::InterfaceCallback {
public:
  void OnInterfaceCreated(std::shared_ptr<backend::network::interface::Interface> iface) override {}
  void OnInterfaceDeleted(std::shared_ptr<backend::network::interface::Interface> iface) override {}
  void OnInterfaceChanged(std::shared_ptr<backend::network::interface::Interface> iface) override {}
};

int main() {
  std::cout << "=== Network Interface Metrics Walker ===" << std::endl;
  backend::events::EventLoop loop;
  WalkerCallback callback;
  backend::network::interface::InterfaceManager manager(loop, callback);
  const auto& interfaces = manager.GetInterfaces();
  if (interfaces.empty()) {
    std::cout << "No interfaces discovered." << std::endl;
    return 0;
  }
  for (const auto& [ifIndex, iface] : interfaces) {
    backend::network::interface::InterfaceMetrics metrics(*iface);
    metrics.Update();
    std::cout << "Interface: " << iface->GetName() << std::endl;
    std::cout << "  Rx Bytes:     " << metrics.GetRxBytes()->GetValue() << std::endl;
    std::cout << "  Tx Bytes:     " << metrics.GetTxBytes()->GetValue() << std::endl;
    std::cout << "  Rx Packets:   " << metrics.GetRxPackets()->GetValue() << std::endl;
    std::cout << "  Tx Packets:   " << metrics.GetTxPackets()->GetValue() << std::endl;
    std::cout << "  Rx Errors:    " << metrics.GetRxErrors()->GetValue() << std::endl;
    std::cout << "  Tx Errors:    " << metrics.GetTxErrors()->GetValue() << std::endl;
    std::cout << "  Rx Dropped:   " << metrics.GetRxDropped()->GetValue() << std::endl;
    std::cout << "  Tx Dropped:   " << metrics.GetTxDropped()->GetValue() << std::endl;
    std::cout << "  Rx Multicast: " << metrics.GetRxMulticast()->GetValue() << std::endl;
    std::cout << "  Collisions:   " << metrics.GetCollisions()->GetValue() << std::endl;
    std::cout << "-------------------------------------" << std::endl;
  }
  return 0;
}
