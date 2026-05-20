#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/box.hpp>

#include "OmniMonInterface.hpp"
#include "TabSelector.hpp"

namespace frontend::ftxui {

// Generic pure-CRTP table view base class.
template <typename Impl, typename RowType, typename NodeType> class TableBase : public FtxuiTabView {
public:
  class RowBase {
  public:
    explicit RowBase(Impl& table, NodeType& node, std::shared_ptr<backend::metrics::SubscriberBase> removingSubscriber)
        : Table(table), Node(node), OnNodeRemoving(removingSubscriber) {}
    ~RowBase() = default;

    RowBase(const RowBase&) = delete;
    RowBase& operator=(const RowBase&) = delete;
    RowBase(RowBase&&) = delete;
    RowBase& operator=(RowBase&&) = delete;

    struct MetricEntry {
      std::string Display;
      std::shared_ptr<backend::metrics::SubscriberBase> Updater;
    };

    Impl& Table;
    std::optional<std::reference_wrapper<NodeType>> Node;
    std::shared_ptr<backend::metrics::SubscriberBase> OnNodeRemoving;
  };

  class ColumnBase {
  public:
    ColumnBase() = default;
    virtual ~ColumnBase() = default;

    ColumnBase(const ColumnBase&) = delete;
    ColumnBase& operator=(const ColumnBase&) = delete;
    ColumnBase(ColumnBase&&) = delete;
    ColumnBase& operator=(ColumnBase&&) = delete;

    virtual std::string GetHeaderText() const = 0;
    virtual void RegisterRow(RowType& row) const = 0;
    virtual std::string GetDataText(bool isRowSelected, RowType& row) const = 0;
    virtual void Decorate(::ftxui::TableSelection selection) const = 0;

    bool IsShown = true;
  };

  explicit TableBase(OmniMonInterface& interface, std::function<void(int)> onTick)
      : _Interface(interface), _CursorRow(_Rows.end()), _TickUpdater(_Interface.OnTickUpdate(onTick)) {}
  ~TableBase() override = default;

  // Disallow copying/moving.
  TableBase(const TableBase&) = delete;
  TableBase& operator=(const TableBase&) = delete;
  TableBase(TableBase&&) = delete;
  TableBase& operator=(TableBase&&) = delete;

  // ---------------------------------------------------------------------
  // FtxuiView overrides
  // ---------------------------------------------------------------------
  bool OnEventImpl(this Impl& self, ::ftxui::Event event) {
    if (self._Rows.empty()) {
      return false;
    }
    if (event == ::ftxui::Event::ArrowUp) {
      if (self._CursorRow != self._Rows.begin()) {
        self._CursorRow = std::prev(self._CursorRow);
      } else {
        self.MoveCursorAndDraw(-1);
      }
      return true;
    } else if (event == ::ftxui::Event::ArrowDown) {
      if (self._CursorRow != std::prev(self._Rows.end())) {
        self._CursorRow = std::next(self._CursorRow);
      } else {
        self.MoveCursorAndDraw(1);
      }
      return true;
    } else if (event == ::ftxui::Event::PageUp) {
      self.MoveCursorAndDraw(-self._TableCapacity);
      return true;
    } else if (event == ::ftxui::Event::PageDown) {
      self.MoveCursorAndDraw(self._TableCapacity);
      return true;
    }
    return false;
  }

  ::ftxui::Element RenderImpl(this Impl& self) {
    using namespace ::ftxui;
    if (self._Rows.empty()) {
      return text("Loading...") | center | reflect([&self](Box box) { self.OnTableSizeChange(box); });
    }
    auto columns = self.GetAllColumns() | std::views::filter([](const auto& c) { return c.IsShown; });

    std::vector<std::vector<std::string>> data;
    // Header row
    data.push_back(columns | std::views::transform([](const auto& c) { return c.GetHeaderText(); }) |
                   std::ranges::to<std::vector<std::string>>());
    // Data rows
    for (auto& row : self._Rows) {
      if (row.Node.has_value()) {
        data.push_back(columns | std::views::transform([&](const auto& c) {
                         bool selected = (std::addressof(row) == std::to_address(self._CursorRow));
                         return c.GetDataText(selected, row);
                       }) |
                       std::ranges::to<std::vector<std::string>>());
      }
    }
    auto table = ::ftxui::Table(data);
    for (auto [idx, col] : std::views::enumerate(columns)) {
      col.Decorate(table.SelectColumn(idx));
    }
    table.SelectAll().DecorateSeparatorVertical(
        ::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
    table.SelectRow(0).Decorate(bold);
    table.SelectRow(0).Decorate(underlined);
    return table.Render() | frame | reflect([&self](Box box) { self.OnTableSizeChange(box); });
  }

  void OnRowRemoving(RowType& row) {
    assert(!_Rows.empty());
    assert(row.Node.has_value());
    auto& node = row.Node.value().get();
    row.Node.reset();
    if (_CursorRow != _Rows.end() && std::to_address(_CursorRow) == std::addressof(row)) {
      auto FindValidRow = [&](auto it) -> decltype(_CursorRow) {
        for (; it != _Rows.begin(); --it) {
          if (it->Node.has_value()) {
            return it;
          }
        }
        if (_Rows.begin()->Node.has_value()) {
          return _Rows.begin();
        } else {
          return _Rows.end();
        }
      };

      auto parent = node.GetParent();
      if (parent.has_value()) {
        auto& newSelected = parent.value().get();
        auto it = std::ranges::find_if(_Rows, [&](auto& r) {
          return r.Node.has_value() && std::addressof(r.Node.value().get()) == std::addressof(newSelected);
        });
        _CursorRow = (it != _Rows.end()) ? it : FindValidRow(_CursorRow);
      } else {
        _CursorRow = FindValidRow(_CursorRow);
      }
    }
    assert(_CursorRow == _Rows.end() || _CursorRow->Node.has_value());
  }

  void UpdateData(this Impl& self, NodeType& selected, std::vector<std::reference_wrapper<NodeType>> nodes) {
    self._Interface.Debug("UpdateData", DebugWindow::DebugLevel::Debug);
    assert(std::ranges::contains(nodes, selected));

    std::list<RowType> oldRows;
    oldRows.swap(self._Rows);
    self._CursorRow = self._Rows.end();

    std::map<NodeType*, typename std::list<RowType>::iterator> existingRows;
    for (auto it = oldRows.begin(); it != oldRows.end(); ++it) {
      if (it->Node.has_value()) {
        existingRows.emplace(std::addressof(it->Node.value().get()), it);
      }
    }

    for (auto& node : nodes | std::views::transform([](auto& ref) -> NodeType& { return ref; })) {
      auto it = existingRows.find(std::addressof(node));
      if (it != existingRows.end()) {
        self._Rows.splice(self._Rows.end(), oldRows, it->second);
        existingRows.erase(it);
        self._Rows.back().UpdateMetrics(self);
      } else {
        self._Rows.emplace_back(self, node);
        auto& row = self._Rows.back();
        if constexpr (requires { self.DiscoverColumns(row); }) {
          self.DiscoverColumns(row);
        }
        for (auto& column : self.GetAllColumns()) {
          column.RegisterRow(row);
        }
      }
      if (node == selected) {
        self._CursorRow = std::prev(self._Rows.end());
      }
    }

    if (self._CursorRow == self._Rows.end() && !self._Rows.empty()) {
      self._CursorRow = self._Rows.begin();
    }
  }

  void RemoveData() {
    _Rows.clear();
    _CursorRow = _Rows.end();
  }

  void MoveCursorAndDraw(this Impl& self, ssize_t offset) {
    assert(self._CursorRow != self._Rows.end());
    assert(self._CursorRow->Node.has_value());
    auto& nextSelected = self._Manager.MoveCursor(self._CursorRow->Node.value().get(), offset);
    auto cursor = std::distance(self._Rows.begin(), self._CursorRow);
    auto nodes = self._Manager.GetAround(nextSelected, cursor, self._TableCapacity);
    self.UpdateData(nextSelected, nodes);
  }

  void Update(this Impl& self) {
    auto size = self._TableCapacity;
    if (size > 0) {
      self.PreUpdate();
      if (self._CursorRow != self._Rows.end()) {
        auto& selected = self._CursorRow->Node.value().get();
        auto cursorPosition = std::min(size - 1, std::distance(self._Rows.begin(), self._CursorRow));
        auto nodes = self._Manager.GetAround(selected, cursorPosition, size);
        self.UpdateData(selected, nodes);
      } else {
        auto nodes = self._Manager.GetTopK(size);
        if (!nodes.empty()) {
          auto& selected = nodes[0].get();
          self.UpdateData(selected, nodes);
        } else {
          self.RemoveData();
        }
      }
    }
  }

protected:
  void OnTableSizeChange(this Impl& self, ::ftxui::Box box) {
    ssize_t newCapacity = box.y_max - box.y_min + 1 - kHeaderHeight;
    if (newCapacity > 0 && newCapacity != self._TableCapacity) {
      self._TableCapacity = newCapacity;
      self.Update();
      self._Interface.ScheduleRefresh();
    }
  }

  OmniMonInterface& _Interface;
  static constexpr ssize_t kHeaderHeight = 1;
  ssize_t _TableCapacity = 0;

  std::list<RowType> _Rows;
  typename std::list<RowType>::iterator _CursorRow = _Rows.end();
  std::shared_ptr<backend::metrics::SubscriberBase> _TickUpdater;
};

} // namespace frontend::ftxui
