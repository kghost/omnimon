omnimon_add_library(omnimon-backend-network STATIC Interface.cpp Interface.hpp InterfaceMetrics.cpp InterfaceMetrics.hpp TrafficControlMetrics.cpp TrafficControlMetrics.hpp InterfaceManager.cpp InterfaceManager.hpp)
omnimon_target_link_libraries(omnimon-backend-network PUBLIC omnimon-backend-metrics omnimon-utils omnimon-backend-events)

# Walkers (Standalone raw metrics applications)
omnimon_add_executable(omnimon-network-interface-basic-walker tools/BasicWalker.cpp)
omnimon_target_link_libraries(omnimon-network-interface-basic-walker PRIVATE omnimon-backend-network)

omnimon_add_executable(omnimon-network-interface-metrics-walker tools/MetricsWalker.cpp)
omnimon_target_link_libraries(omnimon-network-interface-metrics-walker PRIVATE omnimon-backend-network)

omnimon_add_executable(omnimon-network-interface-traffic-control-walker tools/TrafficControlWalker.cpp)
omnimon_target_link_libraries(omnimon-network-interface-traffic-control-walker PRIVATE omnimon-backend-network)

# Tests
omnimon_add_executable(omnimon-network-interface-basic-tests tests/BasicTest.cpp)
omnimon_target_link_libraries(omnimon-network-interface-basic-tests PRIVATE omnimon-backend-network GTest::gtest GTest::gtest_main)
omnimon_add_test(NAME omnimon-network-interface-basic-tests COMMAND omnimon-network-interface-basic-tests)

omnimon_add_executable(omnimon-network-interface-metrics-tests tests/MetricsTest.cpp)
omnimon_target_link_libraries(omnimon-network-interface-metrics-tests PRIVATE omnimon-backend-network GTest::gtest GTest::gtest_main)
omnimon_add_test(NAME omnimon-network-interface-metrics-tests COMMAND omnimon-network-interface-metrics-tests)

omnimon_add_executable(omnimon-network-interface-traffic-control-tests tests/TrafficControlTest.cpp)
omnimon_target_link_libraries(omnimon-network-interface-traffic-control-tests PRIVATE omnimon-backend-network GTest::gtest GTest::gtest_main)
omnimon_add_test(NAME omnimon-network-interface-traffic-control-tests COMMAND omnimon-network-interface-traffic-control-tests)
