#pragma once

#include <memory>
#include <string>

#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>

#include "../../backend/cgroupv2/CGroupManager.hpp"
#include "../../backend/cgroupv2/CGroupMetrics.hpp"
#include "../../backend/cgroupv2/CGroupNode.hpp"
#include "../../utils/Range.hpp"
#include "OmniMonInterface.hpp"
#include "TabSelector.hpp"
#include "TableBase.hpp"

namespace frontend::ftxui {

class CGroupTree;
class CGroupRow;

class CGroupRow : public TableBase<CGroupTree, CGroupRow, backend::cgroupv2::CGroupNode>::RowBase {
public:
  using MetricEntry = typename TableBase<CGroupTree, CGroupRow, backend::cgroupv2::CGroupNode>::RowBase::MetricEntry;

  explicit CGroupRow(CGroupTree& tree, backend::cgroupv2::CGroupNode& node);
  ~CGroupRow();

  CGroupRow(const CGroupRow&) = delete;
  CGroupRow(CGroupRow&&) = delete;
  CGroupRow& operator=(const CGroupRow&) = delete;
  CGroupRow& operator=(CGroupRow&&) = delete;

  void UpdateMetrics(CGroupTree& tree);

  backend::cgroupv2::CGroupMetrics Metrics;

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
};

class CGroupTree : public TableBase<CGroupTree, CGroupRow, backend::cgroupv2::CGroupNode> {
public:
  explicit CGroupTree(OmniMonInterface& omniMon);
  ~CGroupTree() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override { return OnEventImpl(event); }
  ::ftxui::Element Render() override { return RenderImpl(); }

  using Row = CGroupRow;
  using Column = TableBase<CGroupTree, CGroupRow, backend::cgroupv2::CGroupNode>::ColumnBase;

private:
  class DiskColumnSet {
  public:
    explicit DiskColumnSet(const std::string& disk);
    utils::AnyView<Column&> GetColumns();

  private:
    const std::string _Disk;
    std::array<std::unique_ptr<Column>, 6> _Columns;
  };

  friend class TableBase<CGroupTree, CGroupRow, backend::cgroupv2::CGroupNode>;
  friend class CGroupRow;

  backend::cgroupv2::CGroupManager _Manager;

  std::array<std::unique_ptr<Column>, 8> _Columns;
  std::map<std::string, DiskColumnSet> _IoColumns;

  void PreUpdate() {}

  std::array<std::unique_ptr<Column>, 8> CreateDefaultColumns();
  void DiscoverColumns(Row& row);
  utils::AnyView<Column&> GetAllColumns();
  utils::AnyView<Column&> GetDiskColumns(const std::string& disk) { return _IoColumns.at(disk).GetColumns(); }
};

} // namespace frontend::ftxui
