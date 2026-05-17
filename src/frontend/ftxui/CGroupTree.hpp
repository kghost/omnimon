#pragma once

#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "../../backend/cgroupv2/CGroupManager.hpp"
#include "../../backend/cgroupv2/CGroupMetrics.hpp"
#include "../../backend/cgroupv2/CGroupNode.hpp"
#include "../../backend/events/Events.hpp"
#include "../../backend/metrics/Binding.hpp"
#include "../../backend/metrics/SimplePublisher.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

class CGroupTree : public FtxuiTabView {
public:
  explicit CGroupTree(backend::events::EventLoop& loop, std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                      std::function<void()> refresh);
  ~CGroupTree() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;

  class Row {
  public:
    explicit Row(CGroupTree& tree, backend::cgroupv2::CGroupNode& node);
    ~Row();

    CGroupTree& _Tree;
    backend::cgroupv2::CGroupNode& Node;
    backend::cgroupv2::CGroupMetrics Metrics;
    std::shared_ptr<backend::metrics::SubscriberBase> OnNodeRemoving;

    struct MetricEntry {
      std::string Display;
      std::shared_ptr<backend::metrics::SubscriberBase> Updater;
    };

    MetricEntry Pids;
    MetricEntry Memory;
    MetricEntry CpuUsage;
    MetricEntry CpuUser;
    MetricEntry CpuSystem;

    struct IoMetrics {
      MetricEntry ReadBytes;
      MetricEntry WriteBytes;
      MetricEntry ReadCalls;
      MetricEntry WriteCalls;
      MetricEntry DiscardBytes;
      MetricEntry DiscardCalls;
    };

    std::map<std::string, IoMetrics> IoMetrics;

    void UpdateMetrics();
  };

  class Column {
  public:
    virtual ~Column() = default;
    virtual std::string GetHeaderText() const = 0;
    virtual void RegisterRow(Row& row) const = 0;
    virtual std::string GetDataText(bool isRowSelected, bool isColumnSelected, Row& row) const = 0;
    virtual void Decorate(::ftxui::TableSelection selection) const = 0;

    bool IsShown = true;
  };

  void OnRowRemoving(Row& row);

private:
  std::function<void()> _Refresh;

  static constexpr const ssize_t HeaderHeight = 1;
  ssize_t _TableCapacity = 0;
  backend::cgroupv2::CGroupManager _Manager;
  std::list<std::unique_ptr<Row>> _Rows;
  std::list<std::unique_ptr<Column>> _Columns;
  std::list<std::unique_ptr<Row>>::iterator _CursorRow;
  std::list<std::unique_ptr<Column>>::iterator _CursorColumn;
  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;
  std::set<std::string> _RegisteredDiskColumns;

  void OnTableSizeChange(::ftxui::Box box);
  void Update();
  void MoveCursorAndDraw(ssize_t offset);

  void DiscoverColumns(Row& row);
  // TODO: get range to all column
  // TODO: get range to column of a disk

  void UpdateData(backend::cgroupv2::CGroupNode& selected,
                  std::vector<std::reference_wrapper<backend::cgroupv2::CGroupNode>> nodes);
  void RemoveData();

  std::list<std::unique_ptr<Column>> CreateDefaultColumns();
};

} // namespace frontend::ftxui
