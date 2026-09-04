#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "adapters/LwGpsAdapter.hpp"

using namespace lgt92::adapters;
using Catch::Matchers::WithinAbs;

TEST_CASE("LwGpsAdapter: Parse standard GPGGA, GPGSA, and GPRMC", "[gps]") {
    LwGpsAdapter gps{};
    REQUIRE_FALSE(gps.has_valid_fix());

    // Valid GPGGA sentence: 48 deg 07.038' N, 011 deg 31.000' E (Munich area)
    const char* gpgga = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    gps.feed(gpgga);

    // Valid GPGSA sentence with PDOP = 1.2, HDOP = 0.9, VDOP = 0.8
    const char* gpgsa = "$GPGSA,A,3,01,02,03,04,05,06,07,08,,,,,1.2,0.9,0.8*38\r\n";
    gps.feed(gpgsa);

    const char* gprmc = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n";
    gps.feed(gprmc);

    REQUIRE(gps.has_valid_fix());
    auto fix = gps.get_fix_data();
    REQUIRE(fix.has_fix);
    REQUIRE_THAT(fix.latitude, WithinAbs(48.1173, 0.001));
    REQUIRE_THAT(fix.longitude, WithinAbs(11.516667, 0.001));
    REQUIRE_THAT(fix.altitude_m, WithinAbs(545.4f, 0.1f));
    REQUIRE_THAT(fix.hdop, WithinAbs(0.9f, 0.05f));
    REQUIRE(fix.satellites_in_use == 8);
}

TEST_CASE("LwGpsAdapter: Invalid checksum rejection", "[gps]") {
    LwGpsAdapter gps{};

    // Bad checksum
    const char* bad_sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*00\r\n";
    gps.feed(bad_sentence);

    REQUIRE_FALSE(gps.has_valid_fix());
}

TEST_CASE("LwGpsAdapter: Character by character feeding", "[gps]") {
    LwGpsAdapter gps{};
    const char* sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";

    for (const char* p = sentence; *p != '\0'; ++p) {
        gps.feed(*p);
    }

    const char* rmc = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n";
    for (const char* p = rmc; *p != '\0'; ++p) {
        gps.feed(*p);
    }

    REQUIRE(gps.has_valid_fix());
}
