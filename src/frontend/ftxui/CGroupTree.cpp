#include "CGroupTree.hpp"

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <ranges>

#include "../../utils/Clock.hpp"
#include "../../utils/Formatter.hpp"
#include "DebugWindow.hpp"

namespace frontend::ftxui {

CGroupRow::CGroupRow(CGroupTree& tree, backend::cgroupv2::CGroupNode& node)
    : RowBase(tree, node, node.OnNodeRemoving([this](bool removed) {
        if (removed) {
          this->Table.OnRowRemoving(*this);
        }
      })),
      Metrics(node) {}

CGroupRow::~CGroupRow() {}

void CGroupRow::UpdateMetrics(CGroupTree& tree) {
  Metrics.ReadFromDirectory(Node.value().get());
  for (auto& [device, ioStat] :
       Metrics.GetIoStats() | std::views::filter([&](auto& pair) { return !IoMetrics.contains(pair.first); })) {
    for (auto& column : tree.GetDiskColumns(device)) {
      column.RegisterRow(*this);
    }
  }
}

CGroupTree::CGroupTree(OmniMonInterface& interface)
    : TableBase(interface, [this](auto) { Update(); }), _Manager(interface.GetLoop()),
      _Columns(CreateDefaultColumns()) {}

std::string CGroupTree::GetTabName() const { return kCGroupV2TabName; }

std::array<std::unique_ptr<CGroupTree::Column>, 8> CGroupTree::CreateDefaultColumns() {
  struct CursorColumn : public Column {
    std::string GetHeaderText() const override { return "≡"; }
    void RegisterRow(Row& row) const override {}
    std::string GetDataText(bool isRowSelected, Row& row) const override { return isRowSelected ? "⮚" : " "; }
    void Decorate(::ftxui::TableSelection selection) const override {
      selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
    }
  };

  struct PathColumn : public Column {
    std::string GetHeaderText() const override { return "CGroup"; }
    void RegisterRow(Row& row) const override {}
    std::string GetDataText(bool, Row& row) const override {
      return utils::TreeString(row.Node.value().get().GetTreePosition()) + row.Node.value().get().GetName();
    }
    void Decorate(::ftxui::TableSelection) const override {}
  };

  struct StartTimeColumn : public Column {
    std::string GetHeaderText() const override { return "Start"; }
    void RegisterRow(Row& row) const override {}
    std::string GetDataText(bool, Row& row) const override {
      return utils::FormatTime(row.Node.value().get().GetCreateTime());
    }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct PidsColumn : public Column {
    std::string GetHeaderText() const override { return "PIDs"; }
    void RegisterRow(Row& row) const override {
      row.Pids.Updater = backend::metrics::MakeSubscriber(
          row.Metrics.GetPidsCurrent(), [&row](auto metric) { row.Pids.Display = std::to_string(metric); });
    }
    std::string GetDataText(bool, Row& row) const override { return row.Pids.Display; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct MemoryColumn : public Column {
    std::string GetHeaderText() const override { return "Memory"; }
    void RegisterRow(Row& row) const override {
      row.Memory.Updater = backend::metrics::MakeSubscriber(row.Metrics.GetMemoryCurrent(), [&row](auto metric) {
        row.Memory.Display = utils::DiskSizeToString(metric);
      });
    }
    std::string GetDataText(bool, Row& row) const override { return row.Memory.Display; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuUsageColumn : public Column {
    std::string GetHeaderText() const override { return "CPU"; }
    void RegisterRow(Row& row) const override {
      row.CpuUsage.Updater = backend::metrics::MakeSubscriber(
          row.Metrics.GetCpuUsageUsec(), [&row](auto metric) { row.CpuUsage.Display = utils::FormatDuration(metric); });
    }
    std::string GetDataText(bool, Row& row) const override { return row.CpuUsage.Display; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuUserColumn : public Column {
    std::string GetHeaderText() const override { return "User"; }
    void RegisterRow(Row& row) const override {
      row.CpuUser.Updater = backend::metrics::MakeSubscriber(
          row.Metrics.GetCpuUserUsec(), [&row](auto metric) { row.CpuUser.Display = utils::FormatDuration(metric); });
    }
    std::string GetDataText(bool, Row& row) const override { return row.CpuUser.Display; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuSystemColumn : public Column {
    std::string GetHeaderText() const override { return "System"; }
    void RegisterRow(Row& row) const override {
      row.CpuSystem.Updater = backend::metrics::MakeSubscriber(row.Metrics.GetCpuSystemUsec(), [&row](auto metric) {
        row.CpuSystem.Display = utils::FormatDuration(metric);
      });
    }
    std::string GetDataText(bool, Row& row) const override { return row.CpuSystem.Display; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  return std::to_array<std::unique_ptr<Column>>(
      {std::make_unique<CursorColumn>(), std::make_unique<PathColumn>(), std::make_unique<StartTimeColumn>(),
       std::make_unique<PidsColumn>(), std::make_unique<MemoryColumn>(), std::make_unique<CpuUsageColumn>(),
       std::make_unique<CpuUserColumn>(), std::make_unique<CpuSystemColumn>()});
}

template <auto DiskToColumnLabel, auto CGroupRow::IoMetrics::* RowMemberPtr,
          auto backend::cgroupv2::CGroupMetrics::IoStatGauges ::* MetricMemberPtr>
  requires(std::is_invocable_r_v<std::string, decltype(DiskToColumnLabel), std::string>)
struct DiskColumn : public CGroupTree::Column {
public:
  explicit DiskColumn(const std::string& disk) : Disk(disk) {}
  ~DiskColumn() override = default;

  DiskColumn(const DiskColumn&) = delete;
  DiskColumn(DiskColumn&&) = delete;
  DiskColumn& operator=(const DiskColumn&) = delete;
  DiskColumn& operator=(DiskColumn&&) = delete;

  std::string GetHeaderText() const override { return DiskToColumnLabel(Disk); }

  void RegisterRow(CGroupTree::Row& row) const override {
    auto [it, inserted] = row.IoMetrics.try_emplace(Disk);
    auto& ioStats = row.Metrics.GetIoStats();
    auto itStat = ioStats.find(Disk);
    if (itStat != ioStats.end()) {
      (it->second.*RowMemberPtr).Updater =
          backend::metrics::MakeSubscriber(itStat->second.*MetricMemberPtr, [this, &row](auto metric) {
            (row.IoMetrics.at(Disk).*RowMemberPtr).Display = utils::DiskSizeToString(metric);
          });
    }
  }

  std::string GetDataText(bool, CGroupTree::Row& row) const override {
    return (row.IoMetrics.at(Disk).*RowMemberPtr).Display;
  }

  void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }

private:
  const std::string Disk;
};

CGroupTree::DiskColumnSet::DiskColumnSet(const std::string& disk)
    : _Disk(disk),
      _Columns(
          {std::make_unique<
               DiskColumn<[](const std::string& disk) { return "RB[" + disk + "]"; }, &CGroupRow::IoMetrics::ReadBytes,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::ReadBytes>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "WB[" + disk + "]"; }, &CGroupRow::IoMetrics::WriteBytes,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::WriteBytes>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "RC[" + disk + "]"; }, &CGroupRow::IoMetrics::ReadCalls,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::ReadCalls>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "WC[" + disk + "]"; }, &CGroupRow::IoMetrics::WriteCalls,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::WriteCalls>>(_Disk),
           std::make_unique<DiskColumn<[](const std::string& disk) { return "DB[" + disk + "]"; },
                                       &CGroupRow::IoMetrics::DiscardBytes,
                                       &backend::cgroupv2::CGroupMetrics::IoStatGauges::DiscardBytes>>(_Disk),
           std::make_unique<DiskColumn<[](const std::string& disk) { return "DC[" + disk + "]"; },
                                       &CGroupRow::IoMetrics::DiscardCalls,
                                       &backend::cgroupv2::CGroupMetrics::IoStatGauges::DiscardCalls>>(_Disk)}) {}

utils::AnyView<CGroupTree::Column&> CGroupTree::DiskColumnSet::GetColumns() {
  return utils::AnyView<Column&>(std::views::all(_Columns) |
                                 std::views::transform([](auto& col) -> Column& { return *col; }));
}

void CGroupTree::DiscoverColumns(Row& row) {
  for (const auto& [disk, gauges] : row.Metrics.GetIoStats()) {
    _IoColumns.try_emplace(disk, DiskColumnSet(disk));
  }
}

utils::AnyView<CGroupTree::Column&> CGroupTree::GetAllColumns() {
  return utils::AnyView<Column&>(utils::concat<Column&>(
      std::views::all(_Columns) | std::views::transform([](auto& col) -> Column& { return *col; }),
      _IoColumns | std::views::transform([](auto& pair) { return pair.second.GetColumns(); }) | std::views::join));
}

} // namespace frontend::ftxui
