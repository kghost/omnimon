file(GLOB omnimon_backend_cgroupv2_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_library(omnimon-backend-cgroupv2 STATIC ${omnimon_backend_cgroupv2_srcs})
target_link_libraries(omnimon-backend-cgroupv2 PRIVATE omnimon-backend-metrics omnimon-backend-events)

add_executable(omnimon-cgroupv2-walker tools/WalkCGroupNodes.cpp)
target_link_libraries(omnimon-cgroupv2-walker PRIVATE omnimon-backend-cgroupv2)
