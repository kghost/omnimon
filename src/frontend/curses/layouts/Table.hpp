#pragma once

#include "Container.hpp"
#include "TextView.hpp"

namespace frontend::curses {

class Table;
class Column;
class Row;

class TableContainer : public Container {
  // A container which propagates key events to the input handler.
public:
  explicit TableContainer(std::shared_ptr<InputHandler> input, GrowthType growth)
      : Container(growth), _TableInputHandler(input) {}
  ~TableContainer() override = default;

  bool OnKey(TermKeyCode key) override;

private:
  std::weak_ptr<InputHandler> _TableInputHandler;
};

class TableBinding : public InputHandler {
public:
  explicit TableBinding() = default;
  virtual ~TableBinding() = default;

  virtual std::shared_ptr<View> OnNewCell(Table& table, Row& row, Column& column) = 0;
};

class Column : public Container::Context {
public:
  explicit Column(Table& table, Container::ArrangementType arrangement, DisplayLength size)
      : _Table(table), _Arrangement(arrangement), _Size(size) {}
  virtual ~Column() = default;

  Column(const Column&) = delete;
  Column(Column&&) = delete;
  Column& operator=(const Column&) = delete;
  Column& operator=(Column&&) = delete;

  Container::ArrangementType GetArrangement() const override { return _Arrangement; }
  DisplayLength GetSize() const override { return _Size; }

  void SetArrangement(Container::ArrangementType arrangement);
  void SetSize(DisplayLength size);

private:
  Table& _Table;
  Container::ArrangementType _Arrangement;
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

class Row {
public:
  explicit Row(Table& table, std::shared_ptr<TableBinding> binding,
               const std::vector<std::shared_ptr<Column>>& columns);
  virtual ~Row() = default;

  Row(const Row&) = delete;
  Row(Row&&) = delete;
  Row& operator=(const Row&) = delete;
  Row& operator=(Row&&) = delete;

  void SetVisible(bool visible) { _RowContainer->SetVisible(visible); }

  void CalculateLayout();
  void MakeSimilarTo(const Row& other);

private:
  class RowContext : public Container::Context {
  public:
    explicit RowContext() = default;
    ~RowContext() override = default;

    Container::ArrangementType GetArrangement() const override { return Container::ArrangementType::Forward; }
    DisplayLength GetSize() const override { return 1; }
  };

  static std::shared_ptr<const RowContext> GetRowContext();

  const std::shared_ptr<Container> _RowContainer;
};

class Table {
public:
  explicit Table(std::shared_ptr<InputHandler> input, const ColumnBuilderInterface& columns);
  ~Table() = default;

  Table(const Table&) = delete;
  Table(Table&&) = delete;
  Table& operator=(const Table&) = delete;
  Table& operator=(Table&&) = delete;

  std::shared_ptr<Container> GetTableContainer() const { return _TableContainer; }
  void ColumnSizeChanged();
  const std::vector<std::unique_ptr<Row>>& GetRows() const { return _Rows; }
  Row& AppendRow(std::shared_ptr<TableBinding> binding);

private:
  const std::shared_ptr<Container> _TableContainer;
  const std::vector<std::shared_ptr<Column>> _Columns;
  std::vector<std::unique_ptr<Row>> _Rows;
};

} // namespace frontend::curses