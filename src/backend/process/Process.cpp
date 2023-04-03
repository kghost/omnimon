#include "Process.hpp"

#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <ranges>
#include <regex>
#include <sstream>
#include <string>

#include "../../utils/Clock.hpp"
#include "../../utils/Error.hpp"

namespace backend::process {

void Process::Update() {
  ParseStatFile();
  if (_Exists) {
    ParseIoFile();
  }
}

std::string Process::GetCommandLine() const {
  std::ostringstream result;

  try {
    utils::FileHandle fd(PosixE(open((_ProcDirPath / "cmdline").c_str(), O_RDONLY)));
    std::array<char, 1024> buffer;
    for (ssize_t bytes = PosixE(read(fd, buffer.data(), buffer.size())); bytes > 0;
         bytes = PosixE(read(fd, buffer.data(), buffer.size()))) {
      result.write(buffer.data(), bytes);
    }
  } catch (const std::system_error& e) {
    if (e.code() == std::errc::no_such_file_or_directory || e.code() == std::errc::no_such_process) {
      return _Info.comm;
    } else {
      throw;
    }
  }

  if (result.tellp() == 0) {
    return _Info.comm;
  } else {
    return result.str();
  }
}

void Process::ParseStatFile() {
  struct ProcessStat {
    char state;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned int flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long long starttime;
    unsigned long vsize;
    long rss;
  };

  ProcessStat ps;
  std::ifstream ifs(_ProcDirPath / "stat");

  if (!ifs.is_open()) {
    _Exists = false;
    return;
  }

  std::string content;
  try {
    content = std::string(std::istreambuf_iterator<char>(ifs), {});
  } catch (const std::ios_base::failure& e) {
    _Exists = false;
    return;
  }

  auto start = content.find_first_of('(');
  auto end = content.find_last_of(')');

  std::istringstream beforeComm(content.substr(0, start));
  std::istringstream afterComm(content.substr(end + 1, content.length() - (end + 1)));

  beforeComm >> _Info.pid;
  _Info.comm = content.substr(start, end - start + 1);
  afterComm >> ps.state >> _Info.ppid >> ps.pgrp >> ps.session >> ps.tty_nr >> ps.tpgid >> ps.flags >> ps.minflt >>
      ps.cminflt >> ps.majflt >> ps.cmajflt >> ps.utime >> ps.stime >> ps.cutime >> ps.cstime >> ps.priority >>
      ps.nice >> ps.num_threads >> ps.itrealvalue >> ps.starttime >> ps.vsize >> ps.rss;

  _StartTime = utils::JiffyToClock(ps.starttime);
  _LastUpdate = std::chrono::steady_clock::now();
  _Exists = true;

  if (auto ptr = _State.lock()) {
    ptr->SetValue(ps.state);
  }

  if (auto ptr = _Mem.lock()) {
    ptr->SetValue(ps.rss);
  }

  if (auto ptr = _UserTime.lock()) {
    ptr->SetValue(ps.utime);
  }

  if (auto ptr = _SystemTime.lock()) {
    ptr->SetValue(ps.stime);
  }
}

void Process::ParseIoFile() {
  // Open the file
  std::ifstream ifs(_ProcDirPath / "io");
  if (!ifs.is_open()) {
    return;
  }

  // Read the file contents into a string
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

    if (key == "rchar") {
      if (auto ptr = _ReadBytes.lock()) {
        ptr->SetValue(value);
      }
    } else if (key == "wchar") {
      if (auto ptr = _WriteBytes.lock()) {
        ptr->SetValue(value);
      }
    } else if (key == "syscr") {
      if (auto ptr = _ReadCalls.lock()) {
        ptr->SetValue(value);
      }
    } else if (key == "syscw") {
      if (auto ptr = _WriteCalls.lock()) {
        ptr->SetValue(value);
      }
    } else if (key == "read_bytes") {
      if (auto ptr = _DiskReadBytes.lock()) {
        ptr->SetValue(value);
      }
    } else if (key == "write_bytes") {
      if (auto ptr = _DiskWriteBytes.lock()) {
        ptr->SetValue(value);
      }
    } else if (key == "cancelled_write_bytes") {
      if (auto ptr = _DiskCancelledWriteBytes.lock()) {
        ptr->SetValue(value);
      }
    }
  }
}

std::shared_ptr<metrics::Gauge> Process::GetGauge(std::shared_ptr<Process> me, GaugeMember member) {
  if (auto result = (me.get()->*member).lock()) {
    return result;
  } else {
    result = std::make_shared<ProcessGauge>(me);
    (me.get()->*member) = result;
    return result;
  }
}

} // namespace backend::process
