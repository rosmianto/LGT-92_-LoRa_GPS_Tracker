#pragma once

#include "interfaces/IGps.hpp"
#include "interfaces/IImu.hpp"
#include "interfaces/IBattery.hpp"
#include "interfaces/ILoRaWan.hpp"
#include "interfaces/IStorage.hpp"
#include "interfaces/ITimerService.hpp"
#include "interfaces/IUi.hpp"
#include <emscripten.h>
#include <vector>
#include <chrono>

namespace lgt92::sim {

class WasmGps : public interfaces::IGps {
public:
    bool powered_{false};
    bool valid_{true};
    adapters::GpsFixData fix_{
        .has_fix = true,
        .latitude = -6.2088,
        .longitude = 106.8456,
        .altitude_m = 15.0f,
        .hdop = 1.0f,
        .speed_kmh = 0.0f,
        .satellites_in_use = 10
    };

    void power_on() override { powered_ = true; }
    void power_off() override { powered_ = false; }
    bool is_powered() const override { return powered_; }
    bool has_valid_fix() const override { return valid_; }
    adapters::GpsFixData get_fix_data() const override { return fix_; }
    void update() override {}

    void set_coordinates(double lat, double lon, float alt) {
        fix_.latitude = lat;
        fix_.longitude = lon;
        fix_.altitude_m = alt;
        fix_.has_fix = true;
        valid_ = true;
    }

    void set_fix_valid(bool valid) {
        valid_ = valid;
        fix_.has_fix = valid;
    }
};

class WasmImu : public interfaces::IImu {
public:
    bool motion_flag_{false};
    adapters::EulerAngles orientation_{.roll = 0.0f, .pitch = 0.0f, .yaw = 0.0f};

    bool init() override { return true; }
    adapters::EulerAngles read_orientation() override { return orientation_; }
    bool has_motion_detected() override { return motion_flag_; }
    void clear_motion_flag() override { motion_flag_ = false; }

    void trigger_motion() { motion_flag_ = true; }
    void set_orientation(float roll, float pitch, float yaw) {
        orientation_.roll = roll;
        orientation_.pitch = pitch;
        orientation_.yaw = yaw;
    }
};

class WasmBattery : public interfaces::IBattery {
public:
    uint16_t voltage_mv_{3800};

    uint16_t read_voltage_mv() override { return voltage_mv_; }
    void set_voltage(uint16_t mv) { voltage_mv_ = mv; }
};

class WasmLoRaWan : public interfaces::ILoRaWan {
public:
    bool joined_{true};
    DownlinkCallback downlink_cb_{};

    bool is_joined() const override { return joined_; }
    bool join() override { joined_ = true; return true; }

    bool send(uint8_t port, std::span<const uint8_t> payload, bool confirmed) override;

    void set_downlink_callback(DownlinkCallback cb) override {
        downlink_cb_ = cb;
    }

    void process() override {}

    void inject_downlink(uint8_t port, std::span<const uint8_t> payload) {
        if (downlink_cb_) {
            downlink_cb_(port, payload);
        }
    }
};

class WasmStorage : public interfaces::IStorage {
public:
    core::TrackerConfig config_{};
    core::LoRaCredentials creds_{};
    bool config_loaded_{false};

    bool load_config(core::TrackerConfig& config) override {
        if (!config_loaded_) {
            config.reset_to_defaults();
            config_ = config;
            config_loaded_ = true;
        } else {
            config = config_;
        }
        return true;
    }

    bool save_config(const core::TrackerConfig& config) override {
        config_ = config;
        config_loaded_ = true;
        return true;
    }

    bool load_credentials(core::LoRaCredentials& creds) override {
        creds = creds_;
        return true;
    }

    bool save_credentials(const core::LoRaCredentials& creds) override {
        creds_ = creds;
        return true;
    }

    bool factory_reset() override {
        config_.reset_to_defaults();
        return true;
    }
};

class WasmTimer : public interfaces::ITimerService {
public:
    uint32_t get_time_ms() const override {
        return static_cast<uint32_t>(emscripten_get_now());
    }

    void delay_ms(uint32_t /*ms*/) override {}
    void sleep_low_power_ms(uint32_t /*ms*/) override {}
};

class WasmUi : public interfaces::IUi {
public:
    ButtonCallback button_cb_{};

    void set_led(interfaces::LedColor color, bool on) override;
    void blink_led(interfaces::LedColor color, uint16_t on_ms) override;

    void set_alarm_button_callback(ButtonCallback cb) override {
        button_cb_ = cb;
    }

    void press_alarm() {
        if (button_cb_) button_cb_();
    }
};

} // namespace lgt92::sim

