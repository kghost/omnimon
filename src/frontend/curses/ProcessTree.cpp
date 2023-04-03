#include "ProcessTree.hpp"

#include <format>
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

namespace frontend::curses {

class ProcessCell {
public:
  virtual ~ProcessCell() = default;
  virtual std::shared_ptr<View> GetView() const = 0;
  virtual void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                           std::shared_ptr<Process> process) = 0;
};

class ProcessColumn : public Column {
public:
  explicit ProcessColumn(Table& table, Container::ArrangementType arrangement, DisplayLength width)
      : Column(table, arrangement, width) {}
  ~ProcessColumn() override = default;

  DisplayLength GetMarginAfter() const override { return 1; }
  virtual std::shared_ptr<View> HeaderView() const = 0;
  virtual std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) = 0;
};

class ProcessColumnCursor : public ProcessColumn {
public:
  explicit ProcessColumnCursor(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 1) {}
  ~ProcessColumnCursor() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Left);
    cell->SetText("☰");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Left)) {}
    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
      _CursorUpdater = backend::metrics::MakeSubscriber(tree.GetCursor(), [this, &row](auto metric) {
        _View->SetText(metric->GetValue() == row.GetIndex() ? "⮚" : "");
      });
    }

  private:
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _CursorUpdater;
  };
};

class ProcessColumnPid : public ProcessColumn {
public:
  explicit ProcessColumnPid(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 3) {}
  ~ProcessColumnPid() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("PID");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(column); }

  class Cell : public ProcessCell {
  public:
    Cell(ProcessColumn& column) : _Column(column), _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
      if (process) {
        auto text = std::format("{}", process->GetPid());
        if (_Column.GetSize() < text.size()) {
          _Column.SetSize(text.size());
        }
        _View->SetText(text);
      } else {
        _View->SetText("");
      }
    }

  private:
    ProcessColumn& _Column;
    std::shared_ptr<TextView> _View;
  };
};

class ProcessColumnState : public ProcessColumn {
public:
  explicit ProcessColumnState(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 1) {}
  ~ProcessColumnState() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("S");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _StateUpdater;
  };
};

class ProcessColumnCpu : public ProcessColumn {
public:
  explicit ProcessColumnCpu(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 4) {}
  ~ProcessColumnCpu() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("%CPU");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _CpuUpdater;
  };
};

class ProcessColumnMem : public ProcessColumn {
public:
  explicit ProcessColumnMem(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 4) {}
  ~ProcessColumnMem() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("%MEM");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _MemUpdater;
  };
};

class ProcessColumnTime : public ProcessColumn {
public:
  explicit ProcessColumnTime(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 2) {}
  ~ProcessColumnTime() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("Time+");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(column); }

  class Cell : public ProcessCell {
  public:
    Cell(ProcessColumn& column) : _Column(column), _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
      if (process) {
        _TimeUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::Plus>(Process::GetUserTime(process), Process::GetSystemTime(process)),
            [this](auto metric) {
              auto text = std::format(
                  "{:%H:%M:%S}", std::chrono::floor<std::chrono::seconds>(utils::JiffyToDuration(metric->GetValue())));
              if (_Column.GetSize() < text.size()) {
                _Column.SetSize(text.size());
              }
              _View->SetText(text);
            });
      } else {
        _TimeUpdater.reset();
        _View->SetText("");
      }
    }

  private:
    ProcessColumn& _Column;
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _TimeUpdater;
  };
};

class ProcessColumnDiskRead : public ProcessColumn {
public:
  explicit ProcessColumnDiskRead(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 5) {}
  ~ProcessColumnDiskRead() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("DiskR");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _DiskReadUpdater;
  };
};

class ProcessColumnDiskWrite : public ProcessColumn {
public:
  explicit ProcessColumnDiskWrite(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 5) {}
  ~ProcessColumnDiskWrite() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("DiskW");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _DiskWriteUpdater;
  };
};

class ProcessColumnDiskAccumulated : public ProcessColumn {
public:
  explicit ProcessColumnDiskAccumulated(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 5) {}
  ~ProcessColumnDiskAccumulated() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("Disk+");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _DiskAccumulatedUpdater;
  };
};

class ProcessColumnIO : public ProcessColumn {
public:
  explicit ProcessColumnIO(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 5) {}
  ~ProcessColumnIO() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("I/O");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _IOUpdater;
  };
};

class ProcessColumnIOAccumulated : public ProcessColumn {
public:
  explicit ProcessColumnIOAccumulated(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 5) {}
  ~ProcessColumnIOAccumulated() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Right);
    cell->SetText("I/O+");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    ~Cell() override = default;

    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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
    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _IOAccumulatedUpdater;
  };
};

class ProcessColumnStart : public ProcessColumn {
public:
  explicit ProcessColumnStart(Table& table) : ProcessColumn(table, Container::ArrangementType::Forward, 5) {}
  ~ProcessColumnStart() override = default;

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Left);
    cell->SetText("Start");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Right)) {}
    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
      if (process) {
        _View->SetText(std::format("{}", StartTime(process->GetStartTime())));
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

    std::shared_ptr<TextView> _View;
    std::shared_ptr<backend::metrics::SubscriberBase> _StartUpdater;
  };
};

class ProcessColumnCommand : public ProcessColumn {
public:
  explicit ProcessColumnCommand(Table& table) : ProcessColumn(table, Container::ArrangementType::FillRest, 1) {}
  ~ProcessColumnCommand() override = default;
  DisplayLength GetMarginAfter() const override { return 0; }

  std::shared_ptr<View> HeaderView() const override {
    auto cell = std::make_shared<TextView>(TextView::Align::Left);
    cell->SetText("Command");
    return cell;
  }

  std::shared_ptr<ProcessCell> CreateCell(ProcessColumn& column) override { return std::make_shared<Cell>(); }

  class Cell : public ProcessCell {
  public:
    Cell() : _View(std::make_shared<TextView>(TextView::Align::Left)) {}
    std::shared_ptr<View> GetView() const override { return _View; }
    void BindProcess(ProcessTree& tree, ProcessTree::ProcessTreeTableRowBinding& row,
                     std::shared_ptr<Process> process) override {
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

    std::shared_ptr<TextView> _View;
  };
};

ProcessTree::ProcessTree()
    : _TableInputHandler(std::make_shared<TableInputHandler>(*this)),
      _Table(
          _TableInputHandler,
          ColumnBuilder<ProcessColumnCursor, ProcessColumnPid, ProcessColumnState, ProcessColumnMem, ProcessColumnTime,
                        ProcessColumnDiskRead, ProcessColumnDiskWrite, ProcessColumnDiskAccumulated, ProcessColumnIO,
                        ProcessColumnIOAccumulated, ProcessColumnStart, ProcessColumnCommand>()),
      _Cursor(std::make_shared<backend::metrics::SimpleGauge>()) {
  _Table.AppendRow(std::make_shared<ProcessTreeTableHeaderBinding>());
}

std::shared_ptr<View> ProcessTree::GetView() const { return _Table.GetTableContainer(); }

void ProcessTree::Update() {
  std::vector<std::shared_ptr<frontend::curses::Process>> ps;
  if (!_Rows.empty()) {
    DisplayLength index = _Cursor->GetValue();
    auto p = _Rows[index];
    ps = _ProcessCollection.GetAround(p->GetProcess(), index, GetHeight(), true);
  } else {
    _ProcessCollection.UpdateList();
    // ps.push_back(_ProcessCollection.GetProcess(882));
    ps = _ProcessCollection.GetTopK(GetHeight());
  }
  UpdateTable(ps);
}

DisplayLength ProcessTree::GetHeight() const { return _Table.GetTableContainer()->GetLayout().Height - 1; }

void ProcessTree::UpdateTable(const std::vector<std::shared_ptr<frontend::curses::Process>>& ps) {
  int max = std::max(ps.size(), _Rows.size());

  for (int i = 0; i < max; ++i) {
    if (i < ps.size() && i < _Rows.size()) {
      _Rows[i]->UpdateProcess(ps[i]);
    } else if (i >= ps.size() && i < _Rows.size()) {
      _Rows[i]->UpdateProcess(nullptr);
    } else if (i < ps.size() && i >= _Rows.size()) {
      auto row = std::make_shared<ProcessTreeTableRowBinding>(*this, i, ps[i]);
      _Rows.push_back(row);
      _Table.AppendRow(row);
      ps[i]->Update();
    }
  }
}

bool ProcessTree::OnKey(TermKeyCode key) {
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
    if (_Cursor->GetValue() < _Rows.size() - 1) {
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
  default:
    return _Rows[_Cursor->GetValue()]->OnKey(key);
  }
}

void ProcessTree::MoveCursorAndDraw(DisplayLength offset) {
  auto p = _ProcessCollection.MoveCursor(_Rows[_Cursor->GetValue()]->GetProcess(), offset);
  DisplayLength cursor = _Cursor->GetValue();
  auto ps = _ProcessCollection.GetAround(p, cursor, GetHeight(), false);
  _Cursor->Update(cursor);
  UpdateTable(ps);
}


bool ProcessTree::ProcessTreeTableHeaderBinding::OnKey(TermKeyCode key) { return false; }

std::shared_ptr<View> ProcessTree::ProcessTreeTableHeaderBinding::OnNewCell(Table& table, Row& row, Column& column) {
  return dynamic_cast<ProcessColumn&>(column).HeaderView();
}

ProcessTree::ProcessTreeTableRowBinding::ProcessTreeTableRowBinding(ProcessTree& tree, size_t index,
                                                                    std::shared_ptr<Process> process)
    : _Tree(tree), _Index(index), _Process(process) {}

bool ProcessTree::ProcessTreeTableRowBinding::OnKey(TermKeyCode key) { return false; }

std::shared_ptr<View> ProcessTree::ProcessTreeTableRowBinding::OnNewCell(Table& table, Row& row, Column& column) {
  auto& c = dynamic_cast<ProcessColumn&>(column);
  std::shared_ptr<ProcessCell> cell = c.CreateCell(c);
  _Cells.push_back(cell);
  cell->BindProcess(_Tree, *this, _Process);
  return cell->GetView();
}

void ProcessTree::ProcessTreeTableRowBinding::UpdateProcess(std::shared_ptr<Process> process) {
  if (_Process != process) {
    for (auto cell : _Cells) {
      cell->BindProcess(_Tree, *this, process);
    }
    _Process = process;
    _Process->Update();
  }
}

bool ProcessTree::TableInputHandler::OnKey(TermKeyCode key) { return _ProcessTree.OnKey(key); }

} // namespace frontend::curses
