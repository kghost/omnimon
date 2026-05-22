#include <iostream>

#include "../InterfaceManager.hpp"
#include "../TrafficControlMetrics.hpp"

class WalkerCallback : public backend::network::interface::InterfaceCallback {
public:
  void OnInterfaceCreated(std::shared_ptr<backend::network::interface::Interface> iface) override {}
  void OnInterfaceDeleted(std::shared_ptr<backend::network::interface::Interface> iface) override {}
  void OnInterfaceChanged(std::shared_ptr<backend::network::interface::Interface> iface) override {}
};

int main() {
  std::cout << "=== Network Interface Traffic Control Walker ===" << std::endl;
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
    std::cout << "  Qdisc Type: " << iface->GetQdiscType() << std::endl;

    backend::network::interface::TrafficControlMetrics tcMetrics(*iface);
    tcMetrics.Update();

    const auto& tcClasses = tcMetrics.GetTcClasses();
    if (tcClasses.empty()) {
      std::cout << "  TC Classes: None discovered." << std::endl;
    } else {
      std::cout << "  TC Classes:" << std::endl;
      for (const auto& cls : tcClasses) {
        std::cout << "    - Handle: " << cls.Handle << ", Parent: " << cls.Parent << ", Kind: " << cls.Kind
                  << std::endl;
      }
    }
    std::cout << "-------------------------------------" << std::endl;
  }
  return 0;
}
