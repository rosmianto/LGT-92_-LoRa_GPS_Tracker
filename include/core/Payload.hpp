#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include <optional>

namespace lgt92::core {

enum class GpsFixState : uint8_t {
    Off = 0,     // GPS disabled / off (0x00000000)
    NoFix = 1,   // GPS on but no satellite fix (0xFFFFFFFF)
    Valid = 2    // 2D/3D fix with valid coordinates
};

enum class MotionMode : uint8_t {
    Disabled = 0,
    Movement = 1,
    Collision = 2
};

struct UplinkData {
    GpsFixState gps_fix{GpsFixState::NoFix};
    double      latitude{0.0};       // Degrees (positive = North, negative = South)
    double      longitude{0.0};      // Degrees (positive = East, negative = West)
    uint16_t    battery_mv{3600};    // Millivolts (e.g. 3600 = 3.6V)
    bool        alarm{false};        // SOS / Alarm button triggered
    MotionMode  motion_mode{MotionMode::Movement};
    bool        led_on{true};
    uint8_t     firmware_version{0x04};

    // Extended fields (present when full_payload is true)
    bool        full_payload{true};  // 18 bytes if true, 11 bytes if false
    float       roll{0.0f};          // Degrees (-180.0 to +180.0)
    float       pitch{0.0f};         // Degrees (-90.0 to +90.0)
    float       hdop{1.0f};          // Horizontal dilution of precision
    float       altitude_m{0.0f};    // Altitude in meters
};

class PayloadEncoder {
public:
    static constexpr size_t FULL_PAYLOAD_SIZE = 18;
    static constexpr size_t SHORT_PAYLOAD_SIZE = 11;
    static constexpr size_t STARTUP_PAYLOAD_SIZE = 1;

    // Encodes startup beacon (0x11)
    static bool encode_startup(std::span<uint8_t> out_buffer) noexcept;

    // Encodes tracking telemetry
    static size_t encode(const UplinkData& data, std::span<uint8_t> out_buffer) noexcept;

    // Decodes telemetry from raw bytes (useful for unit tests and gateway simulators)
    static std::optional<UplinkData> decode(std::span<const uint8_t> in_buffer) noexcept;
};

} // namespace lgt92::core

