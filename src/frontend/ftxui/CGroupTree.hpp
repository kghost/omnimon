#pragma once

#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "../../backend/cgroupv2/CGroupManager.hpp"
#include "../../backend/cgroupv2/CGroupMetrics.hpp"
#include "../../backend/cgroupv2/CGroupNode.hpp"
#include "../../backend/events/Events.hpp"
#include "../../backend/metrics/Binding.hpp"
#include "../../backend/metrics/SimplePublisher.hpp"
#include "../../utils/Formatter.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

class CGroupTree : public FtxuiTabView {
public:
  explicit CGroupTree(backend::events::EventLoop& loop, std::shared_ptr<backend::metrics::SimplePublisher<int>> tick,
                      std::function<void()> refresh);
  ~CGroupTree() override = default;

  std::string GetTabName() const override;
  bool OnEvent(::ftxui::Event event) override;
  ::ftxui::Element Render() override;

  class Row {
  public:
    explicit Row(std::shared_ptr<backend::cgroupv2::CGroupNode> node);

    std::shared_ptr<backend::cgroupv2::CGroupNode> Node;
    std::string Name;
    std::string PathDisplay;
    std::string MemoryDisplay;
    std::string PidsDisplay;
    std::string CpuUsageDisplay;
    std::string CpuUserDisplay;
    std::string CpuSystemDisplay;
    backend::cgroupv2::CGroupMetrics Metrics;

    void UpdateMetrics();
  };

  class Column {
  public:
    virtual ~Column() = default;
    virtual std::string GetHeaderText() const = 0;
    virtual std::string GetDataText(bool isRowSelected, bool isColumnSelected, Row& row) const = 0;
    virtual void Decorate(::ftxui::TableSelection selection) const = 0;
  };

private:
  std::function<void()> _Refresh;

  static constexpr const ssize_t HeaderHeight = 1;
  ssize_t _TableCapacity = 0;
  backend::cgroupv2::CGroupManager _Manager;
  std::list<std::unique_ptr<Row>> _Rows;
  std::list<std::unique_ptr<Column>> _Columns;
  std::list<std::unique_ptr<Row>>::iterator _CursorRow;
  std::list<std::unique_ptr<Column>>::iterator _CursorColumn;
  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;

  void OnTableSizeChange(::ftxui::Box box);
  void Update();
  void MoveCursorAndDraw(ssize_t offset);
  void UpdateData(std::vector<std::pair<std::shared_ptr<backend::cgroupv2::CGroupNode>, ssize_t>> nodes);
  std::vector<std::pair<std::shared_ptr<backend::cgroupv2::CGroupNode>, ssize_t>> GetTreeNodes();
  void UpdateRowDisplay(Row& row, ssize_t depth);

  std::list<std::unique_ptr<Column>> CreateDefaultColumns();
};

} // namespace frontend::ftxui
