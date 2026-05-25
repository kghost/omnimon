file(GLOB omnimon_backend_events_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
omnimon_add_library(omnimon-backend-events STATIC ${omnimon_backend_events_srcs})
omnimon_target_link_libraries(omnimon-backend-events PRIVATE omnimon-utils)
