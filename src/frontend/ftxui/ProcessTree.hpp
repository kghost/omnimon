#pragma once

#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>
#include <functional>
#include <list>
#include <memory>

#include "../../backend/metrics/Binding.hpp"
#include "../../backend/metrics/SimplePublisher.hpp"
#include "../../backend/process/Process.hpp"
#include "../../backend/process/ProcessManager.hpp"
#include "../../backend/process/ProcessMetrics.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

class ProcessTree : public FtxuiTabView {
public:
  explicit ProcessTree(std::shared_ptr<backend::metrics::SimplePublisher<int>> tick, std::function<void()> refresh);
  ~ProcessTree() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;

  class Row {
  public:
    explicit Row(ProcessTree& tree, backend::process::Process& process);

    ProcessTree& Tree;
    std::optional<std::reference_wrapper<backend::process::Process>> Process;
    backend::process::ProcessMetrics Metrics;
    std::shared_ptr<backend::metrics::SubscriberBase> OnNodeRemoving;

    struct MetricEntry {
      std::string Display;
      std::shared_ptr<backend::metrics::SubscriberBase> Updater;
    };

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
  class Column {
  public:
    virtual ~Column() = default;
    virtual std::string GetHeaderText() const = 0;
    virtual void RegisterRow(Row& row) const = 0;
    virtual std::string GetDataText(bool isRowSelected, bool isColumnSelected, Row& row) const = 0;
    virtual void Decorate(::ftxui::TableSelection selection) const = 0;
  };

  void OnNodeRemoving(Row& row);

private:
  std::function<void()> _Refresh;

  static constexpr const ssize_t HeaderHeight = 1; // Number of rows reserved for the header
  ssize_t _TableCapacity = 0;                      // Current height of the table (excluding header)
  backend::process::ProcessManager _ProcessManager;
  std::list<std::unique_ptr<Row>> _Rows;                      // Row definitions (e.g., which process to display)
  std::list<std::unique_ptr<Column>> _Columns;                // Column definitions (e.g., width, header text)
  std::list<std::unique_ptr<Row>>::iterator _CursorRow;       // Current position of the cursor in the rows
  std::list<std::unique_ptr<Column>>::iterator _CursorColumn; // Current position of the cursor in the columns
  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;

  void OnTableSizeChange(::ftxui::Box box);
  void Update();
  void MoveCursorAndDraw(ssize_t offset);
  void UpdateData(backend::process::Process& selectedProcess,
                  std::vector<std::reference_wrapper<backend::process::Process>> ps);
  void RemoveData();

  std::list<std::unique_ptr<Column>> CreateDefaultColumns();
};

} // namespace frontend::ftxui
