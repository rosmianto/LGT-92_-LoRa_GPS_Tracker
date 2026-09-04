#include "core/DownlinkParser.hpp"
#include <algorithm>

namespace lgt92::core {

std::optional<DownlinkCommand> DownlinkParser::parse(std::span<const uint8_t> buffer) noexcept {
    if (buffer.empty()) {
        return std::nullopt;
    }

    uint8_t opcode = buffer[0];

    switch (opcode) {
        case 0x01: { // Set Transmit Interval (TDC): 4 bytes total [0x01, b1, b2, b3]
            if (buffer.size() < 4) return std::nullopt;
            uint32_t seconds = (static_cast<uint32_t>(buffer[1]) << 16) |
                               (static_cast<uint32_t>(buffer[2]) << 8)  |
                               (static_cast<uint32_t>(buffer[3]));
            // LGT-92 minimum TDC is 6 seconds
            if (seconds < 6) seconds = 6;
            return downlink::SetTransmitInterval{seconds};
        }

        case 0x04: { // System Reset or Factory Reset: 2 bytes [0x04, subcode]
            if (buffer.size() < 2) return std::nullopt;
            if (buffer[1] == 0xFF) {
                return downlink::SystemReset{};
            } else if (buffer[1] == 0xFE) {
                return downlink::FactoryReset{};
            }
            return downlink::UnknownCommand{opcode};
        }

        case 0x05: { // Confirmed Uplinks (CFM): 2 bytes [0x05, 0x01/0x00]
            if (buffer.size() < 2) return std::nullopt;
            return downlink::SetConfirmedUplinks{buffer[1] == 0x01};
        }

        case 0x02: { // Clear Alarm: 2 bytes [0x02, 0x01]
            if (buffer.size() < 2) return std::nullopt;
            if (buffer[1] == 0x01) {
                return downlink::ClearAlarm{};
            }
            return downlink::UnknownCommand{opcode};
        }

        case 0x20: { // Set GPS Positioning Time: 2 bytes [0x20, seconds]
            if (buffer.size() < 2) return std::nullopt;
            return downlink::SetGpsPositioningTime{buffer[1]};
        }

        case 0x21: { // Set Motion Detection Mode: 2 bytes [0x21, mode]
            if (buffer.size() < 2) return std::nullopt;
            return downlink::SetMotionDetectionMode{buffer[1]};
        }

        case 0x23: { // Set Motion Threshold: 2 bytes [0x23, threshold]
            if (buffer.size() < 2) return std::nullopt;
            return downlink::SetMotionThreshold{buffer[1]};
        }

        case 0x24: { // Set Keep-alive Interval: 4 bytes [0x24, b1, b2, b3]
            if (buffer.size() < 4) return std::nullopt;
            uint32_t seconds = (static_cast<uint32_t>(buffer[1]) << 16) |
                               (static_cast<uint32_t>(buffer[2]) << 8)  |
                               (static_cast<uint32_t>(buffer[3]));
            return downlink::SetKeepAliveInterval{seconds};
        }

        case 0x25: { // Set Payload Format: 2 bytes [0x25, format]
            if (buffer.size() < 2) return std::nullopt;
            return downlink::SetPayloadFormat{buffer[1]};
        }

        default:
            return downlink::UnknownCommand{opcode};
    }
}

DownlinkExecutionResult DownlinkParser::execute(const DownlinkCommand& cmd, TrackerConfig& config) noexcept {
    DownlinkExecutionResult result{};

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, downlink::SetTransmitInterval>) {
            config.transmit_interval_s = arg.interval_s;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::SystemReset>) {
            result.reset_requested = true;
        } else if constexpr (std::is_same_v<T, downlink::FactoryReset>) {
            config.reset_to_defaults();
            result.config_modified = true;
            result.factory_reset_requested = true;
            result.reset_requested = true;
        } else if constexpr (std::is_same_v<T, downlink::SetConfirmedUplinks>) {
            config.confirmed_uplinks = arg.confirmed;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::ClearAlarm>) {
            result.alarm_cleared = true;
        } else if constexpr (std::is_same_v<T, downlink::SetGpsPositioningTime>) {
            config.gps_positioning_time_s = arg.timeout_s;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::SetMotionDetectionMode>) {
            config.motion_detection_mode = arg.mode;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::SetMotionThreshold>) {
            config.motion_threshold = arg.threshold;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::SetKeepAliveInterval>) {
            config.keep_alive_interval_s = arg.interval_s;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::SetPayloadFormat>) {
            config.payload_format = arg.format;
            result.config_modified = true;
        } else if constexpr (std::is_same_v<T, downlink::UnknownCommand>) {
            // No action
        }
    }, cmd);

    return result;
}

} // namespace lgt92::core

