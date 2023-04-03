#pragma once

#include <algorithm>
#include <string>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>

using DisplayLength = int;

namespace utils {

bool StringIsAsciiPrintable(const std::string& str);
DisplayLength GraphemeWidth(const icu::UnicodeString& ustr, int32_t start, int32_t end);
DisplayLength StringDisplayTruncate(std::string& str, DisplayLength max);

} // namespace utils
