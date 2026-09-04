#pragma once

#include "core/TrackerConfig.hpp"
#include "core/LoRaCredentials.hpp"
#include "core/Payload.hpp"
#include "core/DownlinkParser.hpp"
#include "interfaces/IGps.hpp"
#include "interfaces/IImu.hpp"
#include "interfaces/IBattery.hpp"
#include "interfaces/ILoRaWan.hpp"
#include "interfaces/IStorage.hpp"
#include "interfaces/ITimerService.hpp"
#include "interfaces/IUi.hpp"

namespace lgt92::core {

enum class TrackerState {
    Uninitialized,
    Joining,
    Idle,
    AcquiringSensors,
    Transmitting,
    Sleeping
};

class TrackerEngine {
public:
    TrackerEngine(
        interfaces::IGps& gps,
        interfaces::IImu& imu,
        interfaces::IBattery& battery,
        interfaces::ILoRaWan& lora,
        interfaces::IStorage& storage,
        interfaces::ITimerService& timer,
        interfaces::IUi& ui
    );

    ~TrackerEngine() = default;

    bool init();
    void step();

    // Asynchronous interrupt event handlers
    void on_alarm_button_pressed();
    void on_motion_detected();

    // Accessors for state inspection (for tests, simulator, and debug)
    [[nodiscard]] TrackerState get_state() const noexcept { return state_; }
    [[nodiscard]] const TrackerConfig& get_config() const noexcept { return config_; }
    [[nodiscard]] TrackerConfig& get_config() noexcept { return config_; }
    [[nodiscard]] bool is_alarm_active() const noexcept { return alarm_active_; }
    [[nodiscard]] uint32_t get_uplink_count() const noexcept { return uplink_count_; }

private:
    void handle_downlink(uint8_t port, std::span<const uint8_t> payload);
    void acquire_sensors_and_transmit();

    interfaces::IGps&         gps_;
    interfaces::IImu&         imu_;
    interfaces::IBattery&     battery_;
    interfaces::ILoRaWan&     lora_;
    interfaces::IStorage&     storage_;
    interfaces::ITimerService& timer_;
    interfaces::IUi&          ui_;

    TrackerConfig   config_{};
    LoRaCredentials credentials_{};
    TrackerState    state_{TrackerState::Uninitialized};

    bool alarm_active_{false};
    bool motion_detected_{false};
    bool immediate_tx_requested_{false};
    uint32_t last_tx_time_ms_{0};
    uint32_t uplink_count_{0};
};

} // namespace lgt92::core

