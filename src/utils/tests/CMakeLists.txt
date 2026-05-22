add_executable(omnimon-utils-tests RangeTest.cpp)
target_link_libraries(omnimon-utils-tests PRIVATE omnimon-utils GTest::gtest GTest::gtest_main)
add_test(NAME omnimon-utils-tests COMMAND omnimon-utils-tests)
