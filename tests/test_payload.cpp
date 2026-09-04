#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/Payload.hpp"
#include <array>
#include <vector>

using namespace lgt92::core;
using Catch::Matchers::WithinAbs;

TEST_CASE("Payload: Startup beacon", "[payload]") {
    std::array<uint8_t, 4> buffer{};
    REQUIRE(PayloadEncoder::encode_startup(buffer));
    REQUIRE(buffer[0] == 0x11);
}

TEST_CASE("Payload: GPS NoFix encoding", "[payload]") {
    UplinkData data{};
    data.gps_fix = GpsFixState::NoFix;
    data.battery_mv = 3600;
    data.alarm = false;
    data.motion_mode = MotionMode::Movement;
    data.led_on = true;
    data.firmware_version = 0x04;
    data.full_payload = false;

    std::array<uint8_t, 16> buffer{};
    size_t len = PayloadEncoder::encode(data, buffer);
    REQUIRE(len == 11);

    // Bytes 0..7 must be 0xFF for NoFix
    for (size_t i = 0; i < 8; ++i) {
        REQUIRE(buffer[i] == 0xFF);
    }

    // Battery: 3600 = 0x0E10
    REQUIRE(buffer[8] == 0x0E);
    REQUIRE(buffer[9] == 0x10);

    // Flags: (1 << 6) | (1 << 5) | 0x04 = 0x40 | 0x20 | 0x04 = 0x64
    REQUIRE(buffer[10] == 0x64);
}

TEST_CASE("Payload: GPS Off encoding", "[payload]") {
    UplinkData data{};
    data.gps_fix = GpsFixState::Off;
    data.battery_mv = 3700;
    data.alarm = false;
    data.full_payload = false;

    std::array<uint8_t, 16> buffer{};
    size_t len = PayloadEncoder::encode(data, buffer);
    REQUIRE(len == 11);

    // Bytes 0..7 must be 0x00 for Off
    for (size_t i = 0; i < 8; ++i) {
        REQUIRE(buffer[i] == 0x00);
    }
}

TEST_CASE("Payload: Alarm flag bit 14", "[payload]") {
    UplinkData data{};
    data.gps_fix = GpsFixState::NoFix;
    data.battery_mv = 3600; // 0x0E10
    data.alarm = true;      // 0x4000 | 0x0E10 = 0x4E10
    data.full_payload = false;

    std::array<uint8_t, 16> buffer{};
    PayloadEncoder::encode(data, buffer);

    REQUIRE(buffer[8] == 0x4E);
    REQUIRE(buffer[9] == 0x10);

    auto decoded = PayloadEncoder::decode(std::span<const uint8_t>(buffer.data(), 11));
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->alarm == true);
    REQUIRE(decoded->battery_mv == 3600);
}

TEST_CASE("Payload: Real Dragino hex frame decode & encode match", "[payload]") {
    // Real LGT-92 uplink packet from TTN packet capture
    const std::vector<uint8_t> frame = {
        0x02, 0x86, 0x3D, 0x80, // Latitude = 42.352000
        0x07, 0x2A, 0x62, 0x20, // Longitude = 120.218144
        0x0E, 0x47,             // Battery = 3655 mV (Alarm = false)
        0x44,                   // MotionMode=1, LED=false, FW=4
        0x00, 0x05,             // Roll = 0.05
        0x00, 0x02,             // Pitch = 0.02
        0x64,                   // HDOP = 1.00
        0x01, 0xF4              // Alt = 5.00 m
    };

    auto decoded = PayloadEncoder::decode(frame);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->gps_fix == GpsFixState::Valid);
    REQUIRE_THAT(decoded->latitude, WithinAbs(42.352000, 0.000002));
    REQUIRE_THAT(decoded->longitude, WithinAbs(120.218144, 0.000002));
    REQUIRE(decoded->battery_mv == 3655);
    REQUIRE_FALSE(decoded->alarm);
    REQUIRE(decoded->motion_mode == MotionMode::Movement);
    REQUIRE_FALSE(decoded->led_on);
    REQUIRE(decoded->firmware_version == 0x04);
    REQUIRE_THAT(decoded->roll, WithinAbs(0.05f, 0.001f));
    REQUIRE_THAT(decoded->pitch, WithinAbs(0.02f, 0.001f));
    REQUIRE_THAT(decoded->hdop, WithinAbs(1.00f, 0.01f));
    REQUIRE_THAT(decoded->altitude_m, WithinAbs(5.00f, 0.01f));

    // Re-encode and verify exact byte match
    std::vector<uint8_t> reencoded(frame.size());
    size_t written = PayloadEncoder::encode(*decoded, reencoded);
    REQUIRE(written == frame.size());
    REQUIRE(reencoded == frame);
}

TEST_CASE("Payload: Negative coordinates (South and West)", "[payload]") {
    UplinkData data{};
    data.gps_fix = GpsFixState::Valid;
    data.latitude = -33.868820; // Sydney, Australia
    data.longitude = 151.209296;
    data.battery_mv = 3800;
    data.full_payload = true;
    data.roll = -12.5f;
    data.pitch = 8.3f;
    data.hdop = 0.8f;
    data.altitude_m = 45.0f;

    std::vector<uint8_t> buffer(18);
    PayloadEncoder::encode(data, buffer);

    auto decoded = PayloadEncoder::decode(buffer);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->gps_fix == GpsFixState::Valid);
    REQUIRE_THAT(decoded->latitude, WithinAbs(-33.868820, 0.000002));
    REQUIRE_THAT(decoded->longitude, WithinAbs(151.209296, 0.000002));
    REQUIRE_THAT(decoded->roll, WithinAbs(-12.5f, 0.01f));
    REQUIRE_THAT(decoded->pitch, WithinAbs(8.3f, 0.01f));
}

