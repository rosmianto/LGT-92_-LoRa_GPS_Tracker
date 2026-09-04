add_executable(lgt92_tests
    test_main.cpp
    test_config.cpp
    test_payload.cpp
    test_downlink.cpp
    test_gps_adapter.cpp
    test_fusion_adapter.cpp
    test_tracker_engine.cpp
)

target_link_libraries(lgt92_tests PRIVATE
    lgt92_core
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(lgt92_tests)

