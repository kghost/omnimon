#include "CGroupTree.hpp"

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

#include "../../utils/Clock.hpp"
#include "../../utils/Formatter.hpp"
#include "DebugWindow.hpp"

namespace frontend::ftxui {

CGroupTree::Row::Row(CGroupTree& tree, backend::cgroupv2::CGroupNode& node) : Tree(tree), Node(node), Metrics(node) {
  OnNodeRemoving = node.OnNodeRemoving([this](bool removed) {
    if (removed) {
      Tree.OnRowRemoving(*this);
    }
  });
}

CGroupTree::Row::~Row() {}

void CGroupTree::Row::UpdateMetrics(CGroupTree& tree) {
  Metrics.ReadFromDirectory(Node.value().get());
  for (auto& [device, ioStat] :
       Metrics.GetIoStats() | std::views::filter([&](auto& pair) { return !IoMetrics.contains(pair.first); })) {
    for (auto& column : tree.GetDiskColumns(device)) {
      column->RegisterRow(*this);
    }
  }
}

CGroupTree::CGroupTree(OmniMonInterface& interface)
    : _Interface(interface), _Manager(interface.GetLoop()), _Columns(CreateDefaultColumns()), _CursorRow(_Rows.end()),
      _TickUpdater(interface.OnTickUpdate([this](auto) { this->Update(); })) {}

std::string CGroupTree::GetTabName() const { return kCGroupV2TabName; }

void CGroupTree::MoveCursorAndDraw(ssize_t offset) {
  if (_CursorRow != _Rows.end()) {
    auto& selected = _CursorRow->Node.value().get();
    auto& nextSelected = _Manager.MoveCursor(selected, offset);
    auto nodes = _Manager.GetAround(nextSelected, std::distance(_Rows.begin(), _CursorRow), _TableCapacity);
    UpdateData(nextSelected, nodes);
  }
}

void CGroupTree::OnTableSizeChange(::ftxui::Box box) {
  ssize_t newCapacity = box.y_max - box.y_min + 1 - HeaderHeight;
  if (newCapacity > 0 && newCapacity != _TableCapacity) {
    _TableCapacity = newCapacity;
    Update();
    _Interface.ScheduleRefresh();
  }
}

void CGroupTree::OnRowRemoving(Row& row) {
  assert(!_Rows.empty());
  assert(row.Node.has_value());
  auto& node = row.Node.value().get();
  row.Node.reset();
  if (_CursorRow != _Rows.end() && std::to_address(_CursorRow) == std::addressof(row)) {
    auto FindValidRow = [&](auto it) -> decltype(_CursorRow) {
      for (; it != _Rows.begin(); --it) {
        if (it->Node.has_value()) {
          return it;
        }
      }
      if (_Rows.begin()->Node.has_value()) {
        return _Rows.begin();
      } else {
        return _Rows.end();
      }
    };

    auto parent = node.GetParent();
    if (parent.has_value()) {
      auto& newSelected = parent.value().get();
      auto it = std::ranges::find_if(_Rows, [&](auto& row) {
        return row.Node.has_value() && std::addressof(row.Node.value().get()) == std::addressof(newSelected);
      });
      _CursorRow = (it != _Rows.end()) ? it : FindValidRow(_CursorRow);
    } else {
      _CursorRow = FindValidRow(_CursorRow);
    }
  }
  assert(_CursorRow == _Rows.end() || _CursorRow->Node.has_value());
}

void CGroupTree::Update() {
  auto size = _TableCapacity;
  if (size > 0) {
    if (_CursorRow != _Rows.end()) {
      auto& selected = _CursorRow->Node.value().get();
      auto cursorPosition = std::min(size - 1, std::distance(_Rows.begin(), _CursorRow));
      auto nodes = _Manager.GetAround(selected, cursorPosition, size);
      UpdateData(selected, nodes);
    } else {
      auto nodes = _Manager.GetTopK(size);
      if (!nodes.empty()) {
        auto selected = nodes[0];
        UpdateData(selected, nodes);
      } else {
        RemoveData();
      }
    }
  }
}

void CGroupTree::UpdateData(backend::cgroupv2::CGroupNode& selected,
                            std::vector<std::reference_wrapper<backend::cgroupv2::CGroupNode>> nodes) {
  assert(std::ranges::contains(nodes, selected));

  std::list<Row> oldRows;
  oldRows.swap(_Rows);
  _CursorRow = _Rows.end();

  std::map<backend::cgroupv2::CGroupNode*, std::list<Row>::iterator> existingRows;
  for (auto it = oldRows.begin(); it != oldRows.end(); ++it) {
    if (it->Node.has_value()) {
      existingRows.emplace(std::addressof(it->Node.value().get()), it);
    }
  }

  for (auto& node : nodes) {
    auto it = existingRows.find(std::addressof(node.get()));
    if (it != existingRows.end()) {
      _Rows.splice(_Rows.end(), oldRows, it->second);
      existingRows.erase(it);
      _Rows.back().UpdateMetrics(*this);
    } else {
      _Rows.emplace_back(*this, node);
      Row& row = _Rows.back();
      DiscoverColumns(row);
      for (auto& column : GetAllColumns()) {
        column->RegisterRow(row);
      }
      _Interface.Debug("Showing new CGroup: " + node.get().GetName(), DebugWindow::DebugLevel::Info);
    }
    if (selected == node) {
      _CursorRow = std::prev(_Rows.end());
    }
  }

  if (_CursorRow == _Rows.end() && !_Rows.empty()) {
    _CursorRow = _Rows.begin();
  }
}

void CGroupTree::RemoveData() {
  _Rows.clear();
  _CursorRow = _Rows.end();
}

bool CGroupTree::OnEvent(::ftxui::Event event) {
  if (_Rows.empty()) {
    return false;
  }

  if (event == ::ftxui::Event::ArrowUp) {
    if (_CursorRow != _Rows.begin()) {
      _CursorRow = std::prev(_CursorRow);
    } else {
      MoveCursorAndDraw(-1);
    }
    return true;
  } else if (event == ::ftxui::Event::ArrowDown) {
    if (_CursorRow != std::prev(_Rows.end())) {
      _CursorRow = std::next(_CursorRow);
    } else {
      MoveCursorAndDraw(1);
    }
    return true;
  } else if (event == ::ftxui::Event::PageUp) {
    MoveCursorAndDraw(-_TableCapacity);
    return true;
  } else if (event == ::ftxui::Event::PageDown) {
    MoveCursorAndDraw(_TableCapacity);
    return true;
  }
  return false;
}

::ftxui::Element CGroupTree::Render() {
  using namespace ::ftxui;

  if (_Rows.empty()) {
    return text("Loading...") | center | reflect([this](Box box) { this->OnTableSizeChange(box); });
  }

  auto filterColumn = std::views::filter([](std::unique_ptr<Column>& column) { return column->IsShown; });

  std::vector<std::vector<std::string>> data;
  data.push_back(GetAllColumns() | filterColumn |
                 std::views::transform([](auto& column) -> std::string { return column->GetHeaderText(); }) |
                 std::ranges::to<std::vector<std::string>>());

  for (auto& row : _Rows) {
    if (row.Node.has_value()) {
      data.push_back(GetAllColumns() | filterColumn | std::views::transform([&](auto& column) -> std::string {
                       bool isRowSelected = (std::addressof(row) == std::to_address(_CursorRow));
                       return column->GetDataText(isRowSelected, row);
                     }) |
                     std::ranges::to<std::vector>());
    }
  }

  auto table = ::ftxui::Table(data);
  table.SelectAll().DecorateSeparatorVertical(
      ::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).Decorate(underlined);

  for (auto [index, column] : std::views::enumerate(GetAllColumns() | filterColumn)) {
    column->Decorate(table.SelectColumn(index));
  }

  return table.Render() | frame | reflect([this](Box box) { this->OnTableSizeChange(box); });
}

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

template <auto DiskToColumnLabel, auto CGroupTree::Row::IoMetrics::* RowMemberPtr,
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
               DiskColumn<[](const std::string& disk) { return "RB[" + disk + "]"; }, &Row::IoMetrics::ReadBytes,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::ReadBytes>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "WB[" + disk + "]"; }, &Row::IoMetrics::WriteBytes,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::WriteBytes>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "RC[" + disk + "]"; }, &Row::IoMetrics::ReadCalls,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::ReadCalls>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "WC[" + disk + "]"; }, &Row::IoMetrics::WriteCalls,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::WriteCalls>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "DB[" + disk + "]"; }, &Row::IoMetrics::DiscardBytes,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::DiscardBytes>>(_Disk),
           std::make_unique<
               DiskColumn<[](const std::string& disk) { return "DC[" + disk + "]"; }, &Row::IoMetrics::DiscardCalls,
                          &backend::cgroupv2::CGroupMetrics::IoStatGauges::DiscardCalls>>(_Disk)}) {}

void CGroupTree::DiscoverColumns(Row& row) {
  for (const auto& [disk, gauges] : row.Metrics.GetIoStats()) {
    _IoColumns.try_emplace(disk, DiskColumnSet(disk));
  }
}

} // namespace frontend::ftxui
