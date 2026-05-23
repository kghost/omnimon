#include "ProcessTree.hpp"

#include "../../backend/process/ProcessMetrics.hpp"
#include "ProcessColumns.hpp"

namespace frontend::ftxui {

ProcessRow::ProcessRow(ProcessTree& tree, backend::process::Process& process)
    : RowBase(tree, process, process.OnNodeRemoving([this](bool removed) {
        if (removed) {
          this->Table.OnRowRemoving(*this);
        }
      })),
      Metrics(process) {}

ProcessTree::ProcessTree(OmniMonInterface& omniMon)
    : TableBase(omniMon, [this](auto) { Update(); }), _Columns(CreateDefaultColumns()) {}

std::string ProcessTree::GetTabName() const { return kProcessTreeTabName; }

void ProcessTree::PreUpdate() { _Manager.UpdateList(); }

utils::AnyView<ProcessTree::Column&> ProcessTree::GetAllColumns() const {
  return utils::AnyView<Column&>(std::views::all(_Columns) |
                                 std::views::transform([](const auto& column) -> Column& { return *column; }));
}

std::list<std::unique_ptr<ProcessTree::Column>> ProcessTree::CreateDefaultColumns() {
  std::list<std::unique_ptr<Column>> columns;
  columns.push_back(std::make_unique<ColumnCursor>());
  columns.push_back(std::make_unique<ColumnPid>());
  columns.push_back(std::make_unique<ColumnState>());
  columns.push_back(std::make_unique<ColumnUser>());
  columns.push_back(std::make_unique<ColumnCpu>());
  columns.push_back(std::make_unique<ColumnMem>());
  columns.push_back(std::make_unique<ColumnTime>());
  columns.push_back(std::make_unique<ColumnDiskRead>());
  columns.push_back(std::make_unique<ColumnDiskWrite>());
  columns.push_back(std::make_unique<ColumnDiskAccumulated>());
  columns.push_back(std::make_unique<ColumnIO>());
  columns.push_back(std::make_unique<ColumnIOAccumulated>());
  columns.push_back(std::make_unique<ColumnStart>());
  columns.push_back(std::make_unique<ColumnCommand>());
  return columns;
}

} // namespace frontend::ftxui
