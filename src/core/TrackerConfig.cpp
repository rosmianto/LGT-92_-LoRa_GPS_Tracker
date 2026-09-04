#include "core/TrackerConfig.hpp"
#include <cstring>

namespace lgt92::core {

#pragma pack(push, 1)
struct SerializedConfigHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t payload_len;
};

struct SerializedConfigPayload {
    uint32_t transmit_interval_s;
    uint32_t alarm_interval_s;
    uint32_t keep_alive_interval_s;
    uint16_t gps_positioning_time_s;
    uint8_t  motion_detection_mode;
    uint8_t  motion_threshold;
    uint8_t  payload_format;
    uint8_t  confirmed_uplinks;
    uint8_t  led_enabled;
    uint8_t  lora_port;
    uint8_t  adr_enabled;
    uint8_t  tx_data_rate;
};

struct SerializedConfigFooter {
    uint16_t crc16;
};
#pragma pack(pop)

void TrackerConfig::reset_to_defaults() noexcept {
    *this = TrackerConfig{};
}

size_t TrackerConfig::serialized_size() const noexcept {
    return sizeof(SerializedConfigHeader) + sizeof(SerializedConfigPayload) + sizeof(SerializedConfigFooter);
}

uint16_t TrackerConfig::calculate_crc16(std::span<const uint8_t> data) noexcept {
    uint16_t crc = 0xFFFF;
    for (uint8_t byte : data) {
        crc ^= static_cast<uint16_t>(byte << 8);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
            } else {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
    }
    return crc;
}

bool TrackerConfig::serialize(std::span<uint8_t> out_buffer) const noexcept {
    if (out_buffer.size() < serialized_size()) {
        return false;
    }

    SerializedConfigHeader header{
        .magic = CONFIG_MAGIC,
        .version = CONFIG_VERSION,
        .payload_len = sizeof(SerializedConfigPayload)
    };

    SerializedConfigPayload payload{
        .transmit_interval_s = transmit_interval_s,
        .alarm_interval_s = alarm_interval_s,
        .keep_alive_interval_s = keep_alive_interval_s,
        .gps_positioning_time_s = gps_positioning_time_s,
        .motion_detection_mode = motion_detection_mode,
        .motion_threshold = motion_threshold,
        .payload_format = payload_format,
        .confirmed_uplinks = static_cast<uint8_t>(confirmed_uplinks ? 1 : 0),
        .led_enabled = static_cast<uint8_t>(led_enabled ? 1 : 0),
        .lora_port = lora_port,
        .adr_enabled = static_cast<uint8_t>(adr_enabled ? 1 : 0),
        .tx_data_rate = tx_data_rate
    };

    std::memcpy(out_buffer.data(), &header, sizeof(header));
    std::memcpy(out_buffer.data() + sizeof(header), &payload, sizeof(payload));

    // Calculate CRC over Header + Payload
    size_t data_len = sizeof(header) + sizeof(payload);
    uint16_t crc = calculate_crc16(out_buffer.subspan(0, data_len));

    SerializedConfigFooter footer{.crc16 = crc};
    std::memcpy(out_buffer.data() + data_len, &footer, sizeof(footer));

    return true;
}

bool TrackerConfig::deserialize(std::span<const uint8_t> in_buffer) noexcept {
    if (in_buffer.size() < serialized_size()) {
        return false;
    }

    SerializedConfigHeader header{};
    std::memcpy(&header, in_buffer.data(), sizeof(header));

    if (header.magic != CONFIG_MAGIC || header.version != CONFIG_VERSION || header.payload_len != sizeof(SerializedConfigPayload)) {
        return false;
    }

    size_t data_len = sizeof(header) + sizeof(SerializedConfigPayload);
    uint16_t expected_crc = calculate_crc16(in_buffer.subspan(0, data_len));

    SerializedConfigFooter footer{};
    std::memcpy(&footer, in_buffer.data() + data_len, sizeof(footer));

    if (footer.crc16 != expected_crc) {
        return false;
    }

    SerializedConfigPayload payload{};
    std::memcpy(&payload, in_buffer.data() + sizeof(header), sizeof(payload));

    transmit_interval_s    = payload.transmit_interval_s;
    alarm_interval_s       = payload.alarm_interval_s;
    keep_alive_interval_s  = payload.keep_alive_interval_s;
    gps_positioning_time_s = payload.gps_positioning_time_s;
    motion_detection_mode  = payload.motion_detection_mode;
    motion_threshold       = payload.motion_threshold;
    payload_format         = payload.payload_format;
    confirmed_uplinks      = (payload.confirmed_uplinks != 0);
    led_enabled            = (payload.led_enabled != 0);
    lora_port              = payload.lora_port;
    adr_enabled            = (payload.adr_enabled != 0);
    tx_data_rate           = payload.tx_data_rate;

    return true;
}

} // namespace lgt92::core

