#pragma once

#include <set>
#include <vector>

#include "../../backend/metrics/Gauge.hpp"
#include "../../backend/process/ProcessListing.hpp"
#include "ProcessOrder.hpp"
#include "layouts/Table.hpp"

namespace frontend::curses {

class ProcessCell;

class ProcessTree {
public:
  explicit ProcessTree();

  class ProcessTreeTableHeaderBinding : public TableBinding {
  public:
    explicit ProcessTreeTableHeaderBinding() = default;
    ~ProcessTreeTableHeaderBinding() override = default;

    bool OnKey(TermKeyCode key) override;
    std::shared_ptr<View> OnNewCell(Table& table, Row& row, Column& column) override;
  };

  class ProcessTreeTableRowBinding : public TableBinding {
  public:
    explicit ProcessTreeTableRowBinding(ProcessTree& tree, size_t index, std::shared_ptr<Process> process);
    ~ProcessTreeTableRowBinding() override = default;

    size_t GetIndex() const { return _Index; }
    bool OnKey(TermKeyCode key) override;
    std::shared_ptr<View> OnNewCell(Table& table, Row& row, Column& column) override;
    std::shared_ptr<Process> GetProcess() const { return _Process; }
    void UpdateProcess(std::shared_ptr<Process> process);

  private:
    ProcessTree& _Tree;
    size_t _Index;
    std::shared_ptr<Process> _Process;
    std::vector<std::shared_ptr<ProcessCell>> _Cells;
  };

  class TableInputHandler : public InputHandler {
  public:
    explicit TableInputHandler(ProcessTree& tree) : _ProcessTree(tree) {}
    ~TableInputHandler() override = default;

    bool OnKey(TermKeyCode key) override;

  private:
    ProcessTree& _ProcessTree;
  };

  class ProcessTreeState {
  public:
    explicit ProcessTreeState() = default;
  };

  std::shared_ptr<View> GetView() const;
  void Update();
  bool OnKey(TermKeyCode key);
  std::shared_ptr<backend::metrics::Gauge> GetCursor() const { return _Cursor; };

private:
  DisplayLength GetHeight() const;
  void UpdateTable(const std::vector<std::shared_ptr<frontend::curses::Process>>& ps);
  void MoveCursorAndDraw(DisplayLength offset);

  ProcessCollection _ProcessCollection;
  std::shared_ptr<InputHandler> _TableInputHandler;
  Table _Table;
  std::shared_ptr<backend::metrics::SimpleGauge> _Cursor;
  std::vector<std::shared_ptr<ProcessTreeTableRowBinding>> _Rows;
};

} // namespace frontend::curses
