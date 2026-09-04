#pragma once

#include "adapters/LwGpsAdapter.hpp"

namespace lgt92::interfaces {

class IGps {
public:
    virtual ~IGps() = default;

    virtual void power_on() = 0;
    virtual void power_off() = 0;
    virtual bool is_powered() const = 0;
    virtual bool has_valid_fix() const = 0;
    virtual adapters::GpsFixData get_fix_data() const = 0;
    virtual void update() = 0; // Poll or process incoming bytes
};

} // namespace lgt92::interfaces

