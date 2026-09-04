#pragma once

#include <span>
#include <cstdint>
#include <cstddef>

namespace lgt92::core {

struct TrackerConfig {
    static constexpr uint32_t CONFIG_MAGIC = 0x4C475432; // "LGT2"
    static constexpr uint16_t CONFIG_VERSION = 1;

    // Transmission timing (seconds)
    uint32_t transmit_interval_s{300};       // Default 5 minutes (min 6s)
    uint32_t alarm_interval_s{60};          // Alarm mode TX duty cycle
    uint32_t keep_alive_interval_s{21600};   // Stationary keep-alive (6h)
    uint16_t gps_positioning_time_s{150};    // Max GPS fix search time

    // Tracking & Sensor behavior
    uint8_t  motion_detection_mode{1};      // 0: Disabled, 1: Motion triggers TX, 2: Collision
    uint8_t  motion_threshold{10};          // Threshold for accelerometer trigger
    uint8_t  payload_format{0};             // 0: Full (GPS+IMU+HDOP), 1: Short (GPS only)
    bool     confirmed_uplinks{false};      // LoRaWAN confirmed uplinks (CFM)
    bool     led_enabled{true};             // LED blink indicators enabled

    // LoRaWAN radio parameters
    uint8_t  lora_port{2};                  // Default LGT-92 application port
    bool     adr_enabled{true};             // Adaptive Data Rate
    uint8_t  tx_data_rate{0};               // Default data rate when ADR disabled

    void reset_to_defaults() noexcept;

    // Serialization for Non-Volatile Storage (EEPROM / Flash)
    [[nodiscard]] size_t serialized_size() const noexcept;
    bool serialize(std::span<uint8_t> out_buffer) const noexcept;
    bool deserialize(std::span<const uint8_t> in_buffer) noexcept;

    static uint16_t calculate_crc16(std::span<const uint8_t> data) noexcept;
};

} // namespace lgt92::core

