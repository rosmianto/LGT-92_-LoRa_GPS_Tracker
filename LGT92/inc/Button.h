#pragma once

// TODO: Detect Button press event within 5s window
enum class ButtonEvent {
    Click_long,
    Click_1time,
    Click_2times,
    Click_3times,
    Click_4times,
    Click_5times
};

class Button {

public:
    bool init() { return true; }
    ButtonEvent getButtonEvent() { return _event; }

private:
    ButtonEvent _event;
};