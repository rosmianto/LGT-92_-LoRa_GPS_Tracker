#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "adapters/FusionAdapter.hpp"

using namespace lgt92::adapters;
using Catch::Matchers::WithinAbs;

TEST_CASE("FusionAdapter: Level static sensor orientation", "[imu]") {
    FusionAdapter ahrs(100.0f); // 100 Hz

    // Gravity pointing straight down (+Z in North-West-Up or +1g)
    for (int i = 0; i < 100; ++i) {
        ahrs.update_6axis({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    }

    auto euler = ahrs.get_euler_angles();
    REQUIRE_THAT(euler.roll, WithinAbs(0.0f, 2.0f));
    REQUIRE_THAT(euler.pitch, WithinAbs(0.0f, 2.0f));
}

TEST_CASE("FusionAdapter: 45 degree tilt response", "[imu]") {
    FusionAdapter ahrs(100.0f);

    // Initial flat orientation
    for (int i = 0; i < 50; ++i) {
        ahrs.update_6axis({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    }

    // Tilt 45 degrees nose up: Accel X = -sin(45 deg) = -0.707g, Accel Z = cos(45 deg) = 0.707g
    for (int i = 0; i < 100; ++i) {
        ahrs.update_6axis({0.0f, 0.0f, 0.0f}, {-0.707f, 0.0f, 0.707f});
    }

    auto euler = ahrs.get_euler_angles();
    REQUIRE_THAT(euler.pitch, WithinAbs(45.0f, 5.0f));
}

