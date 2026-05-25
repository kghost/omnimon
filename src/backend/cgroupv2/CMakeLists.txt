file(GLOB omnimon_backend_cgroupv2_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
omnimon_add_library(omnimon-backend-cgroupv2 STATIC ${omnimon_backend_cgroupv2_srcs})
omnimon_target_link_libraries(omnimon-backend-cgroupv2 PRIVATE omnimon-backend-metrics omnimon-backend-events)

omnimon_add_executable(omnimon-cgroupv2-walker tools/WalkCGroupNodes.cpp)
omnimon_target_link_libraries(omnimon-cgroupv2-walker PRIVATE omnimon-backend-cgroupv2)
