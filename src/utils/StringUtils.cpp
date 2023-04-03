#include "StringUtils.hpp"

namespace utils {

bool StringIsAsciiPrintable(const std::string& str) {
  return std::all_of(str.begin(), str.end(), [](char c) { return c >= ' ' && c <= '~'; });
}

DisplayLength GraphemeWidth(const icu::UnicodeString& ustr, int32_t start, int32_t end) {
  // Use the width of first codepoint to determine the width of the grapheme cluster
  UEastAsianWidth w =
      static_cast<UEastAsianWidth>(u_getIntPropertyValue(ustr.char32At(start), UProperty::UCHAR_EAST_ASIAN_WIDTH));
  switch (w) {
  case UEastAsianWidth::U_EA_FULLWIDTH:
  case UEastAsianWidth::U_EA_WIDE:
    return 2;
  case UEastAsianWidth::U_EA_AMBIGUOUS:
  case UEastAsianWidth::U_EA_NEUTRAL:
  case UEastAsianWidth::U_EA_NARROW:
  case UEastAsianWidth::U_EA_HALFWIDTH:
    return 1;
  default:
    return 0;
  }
}

DisplayLength StringDisplayTruncate(std::string& str, DisplayLength max) {
  // Truncate the string to the maximum display width. Return the width of the truncated string.
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str);
  str.clear();

  UErrorCode uerr = UErrorCode::U_ZERO_ERROR;

  // Create a BreakIterator for grapheme clusters
  std::unique_ptr<icu::BreakIterator> iter(
      icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), uerr));

  // Set the text to iterate over
  iter->setText(ustr);

  DisplayLength width = 0;

  // Iterate over each grapheme cluster in the BreakIterator
  for (int32_t start = iter->first(), end = iter->next(); end != icu::BreakIterator::DONE;
       start = end, end = iter->next()) {
    DisplayLength w = GraphemeWidth(ustr, start, end);
    if (width + w > max) {
      ustr.tempSubStringBetween(0, start).toUTF8String(str);
      return width;
    }
    width += w;
  }

  ustr.toUTF8String(str);
  return width;
}

} // namespace utils
