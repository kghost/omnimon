#pragma once

#include <memory>
#include <ranges> // C++20
#include <vector>

#include "Container.hpp"
#include "View.hpp"

namespace frontend::curses {

class Table;
class Column;
class Row;

class TableCellBinding : public InputHandler {
public:
  explicit TableCellBinding() = default;
  virtual ~TableCellBinding() = default;

  bool OnKey(TermKeyCode key) override { return false; }
  virtual std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) = 0;
};

class TableCell : public Container::Child {
public:
  explicit TableCell(std::shared_ptr<Row> row, std::shared_ptr<Column> column,
                     std::shared_ptr<TableCellBinding> binding);

  Container::ChildArrangement::ArrangementType GetArrangement() const override;
  DisplayLength GetSize() const override;
  DisplayLength GetMarginBefore() const override;
  DisplayLength GetMarginAfter() const override;
  View& GetView() override { return *_View; }

  std::shared_ptr<TableCellBinding> GetBinding() { return _Binding; }

private:
  std::shared_ptr<Row> _Row;
  std::shared_ptr<Column> _Column;
  std::shared_ptr<TableCellBinding> _Binding;
  std::shared_ptr<View> _View;
};

class TableCellFactory {
public:
  explicit TableCellFactory() = default;
  virtual ~TableCellFactory() = default;
  virtual std::shared_ptr<TableCellBinding> NewCell(Table& table, std::shared_ptr<Row> row,
                                                    std::shared_ptr<Column> column) = 0;
};

class TableRowBinding : public InputHandler {
public:
  explicit TableRowBinding() = default;
  virtual ~TableRowBinding() = default;
  bool OnKey(TermKeyCode key) override { return false; }
};

class TableColumnBinding : public InputHandler {
public:
  explicit TableColumnBinding() = default;
  virtual ~TableColumnBinding() = default;
  bool OnKey(TermKeyCode key) override { return false; }
};

class Column : public InputHandler {
public:
  explicit Column(Table& table, std::shared_ptr<TableColumnBinding> binding,
                  Container::ChildArrangement::ArrangementType arrangement, DisplayLength size,
                  DisplayLength marginBefore, DisplayLength marginAfter);
  virtual ~Column() = default;

  Column(const Column&) = delete;
  Column(Column&&) = delete;
  Column& operator=(const Column&) = delete;
  Column& operator=(Column&&) = delete;

  const Container::ChildArrangement& GetArrangement() const { return _Arrangement; }
  DisplayLength GetSize() const { return _Size; }
  void SetSize(DisplayLength size);
  std::shared_ptr<TableColumnBinding> GetBinding() const { return _Binding; }

  bool OnKey(TermKeyCode key) override;

private:
  Table& _Table;
  std::shared_ptr<TableColumnBinding> _Binding;
  Container::ChildArrangement _Arrangement;
  DisplayLength _Size;
};

class ColumnBuilderInterface {
public:
  virtual std::vector<std::shared_ptr<Column>> BuildColumns(Table& table) const = 0;
};

template <typename... Columns> class ColumnBuilder : public ColumnBuilderInterface {
  std::vector<std::shared_ptr<Column>> BuildColumns(Table& table) const override {
    return {std::make_shared<Columns>(table)...};
  }
};

class Row : public Container::Child, public Container {
public:
  // static Layout InitailSize(Container& parent, DisplayLength size) {
  //   if (parent._Growth == GrowthType::Vertical) {
  //     return {size, parent.GetLayout().Width};
  //   } else {
  //     return {parent.GetLayout().Height, size};
  //   }
  // }

  explicit Row(Table& table, std::shared_ptr<TableRowBinding> binding, DisplayLength size);
  virtual ~Row() = default;

  Row(const Row&) = delete;
  Row(Row&&) = delete;
  Row& operator=(const Row&) = delete;
  Row& operator=(Row&&) = delete;

  Container::ChildArrangement::ArrangementType GetArrangement() const override { return _Arrangement.GetArrangement(); }
  DisplayLength GetSize() const override { return _Size; }
  DisplayLength GetMarginBefore() const override { return _Arrangement.GetMarginBefore(); }
  DisplayLength GetMarginAfter() const override { return _Arrangement.GetMarginAfter(); }
  View& GetView() override { return *this; }

  std::shared_ptr<TableRowBinding> GetBinding() const { return _Binding; }
  auto GetCells() {
    return std::views::transform(GetChildren(), [](std::shared_ptr<Container::Child> child) {
      return std::dynamic_pointer_cast<TableCell>(child);
    });
  }

private:
  Table& _Table;
  std::shared_ptr<TableRowBinding> _Binding;
  Container::ChildArrangement _Arrangement;
  DisplayLength _Size;
};

class Table : public Container {
public:
  explicit Table(InputHandler& input, TableCellFactory& cellFactory);
  ~Table() = default;

  Table(const Table&) = delete;
  Table(Table&&) = delete;
  Table& operator=(const Table&) = delete;
  Table& operator=(Table&&) = delete;

  bool OnKey(TermKeyCode key) override;
  bool SetLayout(Layout offset, Layout layout) override;

  void RearrangeLayout();
  std::span<std::shared_ptr<Row>> GetRows() { return _Rows; }
  std::shared_ptr<Row> AppendRow(std::shared_ptr<TableRowBinding> binding, DisplayLength size);

  std::span<std::shared_ptr<Column>> GetColumns() { return _Columns; }
  std::shared_ptr<Column> AppendColumn(std::shared_ptr<TableColumnBinding> binding,
                                       Container::ChildArrangement::ArrangementType arrangement, DisplayLength size,
                                       DisplayLength marginBefore, DisplayLength marginAfter);

private:
  InputHandler& _InputHandler;
  TableCellFactory& _CellFactory;
  std::vector<std::shared_ptr<Column>> _Columns;
  std::vector<std::shared_ptr<Row>> _Rows;
};

} // namespace frontend::curses