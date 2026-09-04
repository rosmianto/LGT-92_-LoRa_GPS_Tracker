#include "core/TrackerEngine.hpp"
#include <array>

namespace lgt92::core {

TrackerEngine::TrackerEngine(
    interfaces::IGps& gps,
    interfaces::IImu& imu,
    interfaces::IBattery& battery,
    interfaces::ILoRaWan& lora,
    interfaces::IStorage& storage,
    interfaces::ITimerService& timer,
    interfaces::IUi& ui
) : gps_(gps),
    imu_(imu),
    battery_(battery),
    lora_(lora),
    storage_(storage),
    timer_(timer),
    ui_(ui) {}

bool TrackerEngine::init() {
    // 1. Load persistent configuration from NVS
    if (!storage_.load_config(config_)) {
        config_.reset_to_defaults();
        storage_.save_config(config_);
    }

    // 2. Load persistent LoRaWAN credentials
    storage_.load_credentials(credentials_);

    // 3. Register callbacks
    lora_.set_downlink_callback([this](uint8_t port, std::span<const uint8_t> payload) {
        handle_downlink(port, payload);
    });

    ui_.set_alarm_button_callback([this]() {
        on_alarm_button_pressed();
    });

    // 4. Initialize IMU
    imu_.init();

    // 5. Initiate LoRaWAN Join
    state_ = TrackerState::Joining;
    if (!lora_.is_joined()) {
        lora_.join();
    }

    if (lora_.is_joined()) {
        // Send initial startup frame (0x11)
        std::array<uint8_t, 4> startup_buf{};
        PayloadEncoder::encode_startup(startup_buf);
        lora_.send(config_.lora_port, std::span<const uint8_t>(startup_buf.data(), 1), false);

        if (config_.led_enabled) {
            ui_.blink_led(interfaces::LedColor::Green, 500);
        }
        state_ = TrackerState::Idle;
    }

    last_tx_time_ms_ = timer_.get_time_ms();
    return true;
}

void TrackerEngine::on_alarm_button_pressed() {
    alarm_active_ = true;
    immediate_tx_requested_ = true;
    if (config_.led_enabled) {
        ui_.blink_led(interfaces::LedColor::Red, 1000);
    }
}

void TrackerEngine::on_motion_detected() {
    motion_detected_ = true;
    if (config_.motion_detection_mode != 0) {
        immediate_tx_requested_ = true;
    }
}

void TrackerEngine::handle_downlink(uint8_t /*port*/, std::span<const uint8_t> payload) {
    auto cmd = DownlinkParser::parse(payload);
    if (!cmd) return;

    auto res = DownlinkParser::execute(*cmd, config_);

    if (res.config_modified) {
        storage_.save_config(config_);
    }

    if (res.alarm_cleared) {
        alarm_active_ = false;
    }

    if (res.reset_requested) {
        if (res.factory_reset_requested) {
            storage_.factory_reset();
        }
        // Re-initialize state
        init();
    }

    if (config_.led_enabled) {
        ui_.blink_led(interfaces::LedColor::Blue, 200);
    }
}

void TrackerEngine::acquire_sensors_and_transmit() {
    state_ = TrackerState::AcquiringSensors;

    // 1. Acquire GPS position
    gps_.power_on();
    uint32_t start_gps_ms = timer_.get_time_ms();
    uint32_t timeout_ms = static_cast<uint32_t>(config_.gps_positioning_time_s) * 1000;

    while (!gps_.has_valid_fix() && (timer_.get_time_ms() - start_gps_ms < timeout_ms)) {
        gps_.update();
        timer_.delay_ms(50);
    }

    adapters::GpsFixData fix_data = gps_.get_fix_data();
    gps_.power_off();

    // 2. Read IMU orientation and clear motion trigger
    adapters::EulerAngles orientation = imu_.read_orientation();
    if (imu_.has_motion_detected()) {
        motion_detected_ = true;
        imu_.clear_motion_flag();
    }

    // 3. Read Battery Voltage
    uint16_t bat_mv = battery_.read_voltage_mv();

    // 4. Assemble Uplink Telemetry
    UplinkData uplink{};
    uplink.full_payload = (config_.payload_format == 0);
    uplink.gps_fix = fix_data.has_fix ? GpsFixState::Valid : GpsFixState::NoFix;
    uplink.latitude = fix_data.latitude;
    uplink.longitude = fix_data.longitude;
    uplink.battery_mv = bat_mv;
    uplink.alarm = alarm_active_;
    uplink.motion_mode = static_cast<MotionMode>(config_.motion_detection_mode);
    uplink.led_on = config_.led_enabled;
    uplink.firmware_version = 0x04;
    uplink.roll = orientation.roll;
    uplink.pitch = orientation.pitch;
    uplink.hdop = fix_data.hdop;
    uplink.altitude_m = fix_data.altitude_m;

    // 5. Encode and Transmit Frame
    state_ = TrackerState::Transmitting;
    std::array<uint8_t, 32> buffer{};
    size_t payload_len = PayloadEncoder::encode(uplink, buffer);

    if (payload_len > 0) {
        lora_.send(
            config_.lora_port,
            std::span<const uint8_t>(buffer.data(), payload_len),
            config_.confirmed_uplinks
        );
        uplink_count_++;

        if (config_.led_enabled) {
            ui_.blink_led(interfaces::LedColor::Blue, 150);
        }
    }

    last_tx_time_ms_ = timer_.get_time_ms();
    immediate_tx_requested_ = false;
    state_ = TrackerState::Idle;
}

void TrackerEngine::step() {
    lora_.process();

    if (state_ == TrackerState::Joining) {
        if (lora_.is_joined()) {
            state_ = TrackerState::Idle;
            std::array<uint8_t, 4> startup_buf{};
            PayloadEncoder::encode_startup(startup_buf);
            lora_.send(config_.lora_port, std::span<const uint8_t>(startup_buf.data(), 1), false);
        }
        return;
    }

    // Determine current transmit interval based on mode
    uint32_t interval_s = config_.transmit_interval_s;
    if (alarm_active_) {
        interval_s = config_.alarm_interval_s;
    } else if (!motion_detected_) {
        interval_s = config_.keep_alive_interval_s;
    }

    uint32_t elapsed_ms = timer_.get_time_ms() - last_tx_time_ms_;
    bool time_to_send = (elapsed_ms >= (interval_s * 1000));

    if (immediate_tx_requested_ || time_to_send) {
        acquire_sensors_and_transmit();
    }
}

} // namespace lgt92::core

