#pragma once

#include "../../bpf/BpfManager.hpp"
#include "../../bpf/BpfMap.hpp"

#include "FileIo.bpf.h"

struct FileIo_bpf;

namespace backend::process::file {

class FileIo {
public:
  explicit FileIo(bpf::BpfManager& manager, uint32_t pid);
  ~FileIo();

  bpf::BpfMapRef<struct file_id, struct file_stat> GetResultMap();

private:
  struct FileIo_bpf* _BpfObj = nullptr;
};

} // namespace backend::process::file
