#include "Table.hpp"
#include "Container.hpp"
#include "View.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace frontend::curses {

TableCell::TableCell(std::shared_ptr<Row> row, std::shared_ptr<Column> column,
                     std::shared_ptr<TableCellBinding> binding)
    : _Row(row), _Column(column), _Binding(binding), _View(binding->CreateView(row, column)) {}

Container::ChildArrangement::ArrangementType TableCell::GetArrangement() const {
  return _Column->GetArrangement().GetArrangement();
}
DisplayLength TableCell::GetSize() const { return _Column->GetSize(); }
DisplayLength TableCell::GetMarginBefore() const { return _Column->GetArrangement().GetMarginBefore(); }
DisplayLength TableCell::GetMarginAfter() const { return _Column->GetArrangement().GetMarginAfter(); }

Column::Column(Table& table, std::shared_ptr<TableColumnBinding> binding,
               Container::ChildArrangement::ArrangementType arrangement, DisplayLength size, DisplayLength marginBefore,
               DisplayLength marginAfter)
    : _Table(table), _Arrangement(arrangement, marginBefore, marginAfter), _Binding(binding), _Size(size) {}

void Column::SetSize(DisplayLength size) {
  if (_Size != size) {
    _Size = size;
    _Table.RearrangeLayout();
  }
}
bool Column::OnKey(TermKeyCode key) { return false; }

Row::Row(Table& table, std::shared_ptr<TableRowBinding> binding, DisplayLength size)
    : _Table(table), _Arrangement(Container::ChildArrangement::ArrangementType::Forward, 0, 0), _Binding(binding),
      _Size(size), Container(Container::GrowthType::Horizontal) {
  // for (auto& column : columns) {
  //   // Create a new column instance for this row that shares the same layout info
  //   // But wait, Column is already a Child. We should probably create a new one
  //   // or make Column a factory.
  //   // Actually, the 'columns' passed in are the master columns from the table.
  //   // We should create a new Column child for this Row.
  //   auto rowColumn = std::make_shared<Column>(table, column->GetArrangement(), column->GetSize());
  //   auto cell = _Binding->OnNewCell(table, *this, *rowColumn);
  //   rowColumn->BindCell(cell);
  //   AppendChild(rowColumn);
  // }
}

Table::Table(InputHandler& input, TableCellFactory& cellFactory)
    : Container(Container::GrowthType::Vertical), _InputHandler(input), _CellFactory(cellFactory) {}

bool Table::OnKey(TermKeyCode key) { return _InputHandler.OnKey(key); }

bool Table::SetLayout(Layout offset, Layout layout) { return Container::SetLayout(offset, layout); }

void Table::RearrangeLayout() {
  if (_Rows.empty()) {
    return;
  }
  auto& row1 = _Rows[0];
  row1->CalculateLayout();
  for (auto& row : _Rows) {
    row->MakeSimilarTo(*row1);
  }
}

std::shared_ptr<Row> Table::AppendRow(std::shared_ptr<TableRowBinding> binding, DisplayLength size) {
  auto row = std::make_shared<Row>(*this, binding, size);
  for (auto& column : _Columns) {
    row->AppendChild(std::make_shared<TableCell>(row, column, _CellFactory.NewCell(*this, row, column)));
  }
  Container::AppendChild(row);
  _Rows.push_back(row);
  return row;
}

std::shared_ptr<Column> Table::AppendColumn(std::shared_ptr<TableColumnBinding> binding,
                                            Container::ChildArrangement::ArrangementType arrangement,
                                            DisplayLength size, DisplayLength marginBefore, DisplayLength marginAfter) {
  auto column = std::make_shared<Column>(*this, binding, arrangement, size, marginBefore, marginAfter);
  _Columns.push_back(column);
  for (auto& row : _Rows) {
    row->AppendChild(std::make_shared<TableCell>(row, column, _CellFactory.NewCell(*this, row, column)));
  }
  return column;
}

} // namespace frontend::curses
