#include <catch2/catch_test_macros.hpp>
#include "core/DownlinkParser.hpp"
#include <vector>

using namespace lgt92::core;

TEST_CASE("Downlink: Set Transmit Interval (TDC 0x01)", "[downlink]") {
    TrackerConfig config{};
    config.transmit_interval_s = 60;

    // 0x00012C = 300 seconds
    const std::vector<uint8_t> payload = {0x01, 0x00, 0x01, 0x2C};
    auto cmd = DownlinkParser::parse(payload);
    REQUIRE(cmd.has_value());

    auto res = DownlinkParser::execute(*cmd, config);
    REQUIRE(res.config_modified);
    REQUIRE(config.transmit_interval_s == 300);
}

TEST_CASE("Downlink: TDC minimum clamp (0x01)", "[downlink]") {
    TrackerConfig config{};

    // 2 seconds requested -> should clamp to 6 seconds
    const std::vector<uint8_t> payload = {0x01, 0x00, 0x00, 0x02};
    auto cmd = DownlinkParser::parse(payload);
    REQUIRE(cmd.has_value());

    auto res = DownlinkParser::execute(*cmd, config);
    REQUIRE(res.config_modified);
    REQUIRE(config.transmit_interval_s == 6);
}

TEST_CASE("Downlink: System Reset ATZ (0x04 0xFF)", "[downlink]") {
    TrackerConfig config{};
    const std::vector<uint8_t> payload = {0x04, 0xFF};
    auto cmd = DownlinkParser::parse(payload);
    REQUIRE(cmd.has_value());

    auto res = DownlinkParser::execute(*cmd, config);
    REQUIRE(res.reset_requested);
    REQUIRE_FALSE(res.factory_reset_requested);
    REQUIRE_FALSE(res.config_modified);
}

TEST_CASE("Downlink: Factory Data Reset AT+FDR (0x04 0xFE)", "[downlink]") {
    TrackerConfig config{};
    config.transmit_interval_s = 9999;

    const std::vector<uint8_t> payload = {0x04, 0xFE};
    auto cmd = DownlinkParser::parse(payload);
    REQUIRE(cmd.has_value());

    auto res = DownlinkParser::execute(*cmd, config);
    REQUIRE(res.reset_requested);
    REQUIRE(res.factory_reset_requested);
    REQUIRE(res.config_modified);
    // Verified reset to default
    REQUIRE(config.transmit_interval_s == 300);
}

TEST_CASE("Downlink: Confirmed uplinks toggle (0x05)", "[downlink]") {
    TrackerConfig config{};
    config.confirmed_uplinks = false;

    // Enable CFM
    auto cmd_on = DownlinkParser::parse(std::vector<uint8_t>{0x05, 0x01});
    REQUIRE(cmd_on.has_value());
    DownlinkParser::execute(*cmd_on, config);
    REQUIRE(config.confirmed_uplinks == true);

    // Disable CFM
    auto cmd_off = DownlinkParser::parse(std::vector<uint8_t>{0x05, 0x00});
    REQUIRE(cmd_off.has_value());
    DownlinkParser::execute(*cmd_off, config);
    REQUIRE(config.confirmed_uplinks == false);
}

TEST_CASE("Downlink: Clear Alarm (0x02 0x01)", "[downlink]") {
    TrackerConfig config{};
    const std::vector<uint8_t> payload = {0x02, 0x01};
    auto cmd = DownlinkParser::parse(payload);
    REQUIRE(cmd.has_value());

    auto res = DownlinkParser::execute(*cmd, config);
    REQUIRE(res.alarm_cleared);
}

TEST_CASE("Downlink: Tracker parameter tuning (0x20..0x25)", "[downlink]") {
    TrackerConfig config{};

    // 0x20: GPS Positioning time = 45s
    auto cmd_20 = DownlinkParser::parse(std::vector<uint8_t>{0x20, 45});
    REQUIRE(cmd_20.has_value());
    DownlinkParser::execute(*cmd_20, config);
    REQUIRE(config.gps_positioning_time_s == 45);

    // 0x21: Motion detection mode = 2
    auto cmd_21 = DownlinkParser::parse(std::vector<uint8_t>{0x21, 2});
    REQUIRE(cmd_21.has_value());
    DownlinkParser::execute(*cmd_21, config);
    REQUIRE(config.motion_detection_mode == 2);

    // 0x23: Motion threshold = 25
    auto cmd_23 = DownlinkParser::parse(std::vector<uint8_t>{0x23, 25});
    REQUIRE(cmd_23.has_value());
    DownlinkParser::execute(*cmd_23, config);
    REQUIRE(config.motion_threshold == 25);

    // 0x24: Keep-alive interval = 3600s (0x000E10)
    auto cmd_24 = DownlinkParser::parse(std::vector<uint8_t>{0x24, 0x00, 0x0E, 0x10});
    REQUIRE(cmd_24.has_value());
    DownlinkParser::execute(*cmd_24, config);
    REQUIRE(config.keep_alive_interval_s == 3600);

    // 0x25: Payload format = 1 (short)
    auto cmd_25 = DownlinkParser::parse(std::vector<uint8_t>{0x25, 1});
    REQUIRE(cmd_25.has_value());
    DownlinkParser::execute(*cmd_25, config);
    REQUIRE(config.payload_format == 1);
}

TEST_CASE("Downlink: Malformed and truncated frames", "[downlink]") {
    // Empty
    REQUIRE_FALSE(DownlinkParser::parse(std::vector<uint8_t>{}).has_value());

    // Truncated 0x01 (needs 4 bytes)
    REQUIRE_FALSE(DownlinkParser::parse(std::vector<uint8_t>{0x01, 0x00}).has_value());

    // Truncated 0x04 (needs 2 bytes)
    REQUIRE_FALSE(DownlinkParser::parse(std::vector<uint8_t>{0x04}).has_value());

    // Unknown opcode
    auto unknown = DownlinkParser::parse(std::vector<uint8_t>{0xDE, 0xAD});
    REQUIRE(unknown.has_value());
    REQUIRE(std::holds_alternative<downlink::UnknownCommand>(*unknown));
}

