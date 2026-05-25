file(GLOB omnimon_backend_system_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_library(omnimon-backend-system STATIC ${omnimon_backend_system_srcs})
target_link_libraries(omnimon-backend-system omnimon-backend-metrics)

add_executable(omnimon-system-walker tools/PrintSysInfo.cpp)
target_link_libraries(omnimon-system-walker PRIVATE omnimon-backend-system)
