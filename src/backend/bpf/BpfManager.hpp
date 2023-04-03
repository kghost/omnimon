#pragma once

#include <bpf/libbpf.h>

namespace backend::bpf {

class BpfManager {
public:
  static BpfManager& GetInstance();

  explicit BpfManager();
  ~BpfManager();

private:
  static int BpfPrintFn(enum libbpf_print_level level, const char* format, va_list args);
};

} // namespace backend::bpf