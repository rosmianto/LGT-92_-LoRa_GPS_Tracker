#include <catch2/catch_test_macros.hpp>

TEST_CASE("Smoke test: Catch2 framework verification", "[smoke]") {
    REQUIRE(true);
    REQUIRE(1 + 1 == 2);
}

