#pragma once

#include <cstdint>

namespace lgt92::interfaces {

class IBattery {
public:
    virtual ~IBattery() = default;

    virtual uint16_t read_voltage_mv() = 0;
};

} // namespace lgt92::interfaces

