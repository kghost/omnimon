#pragma once

#include "../../events/Events.hpp"
#include <linux/netlink.h>
#include <map>
#include <memory>

#include "Interface.hpp"

namespace backend::network::interface {

class InterfaceCallback {
public:
  virtual ~InterfaceCallback() = default;
  virtual void OnInterfaceCreated(std::shared_ptr<Interface> iface) = 0;
  virtual void OnInterfaceDeleted(std::shared_ptr<Interface> iface) = 0;
  virtual void OnInterfaceChanged(std::shared_ptr<Interface> iface) = 0;
};

class InterfaceManager : public events::EventHandle {
public:
  explicit InterfaceManager(events::EventLoop& loop, InterfaceCallback& callback);
  ~InterfaceManager() override = default;

  InterfaceManager(const InterfaceManager&) = delete;
  InterfaceManager& operator=(const InterfaceManager&) = delete;

  void OnRead() override;
  void OnWrite() override;

  const std::map<unsigned int, std::shared_ptr<Interface>>& GetInterfaces() const { return _Interfaces; }

private:
  void InitializeList();
  void ProcessLinkMessage(struct nlmsghdr* nlh);
  void ProcessAddressMessage(struct nlmsghdr* nlh);

  InterfaceCallback& _Callback;
  std::map<unsigned int, std::shared_ptr<Interface>> _Interfaces;
};

} // namespace backend::network::interface
