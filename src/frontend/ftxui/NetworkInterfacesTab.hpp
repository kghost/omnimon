#pragma once

#include <map>
#include <memory>
#include <string>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "../../backend/network/interface/Interface.hpp"
#include "../../backend/network/interface/InterfaceManager.hpp"
#include "../../backend/network/interface/InterfaceMetrics.hpp"
#include "OmniMonInterface.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

class NetworkInterfacesTab : public FtxuiTabView, public backend::network::interface::InterfaceCallback {
public:
  explicit NetworkInterfacesTab(OmniMonInterface& omniMon);
  ~NetworkInterfacesTab() override = default;

  // FtxuiTabView overrides
  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;

  // InterfaceCallback overrides
  void OnInterfaceCreated(std::shared_ptr<backend::network::interface::Interface> iface) override;
  void OnInterfaceDeleted(std::shared_ptr<backend::network::interface::Interface> iface) override;
  void OnInterfaceChanged(std::shared_ptr<backend::network::interface::Interface> iface) override;

private:
  void UpdateMetrics();

  OmniMonInterface& _OmniMon;
  std::map<int, std::shared_ptr<backend::network::interface::Interface>> _InterfacesMap;
  std::map<int, std::unique_ptr<backend::network::interface::InterfaceMetrics>> _MetricsMap;
  backend::network::interface::InterfaceManager _Manager;
  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;

  size_t _SelectedIndex = 0;
  int _ScrollOffset = 0;
  int _VisibleHeight = 20;
};

} // namespace frontend::ftxui
