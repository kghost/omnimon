#pragma once

#include <memory>

#include "ProcessOrder.hpp"
#include "layouts/Table.hpp"

namespace frontend::curses {

class ProcessTree : public InputHandler, public TableCellFactory {
public:
  explicit ProcessTree();
  ~ProcessTree() override = default;

  class ProcessTreeTableRowBinding : public TableRowBinding {
  public:
    virtual std::shared_ptr<TableCellBinding> CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                         std::shared_ptr<TableRowBinding> row) = 0;
  };

  class ProcessTreeTableHeaderBinding : public ProcessTreeTableRowBinding {
  public:
    std::shared_ptr<TableCellBinding> CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                 std::shared_ptr<TableRowBinding> row) override;
  };

  class ProcessTreeTableDataBinding : public ProcessTreeTableRowBinding {
  public:
    explicit ProcessTreeTableDataBinding(DisplayLength index, std::shared_ptr<Process> process);

    DisplayLength GetIndex() const { return _Index; }
    std::shared_ptr<TableCellBinding> CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                 std::shared_ptr<TableRowBinding> row) override;
    std::shared_ptr<Process> GetProcess() const { return _Process; }
    void UpdateProcess(std::shared_ptr<Row> row, std::shared_ptr<Process> process);

  private:
    DisplayLength _Index;
    std::shared_ptr<Process> _Process;
  };

  std::shared_ptr<TableCellBinding> NewCell(Table& table, std::shared_ptr<Row> row,
                                            std::shared_ptr<Column> column) override;
  std::shared_ptr<View> GetView() { return _Table; }

  void Update();
  bool OnKey(TermKeyCode key) override;

  std::shared_ptr<Table> GetTable() { return _Table; }

private:
  static constexpr DisplayLength _TableHeaderHeight = 1;
  DisplayLength GetHeight() const;
  auto GetDataRows() { return _Table->GetRows() | std::views::drop(1); }
  void UpdateTable(std::shared_ptr<frontend::curses::Process> selectedProcess,
                   const std::vector<std::shared_ptr<frontend::curses::Process>>& ps);
  void MoveCursorAndDraw(DisplayLength offset);

  ProcessCollection _ProcessCollection;
  std::shared_ptr<Table> _Table;
};

} // namespace frontend::curses
