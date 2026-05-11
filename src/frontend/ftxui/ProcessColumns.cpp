#include "ProcessColumns.hpp"

#include <format>
#include <ftxui/dom/elements.hpp>
#include <unicode/unistr.h>

#include "../../backend/metrics/Arithmetic.hpp"
#include "../../backend/metrics/Counter.hpp"
#include "../../backend/system/SysInfo.hpp"
#include "../../utils/Clock.hpp"
#include "../../utils/Formatter.hpp"
#include "Options.hpp"

namespace frontend::ftxui {

// ============================================================================
// ColumnCursor
// ============================================================================

std::string ColumnCursor::GetHeaderText() const { return "≡"; }

void ColumnCursor::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnCursor::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return isRowSelected ? "⮚" : " ";
}

void ColumnCursor::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
}

// ============================================================================
// ColumnPid
// ============================================================================

std::string ColumnPid::GetHeaderText() const { return "PID"; }

void ColumnPid::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnPid::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return std::format("{}", row.ProcessPtr->GetPid());
}

void ColumnPid::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::align_right);
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::GREATER_THAN, 3));
}

// ============================================================================
// ColumnState
// ============================================================================

std::string ColumnState::GetHeaderText() const { return "S"; }

void ColumnState::RegisterRow(ProcessTree::Row& row) const {
  row.StateUpdater = backend::metrics::MakeSubscriber(row.ProcessPtr->GetState(), [&row](auto metric) {
    row.StateDisplay = std::format("{:c}", static_cast<char>(metric->GetValue()));
  });
}

std::string ColumnState::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.StateDisplay;
}

void ColumnState::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 1));
}

// ============================================================================
// ColumnUser
// ============================================================================

std::string ColumnUser::GetHeaderText() const { return "User"; }

void ColumnUser::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnUser::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.ProcessPtr->GetUser();
}

void ColumnUser::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::GREATER_THAN, 5));
}

// ============================================================================
// ColumnCpu
// ============================================================================

std::string ColumnCpu::GetHeaderText() const { return "%CPU"; }

void ColumnCpu::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.CpuUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Ratio>(
          std::make_shared<backend::metrics::CounterSlice>(
              std::make_shared<backend::metrics::Plus>(process->GetUserTime(), process->GetSystemTime()),
              Config::GetInstance().RefreshInterval),
          backend::system::SysInfo::GetInstance()->GetSystemJiffies()),
      [&row](auto metric) { row.CpuDisplay = std::format("{:.1f}", metric->GetValue() / 100.0f); });
}

std::string ColumnCpu::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.CpuDisplay;
}

void ColumnCpu::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 4));
}

// ============================================================================
// ColumnMem
// ============================================================================

std::string ColumnMem::GetHeaderText() const { return "%MEM"; }

void ColumnMem::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.MemUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Ratio>(process->GetMem(),
                                                backend::system::SysInfo::GetInstance()->GetTotalMem()),
      [&row](auto metric) { row.MemDisplay = std::format("{:.1f}", metric->GetValue() / 100.0f); });
}

std::string ColumnMem::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.MemDisplay;
}

void ColumnMem::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 4));
}

// ============================================================================
// ColumnTime
// ============================================================================

std::string ColumnTime::GetHeaderText() const { return "Time+"; }

void ColumnTime::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.TimeUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Plus>(process->GetUserTime(), process->GetSystemTime()), [&row](auto metric) {
        auto text = std::format("{:%H:%M:%S}",
                                std::chrono::floor<std::chrono::seconds>(utils::JiffyToDuration(metric->GetValue())));
        row.TimeDisplay = text;
      });
}

std::string ColumnTime::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.TimeDisplay;
}

void ColumnTime::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 8));
}

// ============================================================================
// ColumnDiskRead
// ============================================================================

std::string ColumnDiskRead::GetHeaderText() const { return "DiskR"; }

void ColumnDiskRead::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.DiskReadUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::CounterSlice>(process->GetDiskReadBytes(),
                                                       Config::GetInstance().RefreshInterval),
      [&row](auto metric) { row.DiskReadDisplay = utils::DiskSizeToString(metric->GetValue(), 5); });
}

std::string ColumnDiskRead::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.DiskReadDisplay;
}

void ColumnDiskRead::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 5));
}

// ============================================================================
// ColumnDiskWrite
// ============================================================================

std::string ColumnDiskWrite::GetHeaderText() const { return "DiskW"; }

void ColumnDiskWrite::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.DiskWriteUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::CounterSlice>(process->GetDiskWriteBytes(),
                                                       Config::GetInstance().RefreshInterval),
      [&row](auto metric) { row.DiskWriteDisplay = utils::DiskSizeToString(metric->GetValue(), 5); });
}

std::string ColumnDiskWrite::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.DiskWriteDisplay;
}

void ColumnDiskWrite::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 5));
}

// ============================================================================
// ColumnDiskAccumulated
// ============================================================================

std::string ColumnDiskAccumulated::GetHeaderText() const { return "Disk+"; }

void ColumnDiskAccumulated::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.DiskAccumulatedUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Plus>(process->GetDiskReadBytes(), process->GetDiskWriteBytes()),
      [&row](auto metric) { row.DiskAccumulatedDisplay = utils::DiskSizeToString(metric->GetValue(), 5); });
}

std::string ColumnDiskAccumulated::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.DiskAccumulatedDisplay;
}

void ColumnDiskAccumulated::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 5));
}

// ============================================================================
// ColumnIO
// ============================================================================

std::string ColumnIO::GetHeaderText() const { return "I/O"; }

void ColumnIO::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.IOUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::CounterSlice>(
          std::make_shared<backend::metrics::Plus>(process->GetReadBytes(), process->GetWriteBytes()),
          Config::GetInstance().RefreshInterval),
      [&row](auto metric) { row.IODisplay = utils::DiskSizeToString(metric->GetValue(), 5); });
}

std::string ColumnIO::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.IODisplay;
}

void ColumnIO::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 5));
}

// ============================================================================
// ColumnIOAccumulated
// ============================================================================

std::string ColumnIOAccumulated::GetHeaderText() const { return "I/O+"; }

void ColumnIOAccumulated::RegisterRow(ProcessTree::Row& row) const {
  auto process = row.ProcessPtr;
  row.IOAccumulatedUpdater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Plus>(process->GetReadBytes(), process->GetWriteBytes()),
      [&row](auto metric) { row.IOAccumulatedDisplay = utils::DiskSizeToString(metric->GetValue(), 5); });
}

std::string ColumnIOAccumulated::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return row.IOAccumulatedDisplay;
}

void ColumnIOAccumulated::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 5));
}

// ============================================================================
// ColumnStart
// ============================================================================

std::string ColumnStart::GetHeaderText() const { return "Start"; }

void ColumnStart::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnStart::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  auto sys = utils::FromSteadyClock(row.ProcessPtr->GetStartTime());
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

void ColumnStart::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::size(::ftxui::WidthOrHeight::WIDTH, ::ftxui::Constraint::EQUAL, 5));
}

// ============================================================================
// ColumnCommand
// ============================================================================

std::string ColumnCommand::GetHeaderText() const { return "Command"; }

void ColumnCommand::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnCommand::GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const {
  return TreeString(row.ProcessPtr) + FormatCommand(row.ProcessPtr->GetCommandLine());
}

void ColumnCommand::Decorate(::ftxui::TableSelection selection) const {}

std::string ColumnCommand::TreeString(std::shared_ptr<backend::process::Process> process) {
  std::string result;
  auto list = backend::process::Process::GetTreePosition(process);
  for (auto it = list.begin(); it != list.end(); ++it) {
    auto it2 = it;
    if (++it2 != list.end()) {
      switch (*it) {
      case backend::process::Process::ChildPosition::NotLast:
        result += "│ ";
        break;
      case backend::process::Process::ChildPosition::Last:
        result += "  ";
        break;
      }
    } else {
      switch (*it) {
      case backend::process::Process::ChildPosition::NotLast:
        result += "├─";
        break;
      case backend::process::Process::ChildPosition::Last:
        result += "└─";
        break;
      }
    }
  }
  return result;
}

std::string ColumnCommand::FormatCommand(const std::string& command) {
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

} // namespace frontend::ftxui
