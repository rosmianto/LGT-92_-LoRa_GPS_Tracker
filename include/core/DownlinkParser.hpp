#pragma once

#include "core/TrackerConfig.hpp"
#include <span>
#include <cstdint>
#include <variant>
#include <optional>

namespace lgt92::core {

namespace downlink {

struct SetTransmitInterval {
    uint32_t interval_s;
};

struct SystemReset {};

struct FactoryReset {};

struct SetConfirmedUplinks {
    bool confirmed;
};

struct ClearAlarm {};

struct SetGpsPositioningTime {
    uint16_t timeout_s;
};

struct SetMotionDetectionMode {
    uint8_t mode;
};

struct SetMotionThreshold {
    uint8_t threshold;
};

struct SetKeepAliveInterval {
    uint32_t interval_s;
};

struct SetPayloadFormat {
    uint8_t format;
};

struct UnknownCommand {
    uint8_t opcode;
};

} // namespace downlink

using DownlinkCommand = std::variant<
    downlink::SetTransmitInterval,
    downlink::SystemReset,
    downlink::FactoryReset,
    downlink::SetConfirmedUplinks,
    downlink::ClearAlarm,
    downlink::SetGpsPositioningTime,
    downlink::SetMotionDetectionMode,
    downlink::SetMotionThreshold,
    downlink::SetKeepAliveInterval,
    downlink::SetPayloadFormat,
    downlink::UnknownCommand
>;

struct DownlinkExecutionResult {
    bool config_modified{false};
    bool reset_requested{false};
    bool factory_reset_requested{false};
    bool alarm_cleared{false};
};

class DownlinkParser {
public:
    static std::optional<DownlinkCommand> parse(std::span<const uint8_t> buffer) noexcept;
    static DownlinkExecutionResult execute(const DownlinkCommand& cmd, TrackerConfig& config) noexcept;
};

} // namespace lgt92::core

