file(GLOB omnimon_utils_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_library(omnimon-utils STATIC ${omnimon_utils_srcs})
