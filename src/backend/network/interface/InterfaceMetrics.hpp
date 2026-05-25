#pragma once

#include <chrono>
#include <memory>

#include "../../metrics/SharedGauge.hpp"

struct ifinfomsg;
struct rtattr;

namespace backend::network::interface {

class Interface;

class InterfaceMetrics : public metrics::LastUpdateOwner {
  friend class InterfaceManager;

public:
  explicit InterfaceMetrics(Interface& interface);
  ~InterfaceMetrics() override = default;

  InterfaceMetrics(const InterfaceMetrics&) = delete;
  InterfaceMetrics& operator=(const InterfaceMetrics&) = delete;

  std::chrono::steady_clock::time_point GetLastUpdate() const override { return _LastUpdate; }

  // Gauges
  std::shared_ptr<metrics::SharedGauge> GetRxBytes() const { return _RxBytes; }
  std::shared_ptr<metrics::SharedGauge> GetTxBytes() const { return _TxBytes; }
  std::shared_ptr<metrics::SharedGauge> GetRxPackets() const { return _RxPackets; }
  std::shared_ptr<metrics::SharedGauge> GetTxPackets() const { return _TxPackets; }
  std::shared_ptr<metrics::SharedGauge> GetRxErrors() const { return _RxErrors; }
  std::shared_ptr<metrics::SharedGauge> GetTxErrors() const { return _TxErrors; }
  std::shared_ptr<metrics::SharedGauge> GetRxDropped() const { return _RxDropped; }
  std::shared_ptr<metrics::SharedGauge> GetTxDropped() const { return _TxDropped; }
  std::shared_ptr<metrics::SharedGauge> GetRxMulticast() const { return _RxMulticast; }
  std::shared_ptr<metrics::SharedGauge> GetCollisions() const { return _Collisions; }

  Interface& GetInterface() { return _Interface; }
  const Interface& GetInterface() const { return _Interface; }

  void Update();
  void UpdateFromNetlink(struct ifinfomsg* ifi, char* rta, int rtaLen);

private:
  Interface& _Interface;

  std::chrono::steady_clock::time_point _LastUpdate;

  // Gauges
  std::shared_ptr<metrics::SharedGauge> _RxBytes;
  std::shared_ptr<metrics::SharedGauge> _TxBytes;
  std::shared_ptr<metrics::SharedGauge> _RxPackets;
  std::shared_ptr<metrics::SharedGauge> _TxPackets;
  std::shared_ptr<metrics::SharedGauge> _RxErrors;
  std::shared_ptr<metrics::SharedGauge> _TxErrors;
  std::shared_ptr<metrics::SharedGauge> _RxDropped;
  std::shared_ptr<metrics::SharedGauge> _TxDropped;
  std::shared_ptr<metrics::SharedGauge> _RxMulticast;
  std::shared_ptr<metrics::SharedGauge> _Collisions;
};

} // namespace backend::network::interface
