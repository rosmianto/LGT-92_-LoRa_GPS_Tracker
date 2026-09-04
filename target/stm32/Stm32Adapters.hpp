#pragma once

#include "interfaces/IGps.hpp"
#include "interfaces/IImu.hpp"
#include "interfaces/IBattery.hpp"
#include "interfaces/ILoRaWan.hpp"
#include "interfaces/IStorage.hpp"
#include "interfaces/ITimerService.hpp"
#include "interfaces/IUi.hpp"
#include <cstdint>

namespace lgt92::stm32 {

class Stm32Gps : public interfaces::IGps {
public:
    void power_on() override;
    void power_off() override;
    bool is_powered() const override;
    bool has_valid_fix() const override;
    adapters::GpsFixData get_fix_data() const override;
    void update() override;

private:
    bool powered_{false};
    adapters::LwGpsAdapter parser_{};
};

class Stm32Imu : public interfaces::IImu {
public:
    bool init() override;
    adapters::EulerAngles read_orientation() override;
    bool has_motion_detected() override;
    void clear_motion_flag() override;

private:
    adapters::FusionAdapter ahrs_{10.0f};
    bool motion_flag_{false};
};

class Stm32Battery : public interfaces::IBattery {
public:
    uint16_t read_voltage_mv() override;
};

class Stm32LoRaWan : public interfaces::ILoRaWan {
public:
    bool is_joined() const override;
    bool join() override;
    bool send(uint8_t port, std::span<const uint8_t> payload, bool confirmed) override;
    void set_downlink_callback(DownlinkCallback cb) override;
    void process() override;

private:
    DownlinkCallback downlink_cb_{};
};

class Stm32Storage : public interfaces::IStorage {
public:
    bool load_config(core::TrackerConfig& config) override;
    bool save_config(const core::TrackerConfig& config) override;
    bool load_credentials(core::LoRaCredentials& creds) override;
    bool save_credentials(const core::LoRaCredentials& creds) override;
    bool factory_reset() override;
};

class Stm32TimerService : public interfaces::ITimerService {
public:
    uint32_t get_time_ms() const override;
    void delay_ms(uint32_t ms) override;
    void sleep_low_power_ms(uint32_t ms) override;
};

class Stm32Ui : public interfaces::IUi {
public:
    void set_led(interfaces::LedColor color, bool on) override;
    void blink_led(interfaces::LedColor color, uint16_t on_ms) override;
    void set_alarm_button_callback(ButtonCallback cb) override;

    void handle_button_irq();

private:
    ButtonCallback button_cb_{};
};

} // namespace lgt92::stm32

