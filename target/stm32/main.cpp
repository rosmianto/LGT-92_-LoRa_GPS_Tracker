#include "core/TrackerEngine.hpp"
#include "Stm32Adapters.hpp"

using namespace lgt92::stm32;

static Stm32Gps          s_gps;
static Stm32Imu          s_imu;
static Stm32Battery      s_battery;
static Stm32LoRaWan      s_lora;
static Stm32Storage      s_storage;
static Stm32TimerService s_timer;
static Stm32Ui           s_ui;

int main(void) {
    // 1. STM32 System & Clock Initialization

    // 2. Instantiate and Initialize Core Tracker Engine
    lgt92::core::TrackerEngine engine(
        s_gps, s_imu, s_battery, s_lora, s_storage, s_timer, s_ui
    );

    engine.init();

    // 3. Main execution loop
    while (true) {
        engine.step();
    }

    return 0;
}

