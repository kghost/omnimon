#include "CGroupTree.hpp"

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

#include "../../utils/Clock.hpp"
#include "../../utils/Formatter.hpp"

namespace frontend::ftxui {

CGroupTree::Row::Row(CGroupTree& tree, backend::cgroupv2::CGroupNode& node) : _Tree(tree), Node(node), Metrics(node) {
  OnNodeRemoving = node.OnNodeRemoving([this](bool removed) {
    if (removed) {
      _Tree.OnRowRemoving(*this);
    }
  });
}

CGroupTree::Row::~Row() {}

void CGroupTree::Row::UpdateMetrics() { Metrics.ReadFromDirectory(); }

CGroupTree::CGroupTree(backend::events::EventLoop& loop, std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                       std::function<void()> refresh)
    : _Refresh(std::move(refresh)), _Manager(loop), _Columns(CreateDefaultColumns()), _CursorRow(_Rows.end()),
      _CursorColumn(_Columns.end()),
      _TickUpdater(backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<int>>(
          tick, [this](auto) { this->Update(); })) {}

std::string CGroupTree::GetTabName() const { return kCGroupV2TabName; }

void CGroupTree::MoveCursorAndDraw(ssize_t offset) {
  if (_CursorRow != _Rows.end()) {
    auto& selected = (*_CursorRow)->Node;
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
    _Refresh();
  }
}

void CGroupTree::OnRowRemoving(Row& row) {
  if (!_Rows.empty()) {
    if (_CursorRow != _Rows.end() && _CursorRow->get() == std::addressof(row)) {
      auto parent = row.Node.GetParent();
      if (parent.has_value()) {
        auto it = std::ranges::find(_Rows, std::addressof(parent.value().get()),
                                    [](auto& row) { return std::addressof(row->Node); });
        _CursorRow = (it != _Rows.end()) ? it : _Rows.begin();
      } else {
        _CursorRow = _Rows.begin();
      }
    }
  } else {
    _CursorRow = _Rows.end();
  }
}

void CGroupTree::Update() {
  auto size = _TableCapacity;
  if (size > 0) {
    if (_CursorRow != _Rows.end()) {
      auto& selected = (*_CursorRow)->Node;
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
  std::map<backend::cgroupv2::CGroupNode*, std::list<std::unique_ptr<Row>>::iterator> existingRows;
  for (auto it = _Rows.begin(); it != _Rows.end(); ++it) {
    existingRows.emplace(std::addressof((*it)->Node), it);
  }

  std::list<std::unique_ptr<Row>> oldRows;
  oldRows.swap(_Rows);
  _CursorRow = _Rows.end();

  for (auto& node : nodes) {
    auto it = existingRows.find(std::addressof(node.get()));
    if (it != existingRows.end()) {
      _Rows.splice(_Rows.end(), oldRows, it->second);
      existingRows.erase(it);
      _Rows.back()->UpdateMetrics();
    } else {
      _Rows.push_back(std::make_unique<Row>(*this, node));
      for (auto& column : _Columns) {
        column->RegisterRow(*_Rows.back());
      }
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
    void RegisterRow(Row& row) const override {}
    std::string GetDataText(bool isRowSelected, bool, Row& row) const override { return isRowSelected ? "⮚" : " "; }
    void Decorate(::ftxui::TableSelection selection) const override {
      selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
    }
  };

  struct PathColumn : public Column {
    std::string GetHeaderText() const override { return "CGroup"; }
    void RegisterRow(Row& row) const override {}
    std::string GetDataText(bool, bool, Row& row) const override {
      return utils::TreeString(row.Node.GetTreePosition()) + row.Node.GetName();
    }
    void Decorate(::ftxui::TableSelection) const override {}
  };

  struct MemoryColumn : public Column {
    std::string GetHeaderText() const override { return "Memory"; }
    void RegisterRow(Row& row) const override {
      row.MemoryUpdater = backend::metrics::MakeSubscriber(
          row.Metrics.GetMemoryCurrent(), [&row](auto metric) { row.MemoryDisplay = utils::DiskSizeToString(metric); });
    }
    std::string GetDataText(bool, bool, Row& row) const override { return row.MemoryDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct PidsColumn : public Column {
    std::string GetHeaderText() const override { return "PIDs"; }
    void RegisterRow(Row& row) const override {
      row.PidsUpdater = backend::metrics::MakeSubscriber(
          row.Metrics.GetPidsCurrent(), [&row](auto metric) { row.PidsDisplay = std::to_string(metric); });
    }
    std::string GetDataText(bool, bool, Row& row) const override { return row.PidsDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuUsageColumn : public Column {
    std::string GetHeaderText() const override { return "CPU"; }
    void RegisterRow(Row& row) const override {
      row.CpuUsageUpdater =
          backend::metrics::MakeSubscriber(row.Metrics.GetCpuUsageUsec(), [&row](std::chrono::microseconds metric) {
            row.CpuUsageDisplay = utils::FormatDuration(metric);
          });
    }
    std::string GetDataText(bool, bool, Row& row) const override { return row.CpuUsageDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuUserColumn : public Column {
    std::string GetHeaderText() const override { return "User"; }
    void RegisterRow(Row& row) const override {
      row.CpuUserUpdater =
          backend::metrics::MakeSubscriber(row.Metrics.GetCpuUserUsec(), [&row](std::chrono::microseconds metric) {
            row.CpuUserDisplay = utils::FormatDuration(metric);
          });
    }
    std::string GetDataText(bool, bool, Row& row) const override { return row.CpuUserDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  struct CpuSystemColumn : public Column {
    std::string GetHeaderText() const override { return "System"; }
    void RegisterRow(Row& row) const override {
      row.CpuSystemUpdater =
          backend::metrics::MakeSubscriber(row.Metrics.GetCpuSystemUsec(), [&row](std::chrono::microseconds metric) {
            row.CpuSystemDisplay = utils::FormatDuration(metric);
          });
    }
    std::string GetDataText(bool, bool, Row& row) const override { return row.CpuSystemDisplay; }
    void Decorate(::ftxui::TableSelection selection) const override { selection.DecorateCells(::ftxui::align_right); }
  };

  std::list<std::unique_ptr<Column>> columns;
  columns.push_back(std::make_unique<CursorColumn>());
  columns.push_back(std::make_unique<PathColumn>());
  columns.push_back(std::make_unique<PidsColumn>());
  columns.push_back(std::make_unique<MemoryColumn>());
  columns.push_back(std::make_unique<CpuUsageColumn>());
  columns.push_back(std::make_unique<CpuUserColumn>());
  columns.push_back(std::make_unique<CpuSystemColumn>());
  return columns;
}

} // namespace frontend::ftxui
