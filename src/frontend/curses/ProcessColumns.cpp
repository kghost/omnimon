#include "ProcessColumns.hpp"

#include <format>
#include <unicode/unistr.h>

#include "../../backend/metrics/Arithmetic.hpp"
#include "../../backend/metrics/Counter.hpp"
#include "../../backend/system/SysInfo.hpp"
#include "../../utils/Clock.hpp"
#include "../../utils/Formatter.hpp"
#include "Options.hpp"
#include "Process.hpp"

namespace frontend::curses {

ProcessHeaderCell::ProcessHeaderCell(TextView::Align align, std::string text)
    : _View(std::make_shared<TextView>(align, text)) {
  _View->SetAttr(A_UNDERLINE);
}

std::shared_ptr<View> ProcessHeaderCell::CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) {
  return _View;
}

ProcessDataCell::ProcessDataCell(std::shared_ptr<Column> column,
                                 std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row, TextView::Align align)
    : _Column(column), _Row(row), _View(std::make_shared<TextView>(align)) {}

std::shared_ptr<View> ProcessDataCell::CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) {
  OnRowBindingChanged();
  return _View;
}

std::shared_ptr<TableCellBinding> ProcessColumnCursor::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("≡"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnCursor::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                          std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataAbstractCell {
  public:
    explicit CellBinding(ProcessTree& tree, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : _Tree(tree), _Row(row), _View(std::make_shared<TextView>(TextView::Align::Left)) {}
    std::shared_ptr<View> CreateView(std::shared_ptr<Row> row, std::shared_ptr<Column> column) override {
      _CursorUpdater = _Tree.GetTable()->OnCursorRowUpdate([this](auto data) {
        auto selectedRow = data->GetValue();
        if (selectedRow && selectedRow->GetBinding() == _Row) {
          _View->SetText("⮚");
        } else {
          _View->SetText("");
        }
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

std::shared_ptr<TableCellBinding> ProcessColumnPid::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("PID"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnPid::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                       std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
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

std::shared_ptr<TableCellBinding> ProcessColumnState::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("S"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnState::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _StateUpdater = backend::metrics::MakeSubscriber(process->GetState(), [this](auto metric) {
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

std::shared_ptr<TableCellBinding> ProcessColumnUser::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("User"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnUser::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                        std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Left) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        auto text = process->GetUser();
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

std::shared_ptr<TableCellBinding> ProcessColumnCpu::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("%CPU"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnCpu::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                       std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _CpuUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::Ratio>(
                std::make_shared<backend::metrics::CounterSlice>(
                    std::make_shared<backend::metrics::Plus>(process->GetUserTime(), process->GetSystemTime()),
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

std::shared_ptr<TableCellBinding> ProcessColumnMem::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("%MEM"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnMem::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                       std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _MemUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::Ratio>(process->GetMem(),
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

std::shared_ptr<TableCellBinding> ProcessColumnTime::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("Time+"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnTime::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                        std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _TimeUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::Plus>(process->GetUserTime(), process->GetSystemTime()),
            [this](auto metric) {
              auto text = std::format(
                  "{:%H:%M:%S}", std::chrono::floor<std::chrono::seconds>(utils::JiffyToDuration(metric->GetValue())));
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

std::shared_ptr<TableCellBinding> ProcessColumnDiskRead::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("DiskR"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnDiskRead::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                            std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _DiskReadUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::CounterSlice>(process->GetDiskReadBytes(),
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

std::shared_ptr<TableCellBinding> ProcessColumnDiskWrite::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("DiskW"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnDiskWrite::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                             std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _DiskWriteUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::CounterSlice>(process->GetDiskWriteBytes(),
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

std::shared_ptr<TableCellBinding> ProcessColumnDiskAccumulated::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("Disk+"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnDiskAccumulated::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                   std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _DiskAccumulatedUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::Plus>(process->GetDiskReadBytes(), process->GetDiskWriteBytes()),
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

std::shared_ptr<TableCellBinding> ProcessColumnIO::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("I/O"));
}

std::shared_ptr<TableCellBinding> ProcessColumnIO::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                                        std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _IOUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::CounterSlice>(
                std::make_shared<backend::metrics::Plus>(process->GetReadBytes(), process->GetWriteBytes()),
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

std::shared_ptr<TableCellBinding> ProcessColumnIOAccumulated::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Right, std::string("I/O+"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnIOAccumulated::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                                 std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
        : ProcessDataCell(column, row, TextView::Align::Right) {}
    void OnRowBindingChanged() override {
      auto process = _Row->GetProcess();
      if (process) {
        _IOAccumulatedUpdater = backend::metrics::MakeSubscriber(
            std::make_shared<backend::metrics::Plus>(process->GetReadBytes(), process->GetWriteBytes()),
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

std::shared_ptr<TableCellBinding> ProcessColumnStart::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("Start"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnStart::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                         std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
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

std::shared_ptr<TableCellBinding> ProcessColumnCommand::Header() const {
  return std::make_shared<ProcessHeaderCell>(TextView::Align::Left, std::string("Command"));
}

std::shared_ptr<TableCellBinding>
ProcessColumnCommand::Data(ProcessTree& tree, std::shared_ptr<Column> column,
                           std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row) {
  class CellBinding : public ProcessDataCell {
  public:
    explicit CellBinding(std::shared_ptr<Column> column, std::shared_ptr<ProcessTree::ProcessTreeTableDataBinding> row)
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

} // namespace frontend::curses
