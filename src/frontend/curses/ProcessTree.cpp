#include "ProcessTree.hpp"

#include <memory>
#include <vector>

#include "OmniMon.hpp"
#include "Process.hpp"
#include "layouts/Container.hpp"

#include "ProcessColumns.hpp"

namespace frontend::curses {

ProcessTree::ProcessTree() : _Table(std::make_shared<Table>(*this, *this)) {
  constexpr const auto Forward = Container::ChildArrangement::ArrangementType::Forward;
  constexpr const auto FillRest = Container::ChildArrangement::ArrangementType::FillRest;
  _Table->AppendColumn(std::make_shared<ProcessColumnCursor>(), Forward, 1, 0, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnPid>(), Forward, 3, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnUser>(), Forward, 4, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnState>(), Forward, 1, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnCpu>(), Forward, 4, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnMem>(), Forward, 4, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnTime>(), Forward, 2, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnDiskRead>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnDiskWrite>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnDiskAccumulated>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnIO>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnIOAccumulated>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnStart>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnCommand>(), FillRest, 1, 1, 0);

  _Table->AppendRow(std::make_shared<ProcessTreeTableHeaderBinding>(), _TableHeaderHeight);
}

std::shared_ptr<TableCellBinding> ProcessTree::NewCell(Table& table, std::shared_ptr<Row> row,
                                                       std::shared_ptr<Column> column) {
  auto binding = std::dynamic_pointer_cast<ProcessTreeTableRowBinding>(row->GetBinding());
  return binding->CreateCell(*this, column, row->GetBinding());
}

void ProcessTree::Update() {
  auto cursor = _Table->GetCursorRow();
  if (cursor) {
    _ProcessCollection.UpdateList();
    auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(cursor->GetBinding());
    auto selectedProcess = _ProcessCollection.GetValidAncestor(binding->GetProcess());
    auto ps = _ProcessCollection.GetAround(selectedProcess, binding->GetIndex(), GetHeight());
    UpdateTable(selectedProcess, ps);
  } else {
    _ProcessCollection.UpdateList();
    auto ps = _ProcessCollection.GetTopK(GetHeight());
    if (!ps.empty()) {
      UpdateTable(ps[0], ps);
    } else {
      UpdateTable(nullptr, ps);
    }
  }
}

DisplayLength ProcessTree::GetHeight() const {
  DisplayLength height = _Table->GetLayout().Height;
  if (height < _TableHeaderHeight) {
    return 0;
  } else {
    return height - _TableHeaderHeight;
  }
}

void ProcessTree::UpdateTable(std::shared_ptr<frontend::curses::Process> selectedProcess,
                              const std::vector<std::shared_ptr<frontend::curses::Process>>& ps) {
  auto rows = GetDataRows();
  int i = 0;
  for (auto row : rows) {
    if (i < ps.size()) {
      auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(row->GetBinding());
      auto process = ps[i];
      if (selectedProcess == process) {
        _Table->SetCursorRow(row);
      }
      binding->UpdateProcess(row, process);
    } else {
      row->MarkForDeletion();
      auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(row->GetBinding());
      binding->UpdateProcess(row, nullptr);
    }
    i++;
  }

  while (i < ps.size()) {
    auto binding = std::make_shared<ProcessTreeTableDataBinding>(i, ps[i]);
    auto row = _Table->AppendRow(binding, 1);
    if (selectedProcess == ps[i]) {
      _Table->SetCursorRow(row);
    }
    i++;
  }
}

bool ProcessTree::OnKey(TermKeyCode key) {
  auto row = _Table->GetCursorRow();
  if (row == nullptr) {
    return false;
  }

  // Move cursor of selected row
  switch (key) {
  case KEY_UP: {
    auto prev = _Table->PrevRow(row);
    if (prev != nullptr) {
      auto prevBinding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(prev->GetBinding());
      if (prevBinding != nullptr) {
        _Table->SetCursorRow(prev);
      } else {
        MoveCursorAndDraw(-1);
      }
    } else {
      MoveCursorAndDraw(-1);
    }
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  }
  case KEY_DOWN: {
    auto next = _Table->NextRow(row);
    if (next != nullptr) {
      auto nextBinding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(next->GetBinding());
      if (nextBinding != nullptr) {
        _Table->SetCursorRow(next);
      } else {
        MoveCursorAndDraw(1);
      }
    } else {
      MoveCursorAndDraw(1);
    }
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  }
  case KEY_PPAGE: {
    MoveCursorAndDraw(-GetHeight());
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  }
  case KEY_NPAGE: {
    MoveCursorAndDraw(GetHeight());
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  }
  }

  // Handle key for selected row
  if (row->GetBinding()->OnKey(key)) {
    return true;
  }

  return false;
}

void ProcessTree::MoveCursorAndDraw(DisplayLength offset) {
  auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(_Table->GetCursorRow()->GetBinding());
  auto p = _ProcessCollection.MoveCursor(binding->GetProcess(), offset);
  auto ps = _ProcessCollection.GetAround(p, binding->GetIndex(), GetHeight());
  UpdateTable(p, ps);
}

std::shared_ptr<TableCellBinding>
ProcessTree::ProcessTreeTableHeaderBinding::CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                       std::shared_ptr<TableRowBinding> row) {
  return std::dynamic_pointer_cast<ProcessColumn>(column->GetBinding())->Header();
}

ProcessTree::ProcessTreeTableDataBinding::ProcessTreeTableDataBinding(DisplayLength index,
                                                                      std::shared_ptr<Process> process)
    : _Index(index), _Process(process) {}

std::shared_ptr<TableCellBinding>
ProcessTree::ProcessTreeTableDataBinding::CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                     std::shared_ptr<TableRowBinding> row) {
  return std::dynamic_pointer_cast<ProcessColumn>(column->GetBinding())
      ->Data(tree, column, std::dynamic_pointer_cast<ProcessTree::ProcessTreeTableDataBinding>(row));
}

void ProcessTree::ProcessTreeTableDataBinding::UpdateProcess(std::shared_ptr<Row> row,
                                                             std::shared_ptr<Process> process) {
  if (_Process != process) {
    _Process = process;
    for (auto cell : row->GetCells()) {
      std::dynamic_pointer_cast<ProcessDataAbstractCell>(cell->GetBinding())->OnRowBindingChanged();
    }
  }
}

} // namespace frontend::curses
