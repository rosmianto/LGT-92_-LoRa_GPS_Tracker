#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/TrackerEngine.hpp"
#include "mocks/MockHardware.hpp"

using namespace lgt92::core;
using namespace lgt92::tests;
using Catch::Matchers::WithinAbs;

TEST_CASE("TrackerEngine: Initialization and Startup Packet", "[engine]") {
    MockGps gps;
    MockImu imu;
    MockBattery battery;
    MockLoRaWan lora;
    MockStorage storage;
    MockTimerService timer;
    MockUi ui;

    TrackerEngine engine(gps, imu, battery, lora, storage, timer, ui);

    REQUIRE(engine.init());
    REQUIRE(engine.get_state() == TrackerState::Idle);

    // Startup packet (0x11) must have been sent
    REQUIRE(lora.sent_packets.size() == 1);
    REQUIRE(lora.sent_packets[0].size() == 1);
    REQUIRE(lora.sent_packets[0][0] == 0x11);

    // Green LED must have blinked on join/start
    REQUIRE(ui.green_blinks >= 1);
}

TEST_CASE("TrackerEngine: Periodic Uplink Cycle", "[engine]") {
    MockGps gps;
    MockImu imu;
    MockBattery battery;
    MockLoRaWan lora;
    MockStorage storage;
    MockTimerService timer;
    MockUi ui;

    TrackerEngine engine(gps, imu, battery, lora, storage, timer, ui);
    engine.init();
    lora.sent_packets.clear(); // Clear startup frame

    // Trigger motion -> sends immediate motion uplink
    engine.on_motion_detected();
    engine.step();
    REQUIRE(lora.sent_packets.size() == 1);
    lora.sent_packets.clear();

    // Now test periodic transmission after 300s
    timer.advance_seconds(100);
    engine.step();
    REQUIRE(lora.sent_packets.empty());

    // Advance to 301s -> triggers periodic tracking uplink
    timer.advance_seconds(201);
    engine.step();

    REQUIRE(lora.sent_packets.size() == 1);
    REQUIRE(lora.sent_packets[0].size() == 18); // Default full payload

    auto decoded = PayloadEncoder::decode(lora.sent_packets[0]);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->gps_fix == GpsFixState::Valid);
    REQUIRE_THAT(decoded->latitude, WithinAbs(48.1351, 0.001));
    REQUIRE_THAT(decoded->longitude, WithinAbs(11.5820, 0.001));
    REQUIRE(decoded->battery_mv == 3750);
    REQUIRE_FALSE(decoded->alarm);

    // Blue LED must have blinked for transmission
    REQUIRE(ui.blue_blinks >= 1);
}

TEST_CASE("TrackerEngine: SOS Alarm Button and Downlink Clear", "[engine]") {
    MockGps gps;
    MockImu imu;
    MockBattery battery;
    MockLoRaWan lora;
    MockStorage storage;
    MockTimerService timer;
    MockUi ui;

    TrackerEngine engine(gps, imu, battery, lora, storage, timer, ui);
    engine.init();
    lora.sent_packets.clear();

    // Press SOS button
    ui.press_alarm_button();
    REQUIRE(engine.is_alarm_active());
    REQUIRE(ui.red_blinks >= 1);

    // Step should immediately acquire sensors and transmit an alarm packet
    engine.step();
    REQUIRE(lora.sent_packets.size() == 1);

    auto decoded = PayloadEncoder::decode(lora.sent_packets[0]);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->alarm == true);

    // Downlink to clear alarm arrives: [0x02, 0x01]
    const std::vector<uint8_t> clear_cmd = {0x02, 0x01};
    lora.inject_downlink(2, clear_cmd);

    REQUIRE_FALSE(engine.is_alarm_active());
}

TEST_CASE("TrackerEngine: Downlink Configuration Update", "[engine]") {
    MockGps gps;
    MockImu imu;
    MockBattery battery;
    MockLoRaWan lora;
    MockStorage storage;
    MockTimerService timer;
    MockUi ui;

    TrackerEngine engine(gps, imu, battery, lora, storage, timer, ui);
    engine.init();

    // Change TDC to 60s via downlink [0x01, 0x00, 0x00, 0x3C]
    const std::vector<uint8_t> tdc_cmd = {0x01, 0x00, 0x00, 0x3C};
    lora.inject_downlink(2, tdc_cmd);

    REQUIRE(engine.get_config().transmit_interval_s == 60);

    // Verify change was saved to non-volatile storage
    TrackerConfig saved_config{};
    REQUIRE(storage.load_config(saved_config));
    REQUIRE(saved_config.transmit_interval_s == 60);
}
