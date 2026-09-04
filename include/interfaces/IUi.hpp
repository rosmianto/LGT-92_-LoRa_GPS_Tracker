#pragma once

#include <cstdint>
#include <functional>

namespace lgt92::interfaces {

enum class LedColor {
    Red,
    Green,
    Blue
};

class IUi {
public:
    virtual ~IUi() = default;

    using ButtonCallback = std::function<void()>;

    virtual void set_led(LedColor color, bool on) = 0;
    virtual void blink_led(LedColor color, uint16_t on_ms) = 0;
    virtual void set_alarm_button_callback(ButtonCallback cb) = 0;
};

} // namespace lgt92::interfaces

