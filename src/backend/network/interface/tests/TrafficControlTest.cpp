#include <iostream>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "../InterfaceManager.hpp"
#include "../TrafficControlMetrics.hpp"

namespace backend::network::interface::tests {

namespace {

class TestCallback : public InterfaceCallback {
public:
  void OnInterfaceCreated(std::shared_ptr<Interface> iface) override {}
  void OnInterfaceDeleted(std::shared_ptr<Interface> iface) override {}
  void OnInterfaceChanged(std::shared_ptr<Interface> iface) override {}
};

} // namespace

TEST(TrafficControlTest, TrafficControlMetrics) {
  events::EventLoop loop;
  TestCallback callback;
  InterfaceManager manager(loop, callback);
  const auto& interfaces = manager.GetInterfaces();

  EXPECT_FALSE(interfaces.empty());
  for (const auto& [ifIndex, iface] : interfaces) {
    std::string qdisc = iface->GetQdiscType();
    EXPECT_FALSE(qdisc.empty());
    std::cout << "Interface " << iface->GetName() << " - Qdisc: " << qdisc << std::endl;

    TrafficControlMetrics tcMetrics(*iface);
    tcMetrics.Update();

    const auto& tcClasses = tcMetrics.GetTcClasses();
    std::cout << "Interface " << iface->GetName() << " - TC Class Count: " << tcClasses.size() << std::endl;
    for (const auto& cls : tcClasses) {
      EXPECT_FALSE(cls.Handle.empty());
      EXPECT_FALSE(cls.Parent.empty());
      EXPECT_FALSE(cls.Kind.empty());
    }
  }
}

} // namespace backend::network::interface::tests
