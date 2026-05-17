#pragma once

#include <chrono>
#include <string>
#include <time.h>

namespace utils {

struct timespec DurationToTimeSpec(std::chrono::steady_clock::duration duration);
std::chrono::nanoseconds TimeSpecToDuration(const struct timespec ts);
std::chrono::microseconds JiffyToDuration(uint64_t jiffies);
std::chrono::steady_clock::time_point JiffyBootTime();
std::chrono::steady_clock::time_point JiffyToClock(uint64_t jiffies);

std::chrono::system_clock::time_point SystemBootTime();
std::chrono::system_clock::time_point FromSteadyClock(std::chrono::steady_clock::time_point tp);

std::string FormatDuration(std::chrono::microseconds value);
std::string FormatTime(std::chrono::system_clock::time_point value);

} // namespace utils
