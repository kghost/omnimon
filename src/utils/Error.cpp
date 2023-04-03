#include "Error.hpp"

#include <unistd.h>

namespace utils {

FileHandle::~FileHandle() { PosixE(close(_Fd)); }

} // namespace utils
