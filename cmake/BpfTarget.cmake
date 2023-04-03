function(bpf_program)
  set(ONE_VALUE NAME)
  set(MULTI_VALUE BPF_SRC BPF_HEADERS)
  cmake_parse_arguments(BPF_PROGRAM "" "${ONE_VALUE}" "${MULTI_VALUE}" ${ARGN})

  string(REGEX REPLACE "\.bpf\.c$" ".bpf.o" bpf_program ${BPF_PROGRAM_BPF_SRC})
  string(REGEX REPLACE "\.bpf\.c$" ".skel.h" bpf_skel ${BPF_PROGRAM_BPF_SRC})

  add_custom_command(
    OUTPUT ${bpf_skel}
    COMMAND clang -g -O2 -target bpf -D__TARGET_ARCH_${ARCH} -I${KERNELHEADERS_DIR} -I${CMAKE_BINARY_DIR}/src/backend/bpf/btfs -c ${CMAKE_CURRENT_SOURCE_DIR}/${BPF_PROGRAM_BPF_SRC} -o ${bpf_program}
    COMMAND /usr/sbin/bpftool gen skeleton ${bpf_program} > ${bpf_skel}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${BPF_PROGRAM_BPF_SRC}
    DEPENDS btfs ${CMAKE_CURRENT_SOURCE_DIR}/${BPF_PROGRAM_BPF_HEADERS})

  set_source_files_properties(${bpf_skel} PROPERTIES EXTERNAL_OBJECT true GENERATED true)

endfunction(bpf_program)
