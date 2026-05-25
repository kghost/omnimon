omnimon_add_executable(omnimon-utils-tests RangeTest.cpp)
omnimon_target_link_libraries(omnimon-utils-tests PRIVATE omnimon-utils GTest::gtest GTest::gtest_main)
omnimon_add_test(NAME omnimon-utils-tests COMMAND omnimon-utils-tests)
