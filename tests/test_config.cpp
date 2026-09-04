#include <catch2/catch_test_macros.hpp>
#include "core/TrackerConfig.hpp"
#include <vector>

using namespace lgt92::core;

TEST_CASE("TrackerConfig: Default values", "[config]") {
    TrackerConfig config{};

    REQUIRE(config.transmit_interval_s == 300);
    REQUIRE(config.alarm_interval_s == 60);
    REQUIRE(config.keep_alive_interval_s == 21600);
    REQUIRE(config.gps_positioning_time_s == 150);
    REQUIRE(config.motion_detection_mode == 1);
    REQUIRE(config.motion_threshold == 10);
    REQUIRE(config.payload_format == 0);
    REQUIRE_FALSE(config.confirmed_uplinks);
    REQUIRE(config.led_enabled);
    REQUIRE(config.lora_port == 2);
    REQUIRE(config.adr_enabled);
}

TEST_CASE("TrackerConfig: Serialization and Deserialization", "[config]") {
    TrackerConfig original{};
    original.transmit_interval_s = 600;
    original.alarm_interval_s = 30;
    original.keep_alive_interval_s = 10800;
    original.gps_positioning_time_s = 90;
    original.motion_detection_mode = 2;
    original.motion_threshold = 15;
    original.payload_format = 1;
    original.confirmed_uplinks = true;
    original.led_enabled = false;
    original.lora_port = 10;
    original.adr_enabled = false;
    original.tx_data_rate = 3;

    std::vector<uint8_t> buffer(original.serialized_size());
    REQUIRE(original.serialize(buffer));

    TrackerConfig restored{};
    REQUIRE(restored.deserialize(buffer));

    REQUIRE(restored.transmit_interval_s == 600);
    REQUIRE(restored.alarm_interval_s == 30);
    REQUIRE(restored.keep_alive_interval_s == 10800);
    REQUIRE(restored.gps_positioning_time_s == 90);
    REQUIRE(restored.motion_detection_mode == 2);
    REQUIRE(restored.motion_threshold == 15);
    REQUIRE(restored.payload_format == 1);
    REQUIRE(restored.confirmed_uplinks == true);
    REQUIRE(restored.led_enabled == false);
    REQUIRE(restored.lora_port == 10);
    REQUIRE(restored.adr_enabled == false);
    REQUIRE(restored.tx_data_rate == 3);
}

TEST_CASE("TrackerConfig: CRC Corrupted buffer detection", "[config]") {
    TrackerConfig config{};
    std::vector<uint8_t> buffer(config.serialized_size());
    REQUIRE(config.serialize(buffer));

    // Corrupt a byte in payload
    buffer[sizeof(uint32_t) + 2] ^= 0xFF;

    TrackerConfig corrupted{};
    REQUIRE_FALSE(corrupted.deserialize(buffer));
}

TEST_CASE("TrackerConfig: Buffer too small", "[config]") {
    TrackerConfig config{};
    std::vector<uint8_t> small_buffer(10);

    REQUIRE_FALSE(config.serialize(small_buffer));
    REQUIRE_FALSE(config.deserialize(small_buffer));
}

