#include "core/Payload.hpp"
#include <cmath>
#include <algorithm>

namespace lgt92::core {

bool PayloadEncoder::encode_startup(std::span<uint8_t> out_buffer) noexcept {
    if (out_buffer.empty()) return false;
    out_buffer[0] = 0x11;
    return true;
}

size_t PayloadEncoder::encode(const UplinkData& data, std::span<uint8_t> out_buffer) noexcept {
    size_t required_size = data.full_payload ? FULL_PAYLOAD_SIZE : SHORT_PAYLOAD_SIZE;
    if (out_buffer.size() < required_size) {
        return 0;
    }

    // 1. Latitude and Longitude (Bytes 0..7)
    if (data.gps_fix == GpsFixState::Off) {
        for (size_t i = 0; i < 8; ++i) {
            out_buffer[i] = 0x00;
        }
    } else if (data.gps_fix == GpsFixState::NoFix) {
        for (size_t i = 0; i < 8; ++i) {
            out_buffer[i] = 0xFF;
        }
    } else {
        auto lat_val = static_cast<int32_t>(std::round(data.latitude * 1'000'000.0));
        auto lon_val = static_cast<int32_t>(std::round(data.longitude * 1'000'000.0));

        out_buffer[0] = static_cast<uint8_t>((lat_val >> 24) & 0xFF);
        out_buffer[1] = static_cast<uint8_t>((lat_val >> 16) & 0xFF);
        out_buffer[2] = static_cast<uint8_t>((lat_val >> 8) & 0xFF);
        out_buffer[3] = static_cast<uint8_t>(lat_val & 0xFF);

        out_buffer[4] = static_cast<uint8_t>((lon_val >> 24) & 0xFF);
        out_buffer[5] = static_cast<uint8_t>((lon_val >> 16) & 0xFF);
        out_buffer[6] = static_cast<uint8_t>((lon_val >> 8) & 0xFF);
        out_buffer[7] = static_cast<uint8_t>(lon_val & 0xFF);
    }

    // 2. Battery Voltage & Alarm Flag (Bytes 8..9)
    uint16_t bat_field = static_cast<uint16_t>(data.battery_mv & 0x3FFF);
    if (data.alarm) {
        bat_field |= 0x4000; // Bit 14 (bit 6 of MSB) indicates ALARM
    }
    out_buffer[8] = static_cast<uint8_t>((bat_field >> 8) & 0xFF);
    out_buffer[9] = static_cast<uint8_t>(bat_field & 0xFF);

    // 3. Flags (Byte 10)
    uint8_t mode_val = static_cast<uint8_t>(data.motion_mode);
    uint8_t flag = static_cast<uint8_t>((mode_val << 6) |
                                        (data.led_on ? (1 << 5) : 0) |
                                        (data.firmware_version & 0x1F));
    out_buffer[10] = flag;

    // 4. Extended IMU & GPS fields (Bytes 11..17)
    if (data.full_payload) {
        auto roll_val = static_cast<int16_t>(std::round(data.roll * 100.0f));
        auto pitch_val = static_cast<int16_t>(std::round(data.pitch * 100.0f));
        auto hdop_val = static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(data.hdop * 100.0f)), 0, 255));
        auto alt_val = static_cast<int16_t>(std::round(data.altitude_m * 100.0f));

        out_buffer[11] = static_cast<uint8_t>((roll_val >> 8) & 0xFF);
        out_buffer[12] = static_cast<uint8_t>(roll_val & 0xFF);

        out_buffer[13] = static_cast<uint8_t>((pitch_val >> 8) & 0xFF);
        out_buffer[14] = static_cast<uint8_t>(pitch_val & 0xFF);

        out_buffer[15] = hdop_val;

        out_buffer[16] = static_cast<uint8_t>((alt_val >> 8) & 0xFF);
        out_buffer[17] = static_cast<uint8_t>(alt_val & 0xFF);
    }

    return required_size;
}

std::optional<UplinkData> PayloadEncoder::decode(std::span<const uint8_t> in_buffer) noexcept {
    if (in_buffer.size() != FULL_PAYLOAD_SIZE && in_buffer.size() != SHORT_PAYLOAD_SIZE) {
        return std::nullopt;
    }

    UplinkData data{};
    data.full_payload = (in_buffer.size() == FULL_PAYLOAD_SIZE);

    // 1. Latitude / Longitude
    bool all_zero = true;
    bool all_ff = true;
    for (size_t i = 0; i < 8; ++i) {
        if (in_buffer[i] != 0x00) all_zero = false;
        if (in_buffer[i] != 0xFF) all_ff = false;
    }

    if (all_zero) {
        data.gps_fix = GpsFixState::Off;
        data.latitude = 0.0;
        data.longitude = 0.0;
    } else if (all_ff) {
        data.gps_fix = GpsFixState::NoFix;
        data.latitude = 0.0;
        data.longitude = 0.0;
    } else {
        data.gps_fix = GpsFixState::Valid;
        auto raw_lat = static_cast<int32_t>((static_cast<uint32_t>(in_buffer[0]) << 24) |
                                            (static_cast<uint32_t>(in_buffer[1]) << 16) |
                                            (static_cast<uint32_t>(in_buffer[2]) << 8)  |
                                            (static_cast<uint32_t>(in_buffer[3])));

        auto raw_lon = static_cast<int32_t>((static_cast<uint32_t>(in_buffer[4]) << 24) |
                                            (static_cast<uint32_t>(in_buffer[5]) << 16) |
                                            (static_cast<uint32_t>(in_buffer[6]) << 8)  |
                                            (static_cast<uint32_t>(in_buffer[7])));

        data.latitude = static_cast<double>(raw_lat) / 1'000'000.0;
        data.longitude = static_cast<double>(raw_lon) / 1'000'000.0;
    }

    // 2. Battery & Alarm
    uint16_t raw_bat = static_cast<uint16_t>((in_buffer[8] << 8) | in_buffer[9]);
    data.alarm = (raw_bat & 0x4000) != 0;
    data.battery_mv = static_cast<uint16_t>(raw_bat & 0x3FFF);

    // 3. Flags
    uint8_t flags = in_buffer[10];
    data.motion_mode = static_cast<MotionMode>((flags >> 6) & 0x03);
    data.led_on = (flags & (1 << 5)) != 0;
    data.firmware_version = static_cast<uint8_t>(flags & 0x1F);

    // 4. Full payload fields
    if (data.full_payload) {
        auto raw_roll = static_cast<int16_t>((in_buffer[11] << 8) | in_buffer[12]);
        auto raw_pitch = static_cast<int16_t>((in_buffer[13] << 8) | in_buffer[14]);
        uint8_t raw_hdop = in_buffer[15];
        auto raw_alt = static_cast<int16_t>((in_buffer[16] << 8) | in_buffer[17]);

        data.roll = static_cast<float>(raw_roll) / 100.0f;
        data.pitch = static_cast<float>(raw_pitch) / 100.0f;
        data.hdop = static_cast<float>(raw_hdop) / 100.0f;
        data.altitude_m = static_cast<float>(raw_alt) / 100.0f;
    }

    return data;
}

} // namespace lgt92::core

