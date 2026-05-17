#include "Clock.hpp"

#include <chrono>
#include <time.h>
#include <unistd.h>

#include "Error.hpp"

namespace utils {

struct timespec DurationToTimeSpec(std::chrono::steady_clock::duration duration) {
  struct timespec ts;
  ts.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  ts.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - std::chrono::seconds(ts.tv_sec)).count();
  return ts;
}

std::chrono::nanoseconds TimeSpecToDuration(const struct timespec ts) {
  return std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec);
}

std::chrono::microseconds JiffyToDuration(uint64_t jiffies) {
  int microsecondPerJiffy = 1000000 / sysconf(_SC_CLK_TCK);
  return std::chrono::microseconds(jiffies * microsecondPerJiffy);
}

std::chrono::steady_clock::time_point JiffyBootTime() {
  static std::chrono::steady_clock::time_point boottime = [] {
    struct timespec ts;
    PosixE(clock_gettime(CLOCK_MONOTONIC, &ts));
    auto boot = std::chrono::steady_clock::now() - TimeSpecToDuration(ts);
    return std::chrono::floor<std::chrono::seconds>(boot);
  }();
  return boottime;
}

std::chrono::steady_clock::time_point JiffyToClock(uint64_t jiffies) {
  return JiffyBootTime() + JiffyToDuration(jiffies);
}

std::chrono::system_clock::time_point SystemBootTime() {
  static std::chrono::system_clock::time_point boottime = [] {
    struct timespec ts;
    PosixE(clock_gettime(CLOCK_MONOTONIC, &ts));
    auto boot = std::chrono::system_clock::now() - TimeSpecToDuration(ts);
    return std::chrono::floor<std::chrono::seconds>(boot);
  }();
  return boottime;
}

std::chrono::system_clock::time_point FromSteadyClock(std::chrono::steady_clock::time_point tp) {
  return SystemBootTime() + (tp - JiffyBootTime());
}

std::string FormatDuration(std::chrono::microseconds value) {
  if (value < std::chrono::minutes(1)) {
    return std::format("{:%S}", value);
  } else if (value < std::chrono::hours(1)) {
    return std::format("{:%M:%S}", value);
  } else if (value < std::chrono::days(1)) {
    return std::format("{:%H:%M:%S}", value);
  } else {
    auto days = std::chrono::duration_cast<std::chrono::days>(value).count();
    return std::format("{:d}d{:%H:%M:%S}", days, value);
  }
}

std::string FormatTime(std::chrono::system_clock::time_point value) {
  auto diff = std::chrono::system_clock::now() - value;
  auto local = std::chrono::zoned_time{std::chrono::current_zone(), value};

  if (diff < std::chrono::days(1)) {
    return std::format("{:%H:%M}", local);
  } else if (diff < std::chrono::years(1)) {
    return std::format("{:%b%d}", local);
  } else {
    return std::format("{:%Y}", local);
  }
}

} // namespace utils
