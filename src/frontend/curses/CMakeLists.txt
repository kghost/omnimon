add_subdirectory(layouts)

pkg_check_modules(NCURSESW REQUIRED ncursesw)

file(GLOB omnimon_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_executable(omnimon ${omnimon_srcs})

target_link_libraries(omnimon ${BPF_LIBRARIES})
target_link_libraries(omnimon omnimon-backend-process)
target_link_libraries(omnimon omnimon-backend-system)
target_link_libraries(omnimon process-file-io)
target_link_libraries(omnimon omnimon-frontend-curses-layout)
target_link_libraries(omnimon ${NCURSESW_LIBRARIES})
target_link_libraries(omnimon ${ICUUC_LIBRARIES})

target_include_directories(omnimon PUBLIC ${BPF_INCLUDE_DIRS})
target_compile_options(omnimon PUBLIC ${BPF_CFLAGS_OTHER})
