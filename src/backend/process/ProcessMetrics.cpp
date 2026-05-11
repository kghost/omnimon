#include "ProcessMetrics.hpp"

#include <fcntl.h>
#include <fstream>
#include <pwd.h>
#include <ranges>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>

#include "../../utils/Clock.hpp"
#include "../../utils/Error.hpp"
#include "Process.hpp"

namespace backend::process {

ProcessMetrics::ProcessMetrics(std::shared_ptr<Process> process)
    : _Process(process), _ReadBytes(std::make_shared<ProcessGauge>(*this)),
      _WriteBytes(std::make_shared<ProcessGauge>(*this)), _ReadCalls(std::make_shared<ProcessGauge>(*this)),
      _WriteCalls(std::make_shared<ProcessGauge>(*this)), _DiskReadBytes(std::make_shared<ProcessGauge>(*this)),
      _DiskWriteBytes(std::make_shared<ProcessGauge>(*this)),
      _DiskCancelledWriteBytes(std::make_shared<ProcessGauge>(*this)) {
  UpdateMetrics();
}

std::string ProcessMetrics::GetCommandLine() const {
  std::ostringstream result;

  try {
    utils::FileHandle fd(PosixE(open((_Process->GetProcDirPath() / "cmdline").c_str(), O_RDONLY)));
    std::array<char, 1024> buffer;
    for (ssize_t bytes = PosixE(read(fd, buffer.data(), buffer.size())); bytes > 0;
         bytes = PosixE(read(fd, buffer.data(), buffer.size()))) {
      result.write(buffer.data(), bytes);
    }
  } catch (const std::system_error& e) {
    if (e.code() == std::errc::no_such_file_or_directory || e.code() == std::errc::no_such_process) {
      return _Process->GetComm();
    } else {
      throw;
    }
  }

  if (result.tellp() == 0) {
    return _Process->GetComm();
  } else {
    return result.str();
  }
}

void ProcessMetrics::UpdateMetrics() {
  if (!_Process->Exists()) {
    return;
  }

  std::ifstream ifs(_Process->GetProcDirPath() / "io");
  if (!ifs.is_open()) {
    return;
  }

  std::string content;
  try {
    content = std::string(std::istreambuf_iterator<char>(ifs), {});
  } catch (const std::ios_base::failure& e) {
    return;
  }

  for (const auto line : std::views::split(content, std::string_view{"\n"})) {
    static std::regex pattern(std::string("^(\\w+):\\s+(\\d+)$"), std::regex::optimize);

    std::match_results<decltype(line.begin())> match;
    if (!std::regex_match(line.begin(), line.end(), match, pattern)) {
      continue;
    }

    std::string key = match[1].str();
    metrics::DataType value = std::stoll(match[2].str());
    _LastUpdate = std::chrono::steady_clock::now();

    if (key == "rchar") {
      _ReadBytes->SetValue(value);
    } else if (key == "wchar") {
      _WriteBytes->SetValue(value);
    } else if (key == "syscr") {
      _ReadCalls->SetValue(value);
    } else if (key == "syscw") {
      _WriteCalls->SetValue(value);
    } else if (key == "read_bytes") {
      _DiskReadBytes->SetValue(value);
    } else if (key == "write_bytes") {
      _DiskWriteBytes->SetValue(value);
    } else if (key == "cancelled_write_bytes") {
      _DiskCancelledWriteBytes->SetValue(value);
    }
  }
}

} // namespace backend::process
