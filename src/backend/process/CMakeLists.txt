file(GLOB omnimon_backend_process_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_library(omnimon-backend-process STATIC ${omnimon_backend_process_srcs})
target_link_libraries(omnimon-backend-process omnimon-backend-metrics)
target_link_libraries(omnimon-backend-process omnimon-utils)

add_subdirectory(file)
