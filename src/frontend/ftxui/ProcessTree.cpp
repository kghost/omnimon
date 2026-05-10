#include "ProcessTree.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

#include "Process.hpp"
#include "ProcessColumns.hpp"

namespace frontend::ftxui {

ProcessTree::ProcessTree(std::shared_ptr<backend::metrics::SimplePublisher<int>> tick)
    : _Columns(CreateDefaultColumns()), _CursorRow(_Rows.end()), _CursorColumn(_Columns.end()),
      _TickUpdater(backend::metrics::MakeSubscriber<backend::metrics::SimplePublisher<int>>(
          tick, [this](auto tick) { this->Update(); })) {
  Update();
}

void ProcessTree::MoveCursorAndDraw(ssize_t offset) {
  auto selectedProcess = _ProcessCollection.MoveCursor((*_CursorRow)->Process, offset);
  auto ps = _ProcessCollection.GetAround(selectedProcess, std::distance(_Rows.begin(), _CursorRow), GetTableCapacity());
  UpdateData(selectedProcess, ps);
}

void ProcessTree::Update() {
  auto size = GetTableCapacity();
  if (size > 0) {
    _ProcessCollection.UpdateList();
    auto cursor = _CursorRow;
    std::vector<std::shared_ptr<Process>> ps;
    std::shared_ptr<Process> selectedProcess;
    if (_CursorRow != _Rows.end()) {
      selectedProcess = _ProcessCollection.GetValidAncestor((*_CursorRow)->Process);
      ps = _ProcessCollection.GetAround(selectedProcess, std::distance(_Rows.begin(), _CursorRow), size);
    } else {
      ps = _ProcessCollection.GetTopK(size);
      if (!ps.empty()) {
        selectedProcess = ps[0];
      }
    }

    UpdateData(selectedProcess, ps);
  }
}

void ProcessTree::UpdateData(std::shared_ptr<Process> selectedProcess, std::vector<std::shared_ptr<Process>> ps) {
  std::map<backend::process::PidType, std::reference_wrapper<std::unique_ptr<Row>>> existingProcesses;
  for (auto& row : _Rows) {
    if (row->Process->Exists()) {
      existingProcesses.emplace(row->Process->GetPid(), row);
    }
  }

  std::list<std::unique_ptr<Row>> oldRows;
  oldRows.swap(_Rows);      // Move old rows to a temporary vector for processing
  _CursorRow = _Rows.end(); // Reset cursor row, will be updated in the loop below
  for (auto process : ps) {
    auto it = existingProcesses.find(process->GetPid());
    if (it != existingProcesses.end()) {
      _Rows.emplace_back(std::move(it->second.get()));
    } else {
      _Rows.push_back(std::make_unique<Row>(Row{.Process = process}));
      for (auto& column : _Columns) {
        column->RegisterRow(*_Rows.back());
      }
    }
    if (process == selectedProcess) {
      _CursorRow = std::prev(_Rows.end());
    }
  }
}

void ProcessTree::OnTerminalSizeChange() { Update(); }

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
    MoveCursorAndDraw(-GetTableCapacity()); // Move cursor up by one page and redraw
    return true;
  } else if (event == ::ftxui::Event::PageDown) {
    MoveCursorAndDraw(GetTableCapacity()); // Move cursor down by one page and redraw
    return true;
  } else {
    return false;
  }
}

::ftxui::Element ProcessTree::Render() {
  using namespace ::ftxui;

  if (_Rows.empty()) {
    return filler() | reflect(_TableSize);
  }

  std::vector<std::vector<std::string>> data;
  // Header
  data.push_back(_Columns |
                 std::views::transform([](const auto& column) -> std::string { return column->GetHeaderText(); }) |
                 std::ranges::to<std::vector>());

  // Data
  for (auto& row : _Rows) {
    data.push_back(_Columns | std::views::transform([&](const auto& column) -> std::string {
                     bool isRowSelected = (row == *_CursorRow);
                     bool isColumnSelected = (column == *_CursorColumn);
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

  return table.Render() | frame | reflect(_TableSize);
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

ssize_t ProcessTree::GetTableCapacity() const {
  if (_TableSize.y_max + 1 <= HeaderHeight) {
    return 0; // No space for data rows
  }
  return _TableSize.y_max + 1 - HeaderHeight;
}

} // namespace frontend::ftxui
