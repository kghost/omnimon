file(GLOB omnimon_backend_system_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
omnimon_add_library(omnimon-backend-system STATIC ${omnimon_backend_system_srcs})
omnimon_target_link_libraries(omnimon-backend-system omnimon-backend-metrics)

omnimon_add_executable(omnimon-system-walker tools/PrintSysInfo.cpp)
omnimon_target_link_libraries(omnimon-system-walker PRIVATE omnimon-backend-system)
