bpf_program(NAME bpf-file-io BPF_SRC FileIo.bpf.c)

add_library(process-file-io STATIC FileIo.cpp  ${CMAKE_CURRENT_BINARY_DIR}/FileIo.skel.h)
target_link_libraries(process-file-io PRIVATE btfs)
target_link_libraries(process-file-io PRIVATE bpf_manager)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
