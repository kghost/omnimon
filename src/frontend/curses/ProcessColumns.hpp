#pragma once

#include <memory>
#include <string>

#include "ProcessTree.hpp"
#include "layouts/Table.hpp"
#include "layouts/TextView.hpp"

namespace frontend::curses {

class ProcessHeaderCell : public TableCellBinding {
public:
  ProcessHeaderCell(TextView::Align align, std::string text);
  std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) override;

private:
  std::shared_ptr<TextView> _View;
};

class ProcessDataAbstractCell : public TableCellBinding {
public:
  virtual void OnRowBindingChanged() = 0;
};

class ProcessDataCell : public ProcessDataAbstractCell {
public:
  explicit ProcessDataCell(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row, TextView::Align align);

  std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) override;

protected:
  std::shared_ptr<Column> _Column;
  std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> _Row;
  std::shared_ptr<TextView> _View;
};

class ProcessColumn : public TableColumnBinding {
public:
  virtual std::shared_ptr<TableCellBinding> Header() const = 0;
  virtual std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                                 std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) = 0;
};

class ProcessColumnCursor : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnPid : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnState : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnUser : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnCpu : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnMem : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnTime : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnDiskRead : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnDiskWrite : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnDiskAccumulated : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnIO : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnIOAccumulated : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnStart : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

class ProcessColumnCommand : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override;
  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override;
};

} // namespace frontend::curses
