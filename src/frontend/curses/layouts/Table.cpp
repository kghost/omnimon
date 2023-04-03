#include "Table.hpp"

#include <cassert>
#include <ranges>
#include <vector>

#include "PlaceHolder.hpp"

namespace frontend::curses {

bool TableContainer::OnKey(TermKeyCode key) {
  if (auto ptr = _TableInputHandler.lock()) {
    return ptr->OnKey(key);
  } else {
    return false;
  }
}

void Column::SetArrangement(Container::ArrangementType arrangement) {
  _Arrangement = arrangement;
  _Table.ColumnSizeChanged();
}

void Column::SetSize(DisplayLength size) {
  _Size = size;
  _Table.ColumnSizeChanged();
}

Row::Row(Table& table, std::shared_ptr<TableBinding> binding, const std::vector<std::shared_ptr<Column>>& columns)
    : _RowContainer(std::make_shared<TableContainer>(binding, Container::GrowthType::LeftRight)) {
  table.GetTableContainer()->AppendChild(_RowContainer, GetRowContext());
  for (auto& column : columns) {
    auto cell = binding->OnNewCell(table, *this, *column);
    _RowContainer->AppendChild(cell, column);
  }
}

void Row::CalculateLayout() { _RowContainer->CalculateLayout(); }

void Row::MakeSimilarTo(const Row& other) { _RowContainer->MakeSimilarTo(*other._RowContainer); }

std::shared_ptr<const Row::RowContext> Row::GetRowContext() {
  static auto context = std::make_shared<RowContext>();
  return context;
}

Table::Table(std::shared_ptr<InputHandler> input, const ColumnBuilderInterface& columns)
    : _TableContainer(std::make_shared<TableContainer>(input, Container::GrowthType::TopDown)),
      _Columns(columns.BuildColumns(*this)) {}

void Table::ColumnSizeChanged() {
  auto& row1 = _Rows[0];
  row1->CalculateLayout();
  for (auto& row : _Rows) {
    row->MakeSimilarTo(*row1);
  }
}

Row& Table::AppendRow(std::shared_ptr<TableBinding> binding) {
  return *_Rows.emplace_back(std::make_unique<Row>(*this, binding, _Columns));
}

} // namespace frontend::curses
