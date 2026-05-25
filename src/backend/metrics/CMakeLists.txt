file(GLOB omnimon_backend_metrics_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
omnimon_add_library(omnimon-backend-metrics STATIC ${omnimon_backend_metrics_srcs})
