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
#include "../../backend/metrics/Binding.hpp"
#include "../../utils/Range.hpp"
#include "OmniMonInterface.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

class CGroupTree : public FtxuiTabView {
public:
  explicit CGroupTree(OmniMonInterface& interface);
  ~CGroupTree() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;

  class Row final {
  public:
    explicit Row(CGroupTree& tree, backend::cgroupv2::CGroupNode& node);
    ~Row();

    Row(const Row&) = delete;
    Row(Row&&) = delete;
    Row& operator=(const Row&) = delete;
    Row& operator=(Row&&) = delete;

    CGroupTree& Tree;
    std::optional<std::reference_wrapper<backend::cgroupv2::CGroupNode>> Node;
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

    void UpdateMetrics(CGroupTree& tree);
  };

  class Column {
  public:
    explicit Column() = default;
    virtual ~Column() = default;

    Column(const Column&) = delete;
    Column(Column&&) = delete;
    Column& operator=(const Column&) = delete;
    Column& operator=(Column&&) = delete;

    virtual std::string GetHeaderText() const = 0;
    virtual void RegisterRow(Row& row) const = 0;
    virtual std::string GetDataText(bool isRowSelected, Row& row) const = 0;
    virtual void Decorate(::ftxui::TableSelection selection) const = 0;

    bool IsShown = true;
  };

  void OnRowRemoving(Row& row);

private:
  class DiskColumnSet {
  public:
    explicit DiskColumnSet(const std::string& disk);
    utils::AnyView<Column&> GetColumns();

  private:
    const std::string _Disk;
    std::array<std::unique_ptr<Column>, 6> _Columns;
  };

  OmniMonInterface& _Interface;

  static constexpr const ssize_t HeaderHeight = 1;
  ssize_t _TableCapacity = 0;
  backend::cgroupv2::CGroupManager _Manager;

  std::list<Row> _Rows;
  std::list<Row>::iterator _CursorRow;

  std::array<std::unique_ptr<Column>, 8> _Columns;
  std::map<std::string, DiskColumnSet> _IoColumns;

  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;

  void OnTableSizeChange(::ftxui::Box box);
  void Update();
  void MoveCursorAndDraw(ssize_t offset);

  void UpdateData(backend::cgroupv2::CGroupNode& selected,
                  std::vector<std::reference_wrapper<backend::cgroupv2::CGroupNode>> nodes);
  void RemoveData();

  std::array<std::unique_ptr<Column>, 8> CreateDefaultColumns();
  void DiscoverColumns(Row& row);
  utils::AnyView<Column&> GetAllColumns();
  utils::AnyView<Column&> GetDiskColumns(const std::string& disk) { return _IoColumns.at(disk).GetColumns(); }
};

} // namespace frontend::ftxui
