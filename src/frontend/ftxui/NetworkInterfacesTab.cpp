#include "NetworkInterfacesTab.hpp"

#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>

#include "../../utils/Formatter.hpp"

namespace frontend::ftxui {

NetworkInterfacesTab::NetworkInterfacesTab(OmniMonInterface& omniMon)
    : _OmniMon(omniMon), _Manager(omniMon.GetLoop(), *this) {
  UpdateMetrics();

  // Subscribe to periodic update tick
  _TickUpdater = _OmniMon.OnTickUpdate([this](int) { UpdateMetrics(); });
}

std::string NetworkInterfacesTab::GetTabName() const { return kNetworkInterfacesTabName; }

void NetworkInterfacesTab::UpdateMetrics() {
  for (auto& [ifIndex, metrics] : _MetricsMap) {
    metrics->Update();
  }
  _OmniMon.ScheduleRefresh();
}

void NetworkInterfacesTab::OnInterfaceCreated(std::shared_ptr<backend::network::interface::Interface> iface) {
  int ifIndex = iface->GetIfIndex();
  _InterfacesMap[ifIndex] = iface;
  _MetricsMap[ifIndex] = std::make_unique<backend::network::interface::InterfaceMetrics>(*iface);
  _OmniMon.ScheduleRefresh();
}

void NetworkInterfacesTab::OnInterfaceDeleted(std::shared_ptr<backend::network::interface::Interface> iface) {
  int ifIndex = iface->GetIfIndex();
  _InterfacesMap.erase(ifIndex);
  _MetricsMap.erase(ifIndex);
  _OmniMon.ScheduleRefresh();
}

void NetworkInterfacesTab::OnInterfaceChanged(std::shared_ptr<backend::network::interface::Interface> iface) {
  int ifIndex = iface->GetIfIndex();
  _InterfacesMap[ifIndex] = iface;
  if (!_MetricsMap.contains(ifIndex)) {
    _MetricsMap[ifIndex] = std::make_unique<backend::network::interface::InterfaceMetrics>(*iface);
  }
  _OmniMon.ScheduleRefresh();
}

bool NetworkInterfacesTab::OnEvent(::ftxui::Event event) {
  using namespace ::ftxui;

  std::vector<int> sortedIndices;
  for (const auto& [ifIndex, _] : _InterfacesMap) {
    sortedIndices.push_back(ifIndex);
  }

  if (sortedIndices.empty()) {
    return false;
  }

  if (event == Event::ArrowUp) {
    if (_SelectedIndex > 0) {
      _SelectedIndex--;
      _OmniMon.ScheduleRefresh();
    }
    return true;
  } else if (event == Event::ArrowDown) {
    if (_SelectedIndex + 1 < sortedIndices.size()) {
      _SelectedIndex++;
      _OmniMon.ScheduleRefresh();
    }
    return true;
  } else if (event == Event::PageUp) {
    if (_SelectedIndex >= 2) {
      _SelectedIndex -= 2;
    } else {
      _SelectedIndex = 0;
    }
    _OmniMon.ScheduleRefresh();
    return true;
  } else if (event == Event::PageDown) {
    if (_SelectedIndex + 2 < sortedIndices.size()) {
      _SelectedIndex += 2;
    } else {
      _SelectedIndex = sortedIndices.size() - 1;
    }
    _OmniMon.ScheduleRefresh();
    return true;
  }

  return false;
}

::ftxui::Element NetworkInterfacesTab::Render() {
  using namespace ::ftxui;

  // Gather and sort interfaces by index
  std::vector<int> sortedIndices;
  for (const auto& [ifIndex, _] : _InterfacesMap) {
    sortedIndices.push_back(ifIndex);
  }

  // Handle boundary clamps
  if (sortedIndices.empty()) {
    _SelectedIndex = 0;
  } else if (_SelectedIndex >= sortedIndices.size()) {
    _SelectedIndex = sortedIndices.size() - 1;
  }

  // Compute scroll window
  // Dynamic card height is roughly 9 lines.
  int cardHeight = 9;
  int maxCardsInView = std::max(1, _VisibleHeight / cardHeight);

  if (static_cast<int>(_SelectedIndex) < _ScrollOffset) {
    _ScrollOffset = _SelectedIndex;
  } else if (static_cast<int>(_SelectedIndex) >= _ScrollOffset + maxCardsInView) {
    _ScrollOffset = _SelectedIndex - maxCardsInView + 1;
  }

  if (_ScrollOffset < 0) {
    _ScrollOffset = 0;
  }
  if (!sortedIndices.empty() && _ScrollOffset >= static_cast<int>(sortedIndices.size())) {
    _ScrollOffset = sortedIndices.size() - 1;
  }

  Elements cards;
  int startIdx = _ScrollOffset;
  int endIdx = std::min(static_cast<int>(sortedIndices.size()), startIdx + maxCardsInView);

  for (int i = startIdx; i < endIdx; ++i) {
    int ifIndex = sortedIndices[i];
    auto iface = _InterfacesMap[ifIndex];
    auto& metrics = *_MetricsMap[ifIndex];
    bool isSelected = (static_cast<size_t>(i) == _SelectedIndex);

    // Color operstate
    auto status_enum = iface->GetOperState();
    std::string status = backend::network::interface::ToString(status_enum);
    auto status_color = Color::Yellow;
    if (status_enum == backend::network::interface::Interface::OperState::Up) {
      status_color = Color::Green;
    } else if (status_enum == backend::network::interface::Interface::OperState::Down) {
      status_color = Color::Red;
    }

    std::string speed_str = iface->GetSpeed() >= 0 ? std::format("{} Mbps", iface->GetSpeed()) : "unknown";
    std::string duplex_str = backend::network::interface::ToString(iface->GetDuplex());

    Element header = hbox({text(iface->GetName()) | bold | color(Color::Cyan),
                           text(std::format(" (Index: {})", iface->GetIfIndex())) | dim, filler(), text(" [ ") | dim,
                           text(status) | bold | color(status_color), text(" | DUPLEX: ") | dim,
                           text(duplex_str) | bold | color(Color::BlueLight), text(" | SPEED: ") | dim,
                           text(speed_str) | bold | color(Color::BlueLight), text(" ] ") | dim});

    Element ips = hbox({text("  IPv4: ") | bold,
                        text(backend::network::interface::ToString(iface->GetPrimaryIpV4())) | color(Color::White),
                        text("   IPv6: ") | bold,
                        text(backend::network::interface::ToString(iface->GetPrimaryIpV6())) | color(Color::White),
                        filler(), text("MAC: ") | bold,
                        text(backend::network::interface::ToString(iface->GetMacAddress(), iface->GetIfType())) |
                            color(Color::MagentaLight)});

    Element link_details =
        hbox({text("  MTU: ") | bold, text(std::to_string(iface->GetMtu())) | color(Color::YellowLight),
              text("   Qdisc: ") | bold, text(iface->GetQdiscType()) | color(Color::YellowLight), filler()});

    auto rx_bytes = metrics.GetRxBytes() ? metrics.GetRxBytes()->GetValue() : 0;
    auto tx_bytes = metrics.GetTxBytes() ? metrics.GetTxBytes()->GetValue() : 0;
    auto rx_packets = metrics.GetRxPackets() ? metrics.GetRxPackets()->GetValue() : 0;
    auto tx_packets = metrics.GetTxPackets() ? metrics.GetTxPackets()->GetValue() : 0;
    auto rx_errors = metrics.GetRxErrors() ? metrics.GetRxErrors()->GetValue() : 0;
    auto tx_errors = metrics.GetTxErrors() ? metrics.GetTxErrors()->GetValue() : 0;
    auto rx_dropped = metrics.GetRxDropped() ? metrics.GetRxDropped()->GetValue() : 0;
    auto tx_dropped = metrics.GetTxDropped() ? metrics.GetTxDropped()->GetValue() : 0;
    auto rx_multicast = metrics.GetRxMulticast() ? metrics.GetRxMulticast()->GetValue() : 0;
    auto collisions = metrics.GetCollisions() ? metrics.GetCollisions()->GetValue() : 0;

    Element rx_col = vbox({text("  RECEIVE (Rx)") | bold | color(Color::GreenLight) | underlined,
                           hbox({text("    Bytes: ") | dim, text(utils::DiskSizeToString(rx_bytes)) | bold}),
                           hbox({text("    Packets: ") | dim, text(std::to_string(rx_packets))}),
                           hbox({text("    Errors: ") | dim,
                                 text(std::to_string(rx_errors)) | color(rx_errors > 0 ? Color::Red : Color::White)}),
                           hbox({text("    Dropped: ") | dim,
                                 text(std::to_string(rx_dropped)) | color(rx_dropped > 0 ? Color::Red : Color::White)}),
                           hbox({text("    Multicast: ") | dim, text(std::to_string(rx_multicast))})});

    Element tx_col =
        vbox({text("  TRANSMIT (Tx)") | bold | color(Color::GreenLight) | underlined,
              hbox({text("    Bytes: ") | dim, text(utils::DiskSizeToString(tx_bytes)) | bold}),
              hbox({text("    Packets: ") | dim, text(std::to_string(tx_packets))}),
              hbox({text("    Errors: ") | dim,
                    text(std::to_string(tx_errors)) | color(tx_errors > 0 ? Color::Red : Color::White)}),
              hbox({text("    Dropped: ") | dim,
                    text(std::to_string(tx_dropped)) | color(tx_dropped > 0 ? Color::Red : Color::White)}),
              hbox({text("    Collisions: ") | dim,
                    text(std::to_string(collisions)) | color(collisions > 0 ? Color::Red : Color::White)})});

    Element metrics_box = hbox({rx_col | size(WIDTH, EQUAL, 35), separatorLight(), tx_col | size(WIDTH, EQUAL, 35)});

    Element card_content = vbox({header, separatorLight(), ips, link_details, separatorLight(), metrics_box});

    Element card;
    if (isSelected) {
      card = window(text("⮚ ACTIVE INTERFACE ⮘") | bold | color(Color::Yellow), card_content) | color(Color::Yellow);
    } else {
      card = window(text("INTERFACE") | dim, card_content) | color(Color::GrayDark);
    }

    cards.push_back(card);
  }

  if (cards.empty()) {
    cards.push_back(text("No network interfaces detected.") | center | bold | color(Color::Yellow));
  }

  Element title_bar = hbox({text(" NETWORK INTERFACES ") | bold | color(Color::Yellow),
                            text(" - Real-time metrics and interface information") | dim, filler(),
                            text(std::format("Total: {}", sortedIndices.size())) | bold | color(Color::Cyan)});

  Element footer = hbox({text(" [Tab] Select Tab ") | bold | color(Color::Green), separatorLight(),
                         text(" [↑/↓] Select Interface ") | bold | color(Color::Green), separatorLight(),
                         text(" [PageUp/PageDown] Scroll ") | bold | color(Color::Green), separatorLight(),
                         text(" [q] Quit ") | bold | color(Color::Green)});

  return vbox({title_bar, separator(),
               vbox(std::move(cards)) | flex | reflect([this](Box box) { _VisibleHeight = box.y_max - box.y_min + 1; }),
               separator(), footer}) |
         border;
}

} // namespace frontend::ftxui
