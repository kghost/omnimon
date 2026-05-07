#pragma once

#include <string>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>

using DisplayLength = int;

namespace utils {

bool StringIsAsciiPrintable(const std::string& str);
DisplayLength GraphemeWidth(UChar32 character);
DisplayLength StringDisplayTruncate(std::string& str, DisplayLength max);

} // namespace utils
