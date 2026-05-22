#include <chrono>
#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include "../InterfaceManager.hpp"
#include "../InterfaceMetrics.hpp"

namespace backend::network::interface::tests {

namespace {

class TestCallback : public InterfaceCallback {
public:
  void OnInterfaceCreated(std::shared_ptr<Interface> iface) override {}
  void OnInterfaceDeleted(std::shared_ptr<Interface> iface) override {}
  void OnInterfaceChanged(std::shared_ptr<Interface> iface) override {}
};

} // namespace

TEST(MetricsTest, InterfaceMetrics) {
  events::EventLoop loop;
  TestCallback callback;
  InterfaceManager manager(loop, callback);
  const auto& interfaces = manager.GetInterfaces();

  EXPECT_FALSE(interfaces.empty());
  for (const auto& [ifIndex, iface] : interfaces) {
    InterfaceMetrics ifaceMetrics(*iface);
    ifaceMetrics.Update();
    auto now = std::chrono::steady_clock::now();
    EXPECT_LE(ifaceMetrics.GetLastUpdate(), now);

    EXPECT_NE(ifaceMetrics.GetRxBytes(), nullptr);
    EXPECT_NE(ifaceMetrics.GetTxBytes(), nullptr);
    EXPECT_NE(ifaceMetrics.GetRxPackets(), nullptr);
    EXPECT_NE(ifaceMetrics.GetTxPackets(), nullptr);
    EXPECT_NE(ifaceMetrics.GetRxErrors(), nullptr);
    EXPECT_NE(ifaceMetrics.GetTxErrors(), nullptr);
    EXPECT_NE(ifaceMetrics.GetRxDropped(), nullptr);
    EXPECT_NE(ifaceMetrics.GetTxDropped(), nullptr);
    EXPECT_NE(ifaceMetrics.GetRxMulticast(), nullptr);
    EXPECT_NE(ifaceMetrics.GetCollisions(), nullptr);

    metrics::DataType rx = ifaceMetrics.GetRxBytes()->GetValue();
    metrics::DataType tx = ifaceMetrics.GetTxBytes()->GetValue();
    std::cout << "Interface " << iface->GetName() << " - Rx Bytes: " << rx << ", Tx Bytes: " << tx << std::endl;
  }
}

} // namespace backend::network::interface::tests
