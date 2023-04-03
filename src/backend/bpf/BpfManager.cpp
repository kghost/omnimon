#include "BpfManager.hpp"

#include <type_traits>
#include <unistd.h>

namespace backend::bpf {

BpfManager& BpfManager::GetInstance() {
  static BpfManager instance;
  return instance;
}

BpfManager::BpfManager() {
  libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
  libbpf_set_print(&BpfPrintFn);
}

BpfManager::~BpfManager() {}

int BpfManager::BpfPrintFn(enum libbpf_print_level level, const char* format, va_list args) {
  return vfprintf(stderr, format, args);
}

} // namespace backend::bpf