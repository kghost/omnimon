#include <iostream>
#include <memory>
#include <net/if.h>

#include <gtest/gtest.h>

#include "../InterfaceManager.hpp"

namespace backend::network::interface::tests {

namespace {

class TestCallback : public InterfaceCallback {
public:
  void OnInterfaceCreated(std::shared_ptr<Interface> iface) override {}
  void OnInterfaceDeleted(std::shared_ptr<Interface> iface) override {}
  void OnInterfaceChanged(std::shared_ptr<Interface> iface) override {}
};

} // namespace

TEST(BasicTest, BasicDiscovery) {
  events::EventLoop loop;
  TestCallback callback;
  InterfaceManager manager(loop, callback);
  const auto& interfaces = manager.GetInterfaces();

  EXPECT_FALSE(interfaces.empty());
  unsigned int loIndex = if_nametoindex("lo");
  EXPECT_NE(loIndex, 0u);

  auto it = interfaces.find(loIndex);
  if (it != interfaces.end()) {
    auto lo = it->second;
    EXPECT_EQ(lo->GetName(), "lo");
    EXPECT_GT(lo->GetMtu(), 0u);
    EXPECT_FALSE(lo->GetMacAddress().empty());
    std::cout << "lo IPv4: " << lo->GetPrimaryIpV4() << std::endl;
    std::cout << "lo IPv6: " << lo->GetPrimaryIpV6() << std::endl;
  }
}

} // namespace backend::network::interface::tests
