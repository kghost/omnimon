#include "Process.hpp"

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

namespace backend::process {

const std::chrono::steady_clock::time_point Process::EPOCH;

Process::Process(const std::filesystem::path& dir)
    : _ProcDirPath(dir), _StartTime(EPOCH), _State(std::make_shared<ProcessGauge>(*this)),
      _Mem(std::make_shared<ProcessGauge>(*this)), _UserTime(std::make_shared<ProcessGauge>(*this)),
      _SystemTime(std::make_shared<ProcessGauge>(*this)), _ReadBytes(std::make_shared<ProcessGauge>(*this)),
      _WriteBytes(std::make_shared<ProcessGauge>(*this)), _ReadCalls(std::make_shared<ProcessGauge>(*this)),
      _WriteCalls(std::make_shared<ProcessGauge>(*this)), _DiskReadBytes(std::make_shared<ProcessGauge>(*this)),
      _DiskWriteBytes(std::make_shared<ProcessGauge>(*this)),
      _DiskCancelledWriteBytes(std::make_shared<ProcessGauge>(*this)) {}

Process::~Process() {
  if (_Parent) {
    _Parent->RemoveChild(GetPid());
  }
}

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

std::string Process::GetUser() const {
  static std::map<uid_t, std::string> userCache;
  if (auto it = userCache.find(_Info.uid); it != userCache.end()) {
    return it->second;
  } else {
    struct passwd* pw = getpwuid(_Info.uid);
    std::string user;
    if (pw) {
      user = pw->pw_name;
    } else {
      user = std::to_string(_Info.uid);
    }
    userCache[_Info.uid] = user;
    return user;
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

  struct stat st;
  if (stat(_ProcDirPath.c_str(), &st) == 0) {
    _Info.uid = st.st_uid;
  }

  auto time = utils::JiffyToClock(ps.starttime);
  if (_StartTime == Process::EPOCH) {
    _StartTime = time;
  } else {
    if (_StartTime != time) {
      // This is a new process with the same PID
      // TODO: reset metrics
    }
  }
  _LastUpdate = std::chrono::steady_clock::now();
  _Exists = true;

  _State->SetValue(ps.state);
  _Mem->SetValue(ps.rss);
  _UserTime->SetValue(ps.utime);
  _SystemTime->SetValue(ps.stime);
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

std::list<Process::ChildPosition> Process::GetTreePosition(std::shared_ptr<Process> me) {
  std::list<ChildPosition> result;
  for (auto [parent, current] = std::tuple{me->_Parent, me}; parent; current = parent, parent = parent->_Parent) {
    result.push_front(parent->GetChildPosition(current));
  }
  return result;
}

Process::ChildPosition Process::GetChildPosition(std::shared_ptr<const Process> child) const {
  if (_Children.empty()) {
    return ChildPosition::NotLast;
  }
  if (_Children.rbegin()->second.lock() == child) {
    return ChildPosition::Last;
  } else {
    return ChildPosition::NotLast;
  }
}

std::list<std::shared_ptr<Process>> Process::GetAncestors(std::shared_ptr<Process> p) {
  std::list<std::shared_ptr<Process>> ancestors;
  while (p) {
    ancestors.push_front(p);
    p = p->GetParent();
  }
  return ancestors;
}

} // namespace backend::process
