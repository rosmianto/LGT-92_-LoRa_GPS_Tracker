#pragma once

#include <span>
#include <cstdint>
#include <functional>

namespace lgt92::interfaces {

class ILoRaWan {
public:
    virtual ~ILoRaWan() = default;

    using DownlinkCallback = std::function<void(uint8_t port, std::span<const uint8_t> payload)>;

    virtual bool is_joined() const = 0;
    virtual bool join() = 0;
    virtual bool send(uint8_t port, std::span<const uint8_t> payload, bool confirmed) = 0;
    virtual void set_downlink_callback(DownlinkCallback cb) = 0;
    virtual void process() = 0; // Process radio/mac events
};

} // namespace lgt92::interfaces

