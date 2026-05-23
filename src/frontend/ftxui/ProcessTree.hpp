#pragma once

#include <functional>

#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>

#include "../../backend/process/Process.hpp"
#include "../../backend/process/ProcessManager.hpp"
#include "../../backend/process/ProcessMetrics.hpp"
#include "../../utils/Range.hpp"
#include "TabSelector.hpp"
#include "TableBase.hpp"

namespace frontend::ftxui {

class ProcessTree;
class ProcessRow;

class ProcessRow : public TableBase<ProcessTree, ProcessRow, backend::process::Process>::RowBase {
public:
  explicit ProcessRow(ProcessTree& tree, backend::process::Process& process);

  ProcessRow(const ProcessRow&) = delete;
  ProcessRow& operator=(const ProcessRow&) = delete;
  ProcessRow(ProcessRow&&) noexcept = delete;
  ProcessRow& operator=(ProcessRow&&) noexcept = delete;

  void UpdateMetrics(ProcessTree&) { Metrics.UpdateMetrics(Node.value().get()); }

  backend::process::ProcessMetrics Metrics;

  MetricEntry State;
  MetricEntry Cpu;
  MetricEntry Mem;
  MetricEntry Time;
  MetricEntry DiskRead;
  MetricEntry DiskWrite;
  MetricEntry DiskAccumulated;
  MetricEntry Io;
  MetricEntry IoAccumulated;
};

class ProcessTree : public TableBase<ProcessTree, ProcessRow, backend::process::Process> {
public:
  explicit ProcessTree(OmniMonInterface& omniMon);
  ~ProcessTree() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override { return OnEventImpl(event); }
  ::ftxui::Element Render() override { return RenderImpl(); }

  using Row = ProcessRow;
  using Column = TableBase<ProcessTree, ProcessRow, backend::process::Process>::ColumnBase;

private:
  friend class TableBase<ProcessTree, ProcessRow, backend::process::Process>;

  backend::process::ProcessManager _Manager;
  std::list<std::unique_ptr<Column>> _Columns; // Column definitions

  utils::AnyView<Column&> GetAllColumns() const;
  void PreUpdate();

  std::list<std::unique_ptr<Column>> CreateDefaultColumns();
};

} // namespace frontend::ftxui
