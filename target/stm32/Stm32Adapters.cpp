#include "Stm32Adapters.hpp"

namespace lgt92::stm32 {

// GPS
void Stm32Gps::power_on() {
    powered_ = true;
    // Power on GPS hardware pin (e.g. GPS_POWER_ON)
}

void Stm32Gps::power_off() {
    powered_ = false;
    // Power off GPS hardware pin (GPS_POWER_OFF)
}

bool Stm32Gps::is_powered() const {
    return powered_;
}

bool Stm32Gps::has_valid_fix() const {
    return parser_.has_valid_fix();
}

adapters::GpsFixData Stm32Gps::get_fix_data() const {
    return parser_.get_fix_data();
}

void Stm32Gps::update() {
    // Read UART DMA/RingBuffer and feed to parser_
}

// IMU
bool Stm32Imu::init() {
    ahrs_.reset();
    return true;
}

adapters::EulerAngles Stm32Imu::read_orientation() {
    // In real hardware, read MPU9250 accel/gyro registers over I2C and update ahrs_
    return ahrs_.get_euler_angles();
}

bool Stm32Imu::has_motion_detected() {
    return motion_flag_;
}

void Stm32Imu::clear_motion_flag() {
    motion_flag_ = false;
}

// Battery
uint16_t Stm32Battery::read_voltage_mv() {
    // ADC measurement of VREF / VBAT divider
    return 3700;
}

// LoRaWAN
bool Stm32LoRaWan::is_joined() const {
    return true;
}

bool Stm32LoRaWan::join() {
    return true;
}

bool Stm32LoRaWan::send(uint8_t /*port*/, std::span<const uint8_t> /*payload*/, bool /*confirmed*/) {
    return true;
}

void Stm32LoRaWan::set_downlink_callback(DownlinkCallback cb) {
    downlink_cb_ = cb;
}

void Stm32LoRaWan::process() {
    // Process LoRaMac timer/radio state machine
}

// Storage
bool Stm32Storage::load_config(core::TrackerConfig& config) {
    config.reset_to_defaults();
    return true;
}

bool Stm32Storage::save_config(const core::TrackerConfig& /*config*/) {
    return true;
}

bool Stm32Storage::load_credentials(core::LoRaCredentials& /*creds*/) {
    return true;
}

bool Stm32Storage::save_credentials(const core::LoRaCredentials& /*creds*/) {
    return true;
}

bool Stm32Storage::factory_reset() {
    return true;
}

// Timer
uint32_t Stm32TimerService::get_time_ms() const {
    return 0;
}

void Stm32TimerService::delay_ms(uint32_t /*ms*/) {
}

void Stm32TimerService::sleep_low_power_ms(uint32_t /*ms*/) {
}

// UI
void Stm32Ui::set_led(interfaces::LedColor /*color*/, bool /*on*/) {
}

void Stm32Ui::blink_led(interfaces::LedColor /*color*/, uint16_t /*on_ms*/) {
}

void Stm32Ui::set_alarm_button_callback(ButtonCallback cb) {
    button_cb_ = cb;
}

void Stm32Ui::handle_button_irq() {
    if (button_cb_) {
        button_cb_();
    }
}

} // namespace lgt92::stm32

