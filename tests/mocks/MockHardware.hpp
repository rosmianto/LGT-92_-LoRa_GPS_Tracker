#pragma once

#include "interfaces/IGps.hpp"
#include "interfaces/IImu.hpp"
#include "interfaces/IBattery.hpp"
#include "interfaces/ILoRaWan.hpp"
#include "interfaces/IStorage.hpp"
#include "interfaces/ITimerService.hpp"
#include "interfaces/IUi.hpp"
#include <vector>

namespace lgt92::tests {

class MockGps : public interfaces::IGps {
public:
    bool powered{false};
    bool valid_fix{true};
    adapters::GpsFixData fix_data{
        .has_fix = true,
        .latitude = 48.1351,
        .longitude = 11.5820,
        .altitude_m = 520.0f,
        .hdop = 1.1f,
        .speed_kmh = 0.0f,
        .satellites_in_use = 9
    };

    void power_on() override { powered = true; }
    void power_off() override { powered = false; }
    bool is_powered() const override { return powered; }
    bool has_valid_fix() const override { return valid_fix; }
    adapters::GpsFixData get_fix_data() const override { return fix_data; }
    void update() override {}
};

class MockImu : public interfaces::IImu {
public:
    bool motion_flag{false};
    adapters::EulerAngles orientation{.roll = 1.5f, .pitch = -2.0f, .yaw = 180.0f};

    bool init() override { return true; }
    adapters::EulerAngles read_orientation() override { return orientation; }
    bool has_motion_detected() override { return motion_flag; }
    void clear_motion_flag() override { motion_flag = false; }
};

class MockBattery : public interfaces::IBattery {
public:
    uint16_t voltage_mv{3750};

    uint16_t read_voltage_mv() override { return voltage_mv; }
};

class MockLoRaWan : public interfaces::ILoRaWan {
public:
    bool joined{true};
    std::vector<std::vector<uint8_t>> sent_packets;
    DownlinkCallback downlink_cb;

    bool is_joined() const override { return joined; }
    bool join() override { joined = true; return true; }
    bool send(uint8_t /*port*/, std::span<const uint8_t> payload, bool /*confirmed*/) override {
        sent_packets.emplace_back(payload.begin(), payload.end());
        return true;
    }
    void set_downlink_callback(DownlinkCallback cb) override { downlink_cb = cb; }
    void process() override {}

    void inject_downlink(uint8_t port, std::span<const uint8_t> payload) {
        if (downlink_cb) {
            downlink_cb(port, payload);
        }
    }
};

class MockStorage : public interfaces::IStorage {
public:
    core::TrackerConfig config_storage{};
    core::LoRaCredentials creds_storage{};
    bool has_saved_config{false};
    bool factory_reset_called{false};

    bool load_config(core::TrackerConfig& config) override {
        if (!has_saved_config) return false;
        config = config_storage;
        return true;
    }
    bool save_config(const core::TrackerConfig& config) override {
        config_storage = config;
        has_saved_config = true;
        return true;
    }
    bool load_credentials(core::LoRaCredentials& creds) override {
        creds = creds_storage;
        return true;
    }
    bool save_credentials(const core::LoRaCredentials& creds) override {
        creds_storage = creds;
        return true;
    }
    bool factory_reset() override {
        factory_reset_called = true;
        has_saved_config = false;
        return true;
    }
};

class MockTimerService : public interfaces::ITimerService {
public:
    uint32_t current_time_ms{0};

    uint32_t get_time_ms() const override { return current_time_ms; }
    void delay_ms(uint32_t ms) override { current_time_ms += ms; }
    void sleep_low_power_ms(uint32_t ms) override { current_time_ms += ms; }

    void advance_seconds(uint32_t sec) { current_time_ms += (sec * 1000); }
};

class MockUi : public interfaces::IUi {
public:
    ButtonCallback button_cb;
    bool red_led{false};
    bool green_led{false};
    bool blue_led{false};
    uint32_t red_blinks{0};
    uint32_t green_blinks{0};
    uint32_t blue_blinks{0};

    void set_led(interfaces::LedColor color, bool on) override {
        switch (color) {
            case interfaces::LedColor::Red: red_led = on; break;
            case interfaces::LedColor::Green: green_led = on; break;
            case interfaces::LedColor::Blue: blue_led = on; break;
        }
    }

    void blink_led(interfaces::LedColor color, uint16_t /*on_ms*/) override {
        switch (color) {
            case interfaces::LedColor::Red: red_blinks++; break;
            case interfaces::LedColor::Green: green_blinks++; break;
            case interfaces::LedColor::Blue: blue_blinks++; break;
        }
    }

    void set_alarm_button_callback(ButtonCallback cb) override {
        button_cb = cb;
    }

    void press_alarm_button() {
        if (button_cb) button_cb();
    }
};

} // namespace lgt92::tests

