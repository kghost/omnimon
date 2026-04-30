#include "ProcessTree.hpp"

#include <format>
#include <memory>
#include <unicode/unistr.h>
#include <vector>

#include "../../backend/metrics/Arithmetic.hpp"
#include "../../backend/metrics/Counter.hpp"
#include "../../backend/system/SysInfo.hpp"
#include "../../utils/Clock.hpp"
#include "../../utils/Formatter.hpp"
#include "OmniMon.hpp"
#include "Options.hpp"
#include "Process.hpp"
#include "layouts/Container.hpp"
#include "layouts/TextView.hpp"

namespace frontend::curses {

class ProcessHeaderCell : public TableCellBinding {
public:
  ProcessHeaderCell(TextView::Align align, std::string text) : _View(std::make_shared<TextView>(align, text)) {}
  std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) override { return _View; }

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
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row, TextView::Align align)
      : _Column(column), _Row(row), _View(std::make_shared<TextView>(align)) {}

  std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) override {
    OnRowBindingChanged();
    return _View;
  }

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
  explicit ProcessColumnCursor() = default;
  ~ProcessColumnCursor() override = default;

  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("☰"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataAbstractCell {
    public:
      explicit CellBinding(ProcessTree& tree, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : _Tree(tree), _Row(row), _View(std::make_shared<TextView>(TextView::Align::Left)) {}
      std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) override {
        _CursorUpdater = backend::metrics::MakeSubscriber(_Tree.GetCursor(), [this](auto metric) {
          _View->SetText(metric->GetValue() == _Row->GetIndex() ? "⮚" : "");
        });
        return _View;
      }

      void OnRowBindingChanged() override {}

    private:
      ProcessTree& _Tree;
      std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> _Row;
      std::shared_ptr<backend::metrics::SubscriberBase> _CursorUpdater;
      std::shared_ptr<TextView> _View;
    };

    return std::make_shared<CellBinding>(tree, row);
  }
};

class ProcessColumnPid : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("PID"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          auto text = std::format("{}", process->GetPid());
          if (_Column->GetSize() < text.size()) {
            _Column->SetSize(text.size());
          }
          _View->SetText(text);
        } else {
          _View->SetText("");
        }
      }
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnState : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("S"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _StateUpdater = backend::metrics::MakeSubscriber(Process::GetState(process), [this](auto metric) {
            _View->SetText(std::format("{:c}", static_cast<char>(metric->GetValue())));
          });
        } else {
          _StateUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _StateUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnCpu : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("%CPU"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _CpuUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::Ratio>(
                  std::make_shared<backend::metrics::CounterSlice>(
                      std::make_shared<backend::metrics::Plus>(Process::GetUserTime(process),
                                                               Process::GetSystemTime(process)),
                      Config::GetInstance().RefreshInterval),
                  backend::system::SysInfo::GetInstance()->GetSystemJiffies()),
              [this](auto metric) { _View->SetText(std::format("{:.{}f}", metric->GetValue() / 100.0f, 1)); });
        } else {
          _CpuUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _CpuUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnMem : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("%MEM"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _MemUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::Ratio>(Process::GetMem(process),
                                                        backend::system::SysInfo::GetInstance()->GetTotalMem()),
              [this](auto metric) { _View->SetText(std::format("{:.{}f}", metric->GetValue() / 100.0f, 1)); });
        } else {
          _MemUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _MemUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnTime : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("Time+"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _TimeUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::Plus>(Process::GetUserTime(process), Process::GetSystemTime(process)),
              [this](auto metric) {
                auto text =
                    std::format("{:%H:%M:%S}",
                                std::chrono::floor<std::chrono::seconds>(utils::JiffyToDuration(metric->GetValue())));
                if (_Column->GetSize() < text.size()) {
                  _Column->SetSize(text.size());
                }
                _View->SetText(text);
              });
        } else {
          _TimeUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _TimeUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnDiskRead : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("DiskR"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _DiskReadUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::CounterSlice>(Process::GetDiskReadBytes(process),
                                                               Config::GetInstance().RefreshInterval),
              [this](auto metric) { _View->SetText(utils::DiskSizeToString(metric->GetValue(), 5)); });
        } else {
          _DiskReadUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _DiskReadUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnDiskWrite : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("DiskW"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _DiskWriteUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::CounterSlice>(Process::GetDiskWriteBytes(process),
                                                               Config::GetInstance().RefreshInterval),
              [this](auto metric) { _View->SetText(utils::DiskSizeToString(metric->GetValue(), 5)); });
        } else {
          _DiskWriteUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _DiskWriteUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnDiskAccumulated : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("Disk+"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _DiskAccumulatedUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::Plus>(Process::GetDiskReadBytes(process),
                                                       Process::GetDiskWriteBytes(process)),
              [this](auto metric) { _View->SetText(utils::DiskSizeToString(metric->GetValue(), 5)); });
        } else {
          _DiskAccumulatedUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _DiskAccumulatedUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnIO : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("I/O"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _IOUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::CounterSlice>(
                  std::make_shared<backend::metrics::Plus>(Process::GetReadBytes(process),
                                                           Process::GetWriteBytes(process)),
                  Config::GetInstance().RefreshInterval),
              [this](auto metric) { _View->SetText(utils::DiskSizeToString(metric->GetValue(), 5)); });
        } else {
          _IOUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _IOUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnIOAccumulated : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("I/O+"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Right) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _IOAccumulatedUpdater = backend::metrics::MakeSubscriber(
              std::make_shared<backend::metrics::Plus>(Process::GetReadBytes(process), Process::GetWriteBytes(process)),
              [this](auto metric) { _View->SetText(utils::DiskSizeToString(metric->GetValue(), 5)); });
        } else {
          _IOAccumulatedUpdater.reset();
          _View->SetText("");
        }
      }

    private:
      std::shared_ptr<backend::metrics::SubscriberBase> _IOAccumulatedUpdater;
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnStart : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("Start"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Left) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _View->SetText(StartTime(process->GetStartTime()));
        } else {
          _View->SetText("");
        }
      }

    private:
      std::string StartTime(std::chrono::steady_clock::time_point start) {
        auto sys = utils::FromSteadyClock(start);
        auto diff = std::chrono::system_clock::now() - sys;
        auto local = std::chrono::current_zone()->to_local(sys);
        if (diff < std::chrono::days(1)) {
          return std::format("{:%H:%M}", local);
        } else if (diff < std::chrono::years(1)) {
          return std::format("{:%b%d}", local);
        } else {
          return std::format("{:%Y}", local);
        }
      }
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

class ProcessColumnCommand : public ProcessColumn {
public:
  std::shared_ptr<TableCellBinding> Header() const override {
    return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("Command"));
  }

  std::shared_ptr<TableCellBinding> Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) override {
    class CellBinding : public ProcessDataCell {
    public:
      explicit CellBinding(std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
          : ProcessDataCell(column, row, TextView::Align::Left) {}
      void OnRowBindingChanged() override {
        auto process = _Row->GetProcess();
        if (process) {
          _View->SetText(std::format("{}{}", TreeString(process), FormatCommand(process->GetCommandLine())));
        } else {
          _View->SetText("");
        }
      }

    private:
      static std::string TreeString(std::shared_ptr<Process> process) {
        std::string result;
        auto list = Process::GetTreePosition(process);
        for (auto it = list.begin(); it != list.end(); ++it) {
          auto it2 = it;
          if (++it2 != list.end()) {
            switch (*it) {
            case Process::ChildPosition::NotLast:
              result += "│ ";
              break;
            case Process::ChildPosition::Last:
              result += "  ";
              break;
            }
          } else {
            switch (*it) {
            case Process::ChildPosition::NotLast:
              result += "├─";
              break;
            case Process::ChildPosition::Last:
              result += "└─";
              break;
            }
          }
        }
        return result;
      }

      static std::string FormatCommand(const std::string& command) {
        static auto replacements = std::vector{
            std::tuple{icu::UnicodeString::fromUTF8(" "), icu::UnicodeString::fromUTF8("␣")},
            std::tuple{icu::UnicodeString::fromUTF8("\t"), icu::UnicodeString::fromUTF8("⭾")},
            std::tuple{icu::UnicodeString::fromUTF8("\r"), icu::UnicodeString::fromUTF8("␍")},
            std::tuple{icu::UnicodeString::fromUTF8("\n"), icu::UnicodeString::fromUTF8("␊")},
            std::tuple{icu::UnicodeString(0), icu::UnicodeString::fromUTF8(" ")},
        };

        icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(command);
        for (auto& [f, r] : replacements) {
          ustr.findAndReplace(f, r);
        }

        std::string result;
        ustr.toUTF8String(result);
        return result;
      }
    };
    return std::make_shared<CellBinding>(column, row);
  }
};

ProcessTree::ProcessTree()
    : _Table(std::make_shared<Table>(*this, *this)), _Cursor(std::make_shared<backend::metrics::SimpleGauge>()) {
  constexpr const auto Forward = Container::ChildArrangement::ArrangementType::Forward;
  constexpr const auto FillRest = Container::ChildArrangement::ArrangementType::FillRest;
  _Table->AppendColumn(std::make_shared<ProcessColumnCursor>(), Forward, 1, 0, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnPid>(), Forward, 3, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnState>(), Forward, 1, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnCpu>(), Forward, 4, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnMem>(), Forward, 4, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnTime>(), Forward, 2, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnDiskRead>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnDiskWrite>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnDiskAccumulated>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnIO>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnIOAccumulated>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnStart>(), Forward, 5, 1, 0);
  _Table->AppendColumn(std::make_shared<ProcessColumnCommand>(), FillRest, 1, 1, 0);

  _Table->AppendRow(std::make_shared<ProcessTreeTableHeaderBinding>(), _TableHeaderHeight);
}

std::shared_ptr<TableCellBinding> ProcessTree::NewCell(Table& table, std::shared_ptr<Row> row,
                                                       std::shared_ptr<Column> column) {
  auto binding = std::dynamic_pointer_cast<ProcessTreeTableRowBinding>(row->GetBinding());
  return binding->CreateCell(*this, column, row->GetBinding());
}

void ProcessTree::Update() {
  std::vector<std::shared_ptr<frontend::curses::Process>> ps;
  auto rows = GetDataRows();
  if (rows.size() > 0) {
    DisplayLength index = _Cursor->GetValue();
    auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(rows[index]->GetBinding());
    ps = _ProcessCollection.GetAround(binding->GetProcess(), index, GetHeight(), true);
  } else {
    _ProcessCollection.UpdateList();
    ps = _ProcessCollection.GetTopK(GetHeight());
  }
  UpdateTable(ps, rows);
}

DisplayLength ProcessTree::GetHeight() const {
  DisplayLength height = _Table->GetLayout().Height;
  if (height < _TableHeaderHeight) {
    return 0;
  } else {
    return height - _TableHeaderHeight;
  }
}

void ProcessTree::UpdateTable(const std::vector<std::shared_ptr<frontend::curses::Process>>& ps,
                              std::span<std::shared_ptr<Row>> rows) {
  int max = std::max(ps.size(), rows.size());

  for (int i = 0; i < max; ++i) {
    if (i < ps.size() && i < rows.size()) {
      auto& row = rows[i];
      auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(row->GetBinding());
      binding->UpdateProcess(row, ps[i]);
    } else if (i >= ps.size() && i < rows.size()) {
      // Remove the row
      auto& row = rows[i];
      auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(row->GetBinding());
      binding->UpdateProcess(row, nullptr);
    } else if (i < ps.size() && i >= rows.size()) {
      // Add new row
      auto binding = std::make_shared<ProcessTreeTableDataBinding>(i, ps[i]);
      auto row = _Table->AppendRow(binding, 1);
      ps[i]->Update();
    }
  }
}

bool ProcessTree::OnKey(TermKeyCode key) {
  auto rows = GetDataRows();

  // Move cursor of selected row
  switch (key) {
  case KEY_UP:
    if (_Cursor->GetValue() > 0) {
      _Cursor->Update(_Cursor->GetValue() - 1);
    } else {
      MoveCursorAndDraw(-1);
    }
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  case KEY_DOWN:
    if (_Cursor->GetValue() < rows.size() - 1) {
      _Cursor->Update(_Cursor->GetValue() + 1);
    } else {
      MoveCursorAndDraw(1);
    }
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  case KEY_PPAGE: {
    MoveCursorAndDraw(-GetHeight());
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  }
  case KEY_NPAGE: {
    MoveCursorAndDraw(GetHeight());
    OmniMon::GetInstance().ScheduleDraw();
    return true;
  }
  }

  // Handle key for selected row
  if (rows.size() > _Cursor->GetValue() && rows[_Cursor->GetValue()]->GetBinding()->OnKey(key)) {
    return true;
  }

  return false;
}

void ProcessTree::MoveCursorAndDraw(DisplayLength offset) {
  DisplayLength cursor = _Cursor->GetValue();
  auto rows = GetDataRows();
  if (rows.size() <= 0) {
    return;
  }
  auto binding = std::dynamic_pointer_cast<ProcessTreeTableDataBinding>(rows[cursor]->GetBinding());
  auto p = _ProcessCollection.MoveCursor(binding->GetProcess(), offset);
  auto ps = _ProcessCollection.GetAround(p, cursor, GetHeight(), false);
  _Cursor->Update(cursor);
  UpdateTable(ps, rows);
}

std::shared_ptr<TableCellBinding>
ProcessTree::ProcessTreeTableHeaderBinding::CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                       std::shared_ptr<TableRowBinding> row) {
  return std::dynamic_pointer_cast<ProcessColumn>(column->GetBinding())->Header();
}

ProcessTree::ProcessTreeTableDataBinding::ProcessTreeTableDataBinding(DisplayLength index,
                                                                      std::shared_ptr<Process> process)
    : _Index(index), _Process(process) {}

std::shared_ptr<TableCellBinding>
ProcessTree::ProcessTreeTableDataBinding::CreateCell(ProcessTree& tree, std::shared_ptr<Column> column,
                                                     std::shared_ptr<TableRowBinding> row) {
  return std::dynamic_pointer_cast<ProcessColumn>(column->GetBinding())
      ->Data(tree, column, std::dynamic_pointer_cast<ProcessTree::ProcessTreeTableDataBinding>(row));
}

void ProcessTree::ProcessTreeTableDataBinding::UpdateProcess(std::shared_ptr<Row> row,
                                                             std::shared_ptr<Process> process) {
  if (_Process != process) {
    _Process = process;
    for (auto cell : row->GetCells()) {
      std::dynamic_pointer_cast<ProcessDataAbstractCell>(cell->GetBinding())->OnRowBindingChanged();
    }
    if (_Process) {
      _Process->Update();
    }
  }
}

} // namespace frontend::curses
