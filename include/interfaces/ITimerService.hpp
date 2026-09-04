#pragma once

#include <cstdint>

namespace lgt92::interfaces {

class ITimerService {
public:
    virtual ~ITimerService() = default;

    virtual uint32_t get_time_ms() const = 0;
    virtual void delay_ms(uint32_t ms) = 0;
    virtual void sleep_low_power_ms(uint32_t ms) = 0;
};

} // namespace lgt92::interfaces

