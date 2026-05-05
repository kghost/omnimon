#include "Table.hpp"
#include "Container.hpp"
#include "View.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

namespace frontend::curses {

TableCell::TableCell(std::shared_ptr<Row> row, std::shared_ptr<Column> column,
                     std::shared_ptr<TableCellBinding> binding)
    : _Column(column), _Binding(binding), _View(binding->CreateView(row, column)) {}

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
    _Table.CalculateLayout();
  }
}
bool Column::OnKey(TermKeyCode key) { return false; }

Row::Row(Table& table, std::shared_ptr<TableRowBinding> binding, DisplayLength size)
    : _Table(table), _Arrangement(Container::ChildArrangement::ArrangementType::Forward, 0, 0), _Binding(binding),
      _Size(size), Container(Container::GrowthType::Horizontal) {}

Table::Table(InputHandler& input, TableCellFactory& cellFactory)
    : Container(Container::GrowthType::Vertical), _InputHandler(input), _CellFactory(cellFactory),
      _CursorRow(std::make_shared<backend::metrics::SimplePublisher<std::shared_ptr<Row>>>()),
      _CursorColumn(std::make_shared<backend::metrics::SimplePublisher<std::shared_ptr<Column>>>()) {}

Table::~Table() {
  // Clear all bindings
  for (auto column : _Columns) {
    column->ResetBinding();
  }

  for (auto row : GetRows()) {
    row->ResetBinding();
    for (auto cell : row->GetChildren()) {
      std::dynamic_pointer_cast<TableCell>(cell)->ResetBinding();
    }
  }
}

bool Table::OnKey(TermKeyCode key) { return _InputHandler.OnKey(key); }

void Table::DrawPrepare(const DrawContentContext& attrs) {
  Container::DrawPrepare(attrs);
  std::erase_if(_Columns, [](auto column) { return column->IsMarkForDeletion(); });
}

void Table::CalculateLayout() {
  auto rows = GetRows();
  if (rows.empty()) {
    return;
  }

  auto row1 = *rows.begin();
  row1->CalculateLayout();
  for (auto row : rows) {
    row->MakeSimilarTo(*row1);
  }
}

std::shared_ptr<Row> Table::AppendRow(std::shared_ptr<TableRowBinding> binding, DisplayLength size) {
  auto row = std::make_shared<Row>(*this, binding, size);
  for (auto& column : _Columns) {
    row->AppendChild(std::make_shared<TableCell>(row, column, _CellFactory.NewCell(*this, row, column)));
  }
  Container::AppendChild(row);
  return row;
}

std::shared_ptr<Column> Table::AppendColumn(std::shared_ptr<TableColumnBinding> binding,
                                            Container::ChildArrangement::ArrangementType arrangement,
                                            DisplayLength size, DisplayLength marginBefore, DisplayLength marginAfter) {
  auto column = std::make_shared<Column>(*this, binding, arrangement, size, marginBefore, marginAfter);
  _Columns.push_back(column);
  for (auto row : GetRows()) {
    row->AppendChild(std::make_shared<TableCell>(row, column, _CellFactory.NewCell(*this, row, column)));
  }
  return column;
}

template <typename T> std::shared_ptr<T> FindNext(std::ranges::view auto collection, std::shared_ptr<T> target) {
  auto it = std::ranges::find(collection, target);
  if (it == collection.end() || std::next(it) == collection.end()) {
    return nullptr;
  }
  return *std::next(it);
}

std::shared_ptr<Row> Table::NextRow(const std::shared_ptr<Row>& row) { return FindNext(GetRows(), row); }

std::shared_ptr<Row> Table::PrevRow(const std::shared_ptr<Row>& row) {
  return FindNext(GetRows() | std::views::reverse, row);
}

std::shared_ptr<Column> Table::NextColumn(const std::shared_ptr<Column>& column) {
  return FindNext(GetColumns(), column);
}

std::shared_ptr<Column> Table::PrevColumn(const std::shared_ptr<Column>& column) {
  return FindNext(GetColumns() | std::views::reverse, column);
}

} // namespace frontend::curses
