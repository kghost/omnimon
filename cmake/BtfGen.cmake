function(btf_gen)
  #file(GLOB btfs_defines LIST_DIRECTORIES false RELATIVE "/sys/kernel/btf" "/sys/kernel/btf/*")
  file(GLOB btfs_defines LIST_DIRECTORIES false RELATIVE "/sys/kernel/btf" "/sys/kernel/btf/vmlinux")
  set(outputs "")

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/btfs)

  foreach(btf_source ${btfs_defines})
    string(CONCAT btf_header "btfs/" ${btf_source} ".btf.h")
    list(APPEND outputs ${btf_header})

    add_custom_command(
      OUTPUT ${btf_header}
      COMMAND /usr/sbin/bpftool btf dump file /sys/kernel/btf/${btf_source} format c > ${btf_header}
    )

    set_source_files_properties(${btf_header} PROPERTIES GENERATED true)
  endforeach()

  add_library(btfs INTERFACE ${outputs})
  target_include_directories(btfs INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/btfs>")

endfunction(btf_gen)
