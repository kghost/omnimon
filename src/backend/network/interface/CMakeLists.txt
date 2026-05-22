add_library(omnimon-backend-network STATIC Interface.cpp Interface.hpp InterfaceMetrics.cpp InterfaceMetrics.hpp TrafficControlMetrics.cpp TrafficControlMetrics.hpp InterfaceManager.cpp InterfaceManager.hpp)
target_link_libraries(omnimon-backend-network PUBLIC omnimon-backend-metrics omnimon-utils omnimon-backend-events)


# Walkers (Standalone raw metrics applications)
add_executable(omnimon-network-interface-basic-walker tools/BasicWalker.cpp)
target_link_libraries(omnimon-network-interface-basic-walker PRIVATE omnimon-backend-network)

add_executable(omnimon-network-interface-metrics-walker tools/MetricsWalker.cpp)
target_link_libraries(omnimon-network-interface-metrics-walker PRIVATE omnimon-backend-network)

add_executable(omnimon-network-interface-traffic-control-walker tools/TrafficControlWalker.cpp)
target_link_libraries(omnimon-network-interface-traffic-control-walker PRIVATE omnimon-backend-network)

# Tests
add_executable(omnimon-network-interface-basic-tests tests/BasicTest.cpp)
target_link_libraries(omnimon-network-interface-basic-tests PRIVATE omnimon-backend-network GTest::gtest GTest::gtest_main)
add_test(NAME omnimon-network-interface-basic-tests COMMAND omnimon-network-interface-basic-tests)

add_executable(omnimon-network-interface-metrics-tests tests/MetricsTest.cpp)
target_link_libraries(omnimon-network-interface-metrics-tests PRIVATE omnimon-backend-network GTest::gtest GTest::gtest_main)
add_test(NAME omnimon-network-interface-metrics-tests COMMAND omnimon-network-interface-metrics-tests)

add_executable(omnimon-network-interface-traffic-control-tests tests/TrafficControlTest.cpp)
target_link_libraries(omnimon-network-interface-traffic-control-tests PRIVATE omnimon-backend-network GTest::gtest GTest::gtest_main)
add_test(NAME omnimon-network-interface-traffic-control-tests COMMAND omnimon-network-interface-traffic-control-tests)
