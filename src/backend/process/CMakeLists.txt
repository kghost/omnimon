file(GLOB omnimon_backend_process_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
omnimon_add_library(omnimon-backend-process STATIC ${omnimon_backend_process_srcs})
omnimon_target_link_libraries(omnimon-backend-process omnimon-backend-metrics)
omnimon_target_link_libraries(omnimon-backend-process omnimon-utils)

add_subdirectory(file)
