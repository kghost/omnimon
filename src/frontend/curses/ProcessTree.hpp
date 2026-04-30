#pragma once

#include <memory>
#include <vector>

#include "../../backend/metrics/Gauge.hpp"
#include "ProcessOrder.hpp"
#include "layouts/Table.hpp"

namespace frontend::curses {

class ProcessTree : public InputHandler, public TableCellFactory {
public:
  explicit ProcessTree();

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
  std::shared_ptr<backend::metrics::Gauge> GetCursor() const { return _Cursor; };

private:
  static constexpr DisplayLength _TableHeaderHeight = 1;
  DisplayLength GetHeight() const;
  auto GetDataRows() { return _Table->GetRows() | std::views::drop(1); }
  void UpdateTable(const std::vector<std::shared_ptr<frontend::curses::Process>>& ps,
                   std::span<std::shared_ptr<Row>> rows);
  void MoveCursorAndDraw(DisplayLength offset);

  ProcessCollection _ProcessCollection;
  std::shared_ptr<Table> _Table;
  std::shared_ptr<backend::metrics::SimpleGauge> _Cursor;
};

} // namespace frontend::curses
