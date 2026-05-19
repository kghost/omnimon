#include "ProcessColumns.hpp"

#include <format>
#include <ftxui/dom/elements.hpp>
#include <unicode/unistr.h>

#include "../../backend/metrics/Arithmetic.hpp"
#include "../../backend/metrics/Counter.hpp"
#include "../../backend/process/ProcessMetrics.hpp"
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

std::string ColumnCursor::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
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

std::string ColumnPid::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return std::format("{}", row.Process.value().get().GetPid());
}

void ColumnPid::Decorate(::ftxui::TableSelection selection) const { selection.DecorateCells(::ftxui::align_right); }

// ============================================================================
// ColumnState
// ============================================================================

std::string ColumnState::GetHeaderText() const { return "S"; }

void ColumnState::RegisterRow(ProcessTree::Row& row) const {
  row.State.Updater = backend::metrics::MakeSubscriber(row.Process.value().get().GetState(), [&row](auto state) {
    row.State.Display = std::format("{:c}", static_cast<char>(state));
  });
}

std::string ColumnState::GetDataText(bool isRowSelected, ProcessTree::Row& row) const { return row.State.Display; }

void ColumnState::Decorate(::ftxui::TableSelection selection) const {}

// ============================================================================
// ColumnUser
// ============================================================================

std::string ColumnUser::GetHeaderText() const { return "User"; }

void ColumnUser::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnUser::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return row.Process.value().get().GetUser();
}

void ColumnUser::Decorate(::ftxui::TableSelection selection) const {}

// ============================================================================
// ColumnCpu
// ============================================================================

std::string ColumnCpu::GetHeaderText() const { return "%CPU"; }

void ColumnCpu::RegisterRow(ProcessTree::Row& row) const {
  auto& process = row.Process.value().get();
  row.Cpu.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Ratio>(
          std::make_shared<backend::metrics::CounterSlice>(
              std::make_shared<backend::metrics::Plus>(process.GetUserTime(), process.GetSystemTime()),
              Config::GetInstance().RefreshInterval),
          backend::system::SysInfo::GetInstance()->GetSystemJiffies()),
      [&row](auto cpu) { row.Cpu.Display = std::format("{:.1f}", cpu / 100.0f); });
}

std::string ColumnCpu::GetDataText(bool isRowSelected, ProcessTree::Row& row) const { return row.Cpu.Display; }

void ColumnCpu::Decorate(::ftxui::TableSelection selection) const { selection.DecorateCells(::ftxui::align_right); }

// ============================================================================
// ColumnMem
// ============================================================================

std::string ColumnMem::GetHeaderText() const { return "%MEM"; }

void ColumnMem::RegisterRow(ProcessTree::Row& row) const {
  row.Mem.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Ratio>(row.Process.value().get().GetMem(),
                                                backend::system::SysInfo::GetInstance()->GetTotalMem()),
      [&row](auto mem) { row.Mem.Display = std::format("{:.1f}", mem / 100.0f); });
}

std::string ColumnMem::GetDataText(bool isRowSelected, ProcessTree::Row& row) const { return row.Mem.Display; }

void ColumnMem::Decorate(::ftxui::TableSelection selection) const { selection.DecorateCells(::ftxui::align_right); }

// ============================================================================
// ColumnTime
// ============================================================================

std::string ColumnTime::GetHeaderText() const { return "Time+"; }

void ColumnTime::RegisterRow(ProcessTree::Row& row) const {
  auto& process = row.Process.value().get();
  row.Time.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Plus>(process.GetUserTime(), process.GetSystemTime()), [&row](auto time) {
        row.Time.Display =
            std::format("{:%H:%M:%S}", std::chrono::floor<std::chrono::seconds>(utils::JiffyToDuration(time)));
      });
}

std::string ColumnTime::GetDataText(bool isRowSelected, ProcessTree::Row& row) const { return row.Time.Display; }

void ColumnTime::Decorate(::ftxui::TableSelection selection) const {}

// ============================================================================
// ColumnDiskRead
// ============================================================================

std::string ColumnDiskRead::GetHeaderText() const { return "DiskR"; }

void ColumnDiskRead::RegisterRow(ProcessTree::Row& row) const {
  row.DiskRead.Updater =
      backend::metrics::MakeSubscriber(std::make_shared<backend::metrics::CounterSlice>(
                                           row.Metrics.GetDiskReadBytes(), Config::GetInstance().RefreshInterval),
                                       [&row](auto metric) { row.DiskRead.Display = utils::DiskSizeToString(metric); });
}

std::string ColumnDiskRead::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return row.DiskRead.Display;
}

void ColumnDiskRead::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::align_right);
}

// ============================================================================
// ColumnDiskWrite
// ============================================================================

std::string ColumnDiskWrite::GetHeaderText() const { return "DiskW"; }

void ColumnDiskWrite::RegisterRow(ProcessTree::Row& row) const {
  row.DiskWrite.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::CounterSlice>(row.Metrics.GetDiskWriteBytes(),
                                                       Config::GetInstance().RefreshInterval),
      [&row](auto metric) { row.DiskWrite.Display = utils::DiskSizeToString(metric); });
}

std::string ColumnDiskWrite::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return row.DiskWrite.Display;
}

void ColumnDiskWrite::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::align_right);
}

// ============================================================================
// ColumnDiskAccumulated
// ============================================================================

std::string ColumnDiskAccumulated::GetHeaderText() const { return "Disk+"; }

void ColumnDiskAccumulated::RegisterRow(ProcessTree::Row& row) const {
  auto& process = row.Metrics;
  row.DiskAccumulated.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Plus>(process.GetDiskReadBytes(), process.GetDiskWriteBytes()),
      [&row](auto metric) { row.DiskAccumulated.Display = utils::DiskSizeToString(metric); });
}

std::string ColumnDiskAccumulated::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return row.DiskAccumulated.Display;
}

void ColumnDiskAccumulated::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::align_right);
}

// ============================================================================
// ColumnIO
// ============================================================================

std::string ColumnIO::GetHeaderText() const { return "I/O"; }

void ColumnIO::RegisterRow(ProcessTree::Row& row) const {
  auto& process = row.Metrics;
  row.Io.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::CounterSlice>(
          std::make_shared<backend::metrics::Plus>(process.GetReadBytes(), process.GetWriteBytes()),
          Config::GetInstance().RefreshInterval),
      [&row](auto metric) { row.Io.Display = utils::DiskSizeToString(metric); });
}

std::string ColumnIO::GetDataText(bool isRowSelected, ProcessTree::Row& row) const { return row.Io.Display; }

void ColumnIO::Decorate(::ftxui::TableSelection selection) const { selection.DecorateCells(::ftxui::align_right); }

// ============================================================================
// ColumnIOAccumulated
// ============================================================================

std::string ColumnIOAccumulated::GetHeaderText() const { return "I/O+"; }

void ColumnIOAccumulated::RegisterRow(ProcessTree::Row& row) const {
  auto& process = row.Metrics;
  row.IoAccumulated.Updater = backend::metrics::MakeSubscriber(
      std::make_shared<backend::metrics::Plus>(process.GetReadBytes(), process.GetWriteBytes()),
      [&row](auto metric) { row.IoAccumulated.Display = utils::DiskSizeToString(metric); });
}

std::string ColumnIOAccumulated::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return row.IoAccumulated.Display;
}

void ColumnIOAccumulated::Decorate(::ftxui::TableSelection selection) const {
  selection.DecorateCells(::ftxui::align_right);
}

// ============================================================================
// ColumnStart
// ============================================================================

std::string ColumnStart::GetHeaderText() const { return "Start"; }

void ColumnStart::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnStart::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  return utils::FormatTime(utils::FromSteadyClock(row.Process.value().get().GetStartTime()));
}

void ColumnStart::Decorate(::ftxui::TableSelection selection) const {}

// ============================================================================
// ColumnCommand
// ============================================================================

std::string ColumnCommand::GetHeaderText() const { return "Command"; }

void ColumnCommand::RegisterRow(ProcessTree::Row& row) const {}

std::string ColumnCommand::GetDataText(bool isRowSelected, ProcessTree::Row& row) const {
  auto& process = row.Process.value().get();
  return utils::TreeString(process.GetTreePosition()) + FormatCommand(row.Metrics.GetCommandLine(process));
}

void ColumnCommand::Decorate(::ftxui::TableSelection selection) const {}

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
