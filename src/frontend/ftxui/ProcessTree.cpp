#include "ProcessTree.hpp"

#include <algorithm>
#include <cassert>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <functional>
#include <ranges>

#include "../../backend/process/ProcessMetrics.hpp"
#include "ProcessColumns.hpp"

namespace frontend::ftxui {

ProcessTree::Row::Row(ProcessTree& tree, backend::process::Process& process)
    : Tree(tree), Process(process), Metrics(process), OnNodeRemoving(process.OnNodeRemoving([this](bool removed) {
        if (removed) {
          Tree.OnNodeRemoving(*this);
        }
      })) {}

ProcessTree::ProcessTree(std::shared_ptr<backend::metrics::SimplePublisher<int>> tick, std::function<void()> refresh)
    : _Refresh(refresh), _Columns(CreateDefaultColumns()), _CursorRow(_Rows.end()), _CursorColumn(_Columns.end()),
      _TickUpdater(backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<int>>(
          tick, [this](auto tick) { this->Update(); })) {}

std::string ProcessTree::GetTabName() const { return kProcessTreeTabName; }

void ProcessTree::MoveCursorAndDraw(ssize_t offset) {
  auto& selectedProcess = _ProcessManager.MoveCursor((*_CursorRow)->Process.value().get(), offset);
  auto ps = _ProcessManager.GetAround(selectedProcess, std::distance(_Rows.begin(), _CursorRow), _TableCapacity);
  UpdateData(selectedProcess, ps);
}

void ProcessTree::OnTableSizeChange(::ftxui::Box box) {
  ssize_t newCapacity = box.y_max - box.y_min + 1 - HeaderHeight;
  if (newCapacity > 0 && newCapacity != _TableCapacity) {
    _TableCapacity = newCapacity;
    Update();
    _Refresh();
  }
}

void ProcessTree::OnNodeRemoving(Row& row) {
  assert(!_Rows.empty());
  assert(row.Process.has_value());
  auto& process = row.Process.value().get();
  row.Process.reset();
  if (_CursorRow != _Rows.end() && _CursorRow->get() == std::addressof(row)) {
    auto FindValidRow = [&](auto it) -> decltype(_CursorRow) {
      for (; it != _Rows.begin(); --it) {
        if ((*it)->Process.has_value()) {
          return it;
        }
      }
      if ((*_Rows.begin())->Process.has_value()) {
        return _Rows.begin();
      } else {
        return _Rows.end();
      }
    };

    auto parent = process.GetParent();
    if (parent.has_value()) {
      auto& newSelected = parent.value().get();
      auto it = std::ranges::find_if(_Rows, [&](auto& row) {
        return row->Process.has_value() && std::addressof(row->Process.value().get()) == std::addressof(newSelected);
      });
      _CursorRow = (it != _Rows.end()) ? it : FindValidRow(_CursorRow);
    } else {
      _CursorRow = FindValidRow(_CursorRow);
    }
  }
  assert(_CursorRow == _Rows.end() || (*_CursorRow)->Process.has_value());
}

void ProcessTree::Update() {
  auto size = _TableCapacity;
  if (size > 0) {
    _ProcessManager.UpdateList();
    std::vector<std::reference_wrapper<backend::process::Process>> ps;
    if (_CursorRow != _Rows.end()) {
      auto& selectedProcess = (*_CursorRow)->Process.value().get();
      auto cursorPosition = std::min(size - 1, std::distance(_Rows.begin(), _CursorRow));
      ps = _ProcessManager.GetAround(selectedProcess, cursorPosition, size);
      UpdateData(selectedProcess, ps);
    } else {
      ps = _ProcessManager.GetTopK(size);
      if (!ps.empty()) {
        UpdateData(ps[0].get(), ps);
      }
    }
  }
}

void ProcessTree::UpdateData(backend::process::Process& selectedProcess,
                             std::vector<std::reference_wrapper<backend::process::Process>> ps) {
  assert(std::ranges::contains(ps, selectedProcess));
  std::map<backend::process::PidType, std::reference_wrapper<std::unique_ptr<Row>>> existingProcesses;
  for (auto& row : _Rows) {
    if (row->Process.has_value()) {
      existingProcesses.emplace(row->Process.value().get().GetPid(), row);
    }
  }

  std::list<std::unique_ptr<Row>> oldRows;
  oldRows.swap(_Rows);      // Move old rows to a temporary list to preserve their memory while we create new rows
  _CursorRow = _Rows.end(); // Reset cursor row, will be updated in the loop below
  for (auto& process : ps | std::views::transform([](auto& ref) -> backend::process::Process& { return ref.get(); })) {
    auto it = existingProcesses.find(process.GetPid());
    if (it != existingProcesses.end()) {
      _Rows.emplace_back(std::move(it->second.get()));
      _Rows.back()->Metrics.UpdateMetrics(process);
    } else {
      _Rows.push_back(std::make_unique<Row>(*this, process));
      for (auto& column : _Columns) {
        column->RegisterRow(*_Rows.back());
      }
    }
    if (process == selectedProcess) {
      _CursorRow = std::prev(_Rows.end());
    }
  }
  assert(_CursorRow != _Rows.end());
}

void ProcessTree::RemoveData() {
  _Rows.clear();
  _CursorRow = _Rows.end();
}

bool ProcessTree::OnEvent(::ftxui::Event event) {
  if (event == ::ftxui::Event::ArrowUp) {
    if (_CursorRow != _Rows.begin()) {
      _CursorRow = std::prev(_CursorRow);
    } else {
      MoveCursorAndDraw(-1); // Move cursor to the previous process and redraw
    }
    return true;
  } else if (event == ::ftxui::Event::ArrowDown) {
    if (_CursorRow != std::prev(_Rows.end())) {
      _CursorRow = std::next(_CursorRow);
    } else {
      MoveCursorAndDraw(1); // Move cursor to the next process and redraw
    }
    return true;
  } else if (event == ::ftxui::Event::PageUp) {
    MoveCursorAndDraw(-_TableCapacity); // Move cursor up by one page and redraw
    return true;
  } else if (event == ::ftxui::Event::PageDown) {
    MoveCursorAndDraw(_TableCapacity); // Move cursor down by one page and redraw
    return true;
  } else {
    return false;
  }
}

::ftxui::Element ProcessTree::Render() {
  using namespace ::ftxui;

  if (_Rows.empty()) {
    return text("Loading...") | center | reflect([this](Box box) { this->OnTableSizeChange(box); });
  }

  std::vector<std::vector<std::string>> data;
  // Header
  data.push_back(_Columns |
                 std::views::transform([](const auto& column) -> std::string { return column->GetHeaderText(); }) |
                 std::ranges::to<std::vector>());

  // Data
  for (auto& row : _Rows) {
    if (row->Process.has_value()) {
      data.push_back(_Columns | std::views::transform([&](const auto& column) -> std::string {
                       bool isRowSelected = (row == *_CursorRow);
                       bool isColumnSelected = (column == *_CursorColumn);
                       return column->GetDataText(isRowSelected, isColumnSelected, *row);
                     }) |
                     std::ranges::to<std::vector>());
    }
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

std::list<std::unique_ptr<ProcessTree::Column>> ProcessTree::CreateDefaultColumns() {
  std::list<std::unique_ptr<Column>> columns;
  columns.push_back(std::make_unique<ColumnCursor>());
  columns.push_back(std::make_unique<ColumnPid>());
  columns.push_back(std::make_unique<ColumnState>());
  columns.push_back(std::make_unique<ColumnUser>());
  columns.push_back(std::make_unique<ColumnCpu>());
  columns.push_back(std::make_unique<ColumnMem>());
  columns.push_back(std::make_unique<ColumnTime>());
  columns.push_back(std::make_unique<ColumnDiskRead>());
  columns.push_back(std::make_unique<ColumnDiskWrite>());
  columns.push_back(std::make_unique<ColumnDiskAccumulated>());
  columns.push_back(std::make_unique<ColumnIO>());
  columns.push_back(std::make_unique<ColumnIOAccumulated>());
  columns.push_back(std::make_unique<ColumnStart>());
  columns.push_back(std::make_unique<ColumnCommand>());
  return columns;
}

} // namespace frontend::ftxui
