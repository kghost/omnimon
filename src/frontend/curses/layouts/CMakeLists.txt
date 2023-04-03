file(GLOB omnimon_frontend_curses_layout_srcs LIST_DIRECTORIES false "./*.cpp" "./*.hpp")
add_library(omnimon-frontend-curses-layout STATIC ${omnimon_frontend_curses_layout_srcs})
target_include_directories(omnimon-frontend-curses-layout PUBLIC ${NCURSESW_INCLUDE_DIRS})
target_link_libraries(omnimon-frontend-curses-layout ${NCURSESW_LIBRARIES})
