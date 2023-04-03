#include <bpf/libbpf.h>
#include <format>
#include <stdexcept>

#include "FileIo.bpf.h"
#include "FileIo.hpp"
#include "FileIo.skel.h"

namespace backend::process::file {

FileIo::FileIo(bpf::BpfManager& manager, uint32_t pid) {
  LIBBPF_OPTS(bpf_object_open_opts, open_opts);
  _BpfObj = FileIo_bpf__open_opts(&open_opts);
  if (!_BpfObj) {
    throw std::runtime_error("failed to open BPF object");
  }

  _BpfObj->rodata->target_pid = 0;

  int err = FileIo_bpf__load(_BpfObj);
  if (err) {
    throw std::runtime_error(std::format("failed to load BPF object: {}", err));
  }

  err = FileIo_bpf__attach(_BpfObj);
  if (err) {
    throw std::runtime_error(std::format("failed to attach BPF programs: {}", err));
  }
}

FileIo::~FileIo() {
  if (_BpfObj != nullptr) {
    FileIo_bpf__destroy(_BpfObj);
  }
}

bpf::BpfMapRef<struct file_id, struct file_stat> FileIo::GetResultMap() {
  return bpf::BpfMapRef<struct file_id, struct file_stat>{_BpfObj->maps.entries};
}

} // namespace backend::process::file