file(GLOB omnimon_utils_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
omnimon_add_library(omnimon-utils STATIC ${omnimon_utils_srcs})
omnimon_target_link_libraries(omnimon-utils PUBLIC ${ICUUC_LIBRARIES} ${ICUI18N_LIBRARIES})

add_subdirectory(tests)
