pkg_check_modules(BPF libbpf REQUIRED)

include(BtfGen)
include(BpfTarget)

btf_gen()

file(GLOB bpf_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_library(bpf_manager STATIC ${bpf_srcs})
