#pragma once

#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>
#include <list>
#include <memory>

#include "../../backend/metrics/Binding.hpp"
#include "../../backend/metrics/SimplePublisher.hpp"
#include "ProcessOrder.hpp"
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
    std::shared_ptr<backend::process::Process> ProcessPtr;

    // Column associated data, stored as strings cached from updaters
    std::string StateDisplay;
    std::string CpuDisplay;
    std::string MemDisplay;
    std::string TimeDisplay;
    std::string DiskReadDisplay;
    std::string DiskWriteDisplay;
    std::string DiskAccumulatedDisplay;
    std::string IODisplay;
    std::string IOAccumulatedDisplay;

    // Updaters for each column
    std::shared_ptr<backend::metrics::SubscriberBase> StateUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> CpuUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> MemUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> TimeUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> DiskReadUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> DiskWriteUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> DiskAccumulatedUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> IOUpdater;
    std::shared_ptr<backend::metrics::SubscriberBase> IOAccumulatedUpdater;
  };
  class Column {
  public:
    virtual ~Column() = default;
    virtual std::string GetHeaderText() const = 0;
    virtual void RegisterRow(Row& row) const = 0;
    virtual std::string GetDataText(bool isRowSelected, bool isColumnSelected, Row& row) const = 0;
    virtual void Decorate(::ftxui::TableSelection selection) const = 0;
  };

private:
  std::function<void()> _Refresh;

  static constexpr const ssize_t HeaderHeight = 1; // Number of rows reserved for the header
  ssize_t _TableCapacity = 0;                      // Current height of the table (excluding header)
  ProcessCollection _ProcessCollection;
  std::list<std::unique_ptr<Row>> _Rows;                      // Row definitions (e.g., which process to display)
  std::list<std::unique_ptr<Column>> _Columns;                // Column definitions (e.g., width, header text)
  std::list<std::unique_ptr<Row>>::iterator _CursorRow;       // Current position of the cursor in the rows
  std::list<std::unique_ptr<Column>>::iterator _CursorColumn; // Current position of the cursor in the columns
  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;

  void OnTableSizeChange(::ftxui::Box box);
  void Update();
  void MoveCursorAndDraw(ssize_t offset);
  void UpdateData(std::shared_ptr<backend::process::Process> selectedProcess,
                  std::vector<std::shared_ptr<backend::process::Process>> ps);

  std::list<std::unique_ptr<Column>> CreateDefaultColumns();
};

} // namespace frontend::ftxui
