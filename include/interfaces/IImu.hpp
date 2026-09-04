#pragma once

#include "adapters/FusionAdapter.hpp"

namespace lgt92::interfaces {

class IImu {
public:
    virtual ~IImu() = default;

    virtual bool init() = 0;
    virtual adapters::EulerAngles read_orientation() = 0;
    virtual bool has_motion_detected() = 0;
    virtual void clear_motion_flag() = 0;
};

} // namespace lgt92::interfaces

