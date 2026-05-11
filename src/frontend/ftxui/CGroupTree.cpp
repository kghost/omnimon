#include "CGroupTree.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <map>
#include <ranges>
#include <vector>

namespace frontend::ftxui {

CGroupTree::Row::Row(std::shared_ptr<backend::cgroupv2::CGroupNode> node) : Node(std::move(node)), Metrics(Node) {}

void CGroupTree::Row::UpdateMetrics() { Metrics.ReadFromDirectory(); }

CGroupTree::CGroupTree(backend::events::EventLoop& loop, std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                       std::function<void()> refresh)
    : _Refresh(std::move(refresh)), _Manager(loop), _Columns(CreateDefaultColumns()), _CursorRow(_Rows.end()),
      _CursorColumn(_Columns.end()),
      _TickUpdater(backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<int>>(
          tick, [this](auto) { this->Update(); })) {}

std::string CGroupTree::GetTabName() const { return kCGroupV2TabName; }

void CGroupTree::MoveCursorAndDraw(ssize_t offset) {
  if (_Rows.empty()) {
    return;
  }

  auto distance = std::distance(_Rows.begin(), _CursorRow);
  auto target = static_cast<ssize_t>(distance) + offset;
  if (target < 0) {
    _CursorRow = _Rows.begin();
  } else if (target >= static_cast<ssize_t>(_Rows.size())) {
    _CursorRow = std::prev(_Rows.end());
  } else {
    _CursorRow = std::next(_Rows.begin(), target);
  }
  _Refresh();
}

void CGroupTree::OnTableSizeChange(::ftxui::Box box) {
  ssize_t newCapacity = box.y_max - box.y_min + 1 - HeaderHeight;
  if (newCapacity > 0 && newCapacity != _TableCapacity) {
    _TableCapacity = newCapacity;
    Update();
    _Refresh();
  }
}

void CGroupTree::Update() {
  auto nodes = GetTreeNodes();
  if (nodes.empty()) {
    _Rows.clear();
    _CursorRow = _Rows.end();
    return;
  }

  UpdateData(std::move(nodes));
}

std::vector<std::pair<std::shared_ptr<backend::cgroupv2::CGroupNode>, ssize_t>> CGroupTree::GetTreeNodes() {
  std::vector<std::pair<std::shared_ptr<backend::cgroupv2::CGroupNode>, ssize_t>> result;

  auto root = _Manager.GetRoot();
  std::function<void(const std::shared_ptr<backend::cgroupv2::CGroupNode>&, ssize_t)> walk;
  walk = [&](const std::shared_ptr<backend::cgroupv2::CGroupNode>& node, ssize_t depth) {
    result.emplace_back(node, depth);
    for (const auto& child : node->GetChildren()) {
      walk(child, depth + 1);
    }
  };

  walk(root, 0);
  return result;
}

void CGroupTree::UpdateRowDisplay(Row& row, ssize_t depth) {
  const auto& node = row.Node;
  row.Name = node->GetName();
  row.PathDisplay = std::string(static_cast<size_t>(depth) * 2, ' ') + row.Name;
  row.UpdateMetrics();

  const auto formatNumber = [&](const std::optional<backend::metrics::DataType>& value) {
    if (!value.has_value()) {
      return std::string("-");
    }
    return std::format("{}", *value);
  };

  row.MemoryDisplay = row.Metrics.GetMemoryCurrent().has_value()
                          ? utils::DiskSizeToString(*row.Metrics.GetMemoryCurrent())
                          : std::string("-");
  row.PidsDisplay = formatNumber(row.Metrics.GetPidsCurrent());

  if (row.Metrics.GetCpuUsageUsec().has_value()) {
    row.CpuUsageDisplay = std::format("{:.2f}s", *row.Metrics.GetCpuUsageUsec() / 1'000'000.0);
  } else {
    row.CpuUsageDisplay = "-";
  }
  if (row.Metrics.GetCpuUserUsec().has_value()) {
    row.CpuUserDisplay = std::format("{:.2f}s", *row.Metrics.GetCpuUserUsec() / 1'000'000.0);
  } else {
    row.CpuUserDisplay = "-";
  }
  if (row.Metrics.GetCpuSystemUsec().has_value()) {
    row.CpuSystemDisplay = std::format("{:.2f}s", *row.Metrics.GetCpuSystemUsec() / 1'000'000.0);
  } else {
    row.CpuSystemDisplay = "-";
  }
}

void CGroupTree::UpdateData(std::vector<std::pair<std::shared_ptr<backend::cgroupv2::CGroupNode>, ssize_t>> nodes) {
  std::map<std::string, std::list<std::unique_ptr<Row>>::iterator> existingRows;
  for (auto it = _Rows.begin(); it != _Rows.end(); ++it) {
    existingRows.emplace((*it)->Node->GetPath().string(), it);
  }

  std::list<std::unique_ptr<Row>> oldRows;
  oldRows.swap(_Rows);
  _CursorRow = _Rows.end();

  for (auto& [node, depth] : nodes) {
    const auto path = node->GetPath().string();
    auto it = existingRows.find(path);
    if (it != existingRows.end()) {
      _Rows.splice(_Rows.end(), oldRows, it->second);
      UpdateRowDisplay(*_Rows.back(), depth);
      existingRows.erase(it);
    } else {
      _Rows.push_back(std::make_unique<Row>(node));
      UpdateRowDisplay(*_Rows.back(), depth);
    }
  }

  if (_CursorRow == _Rows.end() && !_Rows.empty()) {
    _CursorRow = _Rows.begin();
  }
}

bool CGroupTree::OnEvent(::ftxui::Event event) {
  if (_Rows.empty()) {
    return false;
  }

  if (event == ::ftxui::Event::ArrowUp) {
    if (_CursorRow != _Rows.begin()) {
      _CursorRow = std::prev(_CursorRow);
    }
    return true;
  } else if (event == ::ftxui::Event::ArrowDown) {
    if (_CursorRow != std::prev(_Rows.end())) {
      _CursorRow = std::next(_CursorRow);
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

  std::vector<std::vector<std::string>> data;
  data.push_back(_Columns |
                 std::views::transform([](const auto& column) -> std::string { return column->GetHeaderText(); }) |
                 std::ranges::to<std::vector>());

  for (auto& row : _Rows) {
    data.push_back(_Columns | std::views::transform([&](const auto& column) -> std::string {
                     bool isRowSelected = (row == *_CursorRow);
                     bool isColumnSelected = false;
                     return column->GetDataText(isRowSelected, isColumnSelected, *row);
                   }) |
                   std::ranges::to<std::vector>());
  }

  auto table = ::ftxui::Table(data);
  table.SelectAll().DecorateSeparatorVertical(
      ::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).Decorate(underlined);

  for (auto [index, column] : std::views::enumerate(_Columns)) {
    column->Decorate(table.SelectColumn(index));
  }

  return table.Render() | frame | reflect([this](Box box) { this->OnTableSizeChange(box); });
}

std::list<std::unique_ptr<CGroupTree::Column>> CGroupTree::CreateDefaultColumns() {
  struct CursorColumn : public Column {
    std::string GetHeaderText() const override { return "≡"; }
    std::string GetDataText(bool isRowSelected, bool, Row& row) const override { return isRowSelected ? "⮚" : " "; }
    void Decorate(::ftxui::TableSelection selection) const override {
      selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
    }
  };

  struct PathColumn : public Column {
    std::string GetHeaderText() const override { return "CGroup"; }
    std::string GetDataText(bool, bool, Row& row) const override { return row.PathDisplay; }
    void Decorate(::ftxui::TableSelection) const override {}
  };

  struct MemoryColumn : public Column {
    std::string GetHeaderText() const override { return "Memory"; }
    std::string GetDataText(bool, bool, Row& row) const override { return row.MemoryDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct PidsColumn : public Column {
    std::string GetHeaderText() const override { return "PIDs"; }
    std::string GetDataText(bool, bool, Row& row) const override { return row.PidsDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuUsageColumn : public Column {
    std::string GetHeaderText() const override { return "CPU"; }
    std::string GetDataText(bool, bool, Row& row) const override { return row.CpuUsageDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuUserColumn : public Column {
    std::string GetHeaderText() const override { return "User"; }
    std::string GetDataText(bool, bool, Row& row) const override { return row.CpuUserDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuSystemColumn : public Column {
    std::string GetHeaderText() const override { return "System"; }
    std::string GetDataText(bool, bool, Row& row) const override { return row.CpuSystemDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  std::list<std::unique_ptr<Column>> columns;
  columns.push_back(std::make_unique<CursorColumn>());
  columns.push_back(std::make_unique<PathColumn>());
  columns.push_back(std::make_unique<MemoryColumn>());
  columns.push_back(std::make_unique<PidsColumn>());
  columns.push_back(std::make_unique<CpuUsageColumn>());
  columns.push_back(std::make_unique<CpuUserColumn>());
  columns.push_back(std::make_unique<CpuSystemColumn>());
  return columns;
}

} // namespace frontend::ftxui
