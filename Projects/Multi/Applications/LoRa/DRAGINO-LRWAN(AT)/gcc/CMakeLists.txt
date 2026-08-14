cmake_minimum_required(VERSION 3.20)

project(lgt92 LANGUAGES C CXX ASM)

# enable_testing()

# Global configuration defaults
set(REGION "EU868" CACHE STRING "LoRaWAN Region")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/gcc-stm32l072cz.ld")

# 1. Base search directories
set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..")

# 2. Source file manifests
set(SRC_FILES
    # Project End Node
    "../src/bsp.c"
    "../src/debug.c"
    "../src/hw_gpio.c"
    "../src/hw_rtc.c"
    "../src/hw_spi.c"
    "../src/main.c"
    "../src/stm32l0xx_hal_msp.c"
    "../src/stm32l0xx_hw.c"
    "../src/stm32l0xx_it.c"
    "../src/vcom.c"
    "../src/at.c"
    "../src/command.c"
    "../src/lora.c"
    "../src/tiny_sscanf.c"
    "../src/tiny_vsnprintf.c"
    "../src/subsystems/imu_fusion.c"

    # Startup Assembly
    "${BASE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/gcc/startup_stm32l072xx.s"

    # CMSIS Device Templates
    "${BASE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c"

    # BSP Components
    "${BASE_DIR}/Drivers/BSP/Components/flash_eraseprogram/flash_eraseprogram.c"
    "${BASE_DIR}/Drivers/BSP/Components/iwdg/iwdg.c"
    "${BASE_DIR}/Drivers/BSP/Components/sx1276/sx1276.c"
    "${BASE_DIR}/Drivers/BSP/Components/gpio_exti/gpio_exti.c"
    "${BASE_DIR}/Drivers/BSP/Components/filter/filter.c"
    "${BASE_DIR}/Drivers/BSP/Components/IIC/IIC.c"
    "${BASE_DIR}/Drivers/BSP/Components/gps/gps.c"
    "${BASE_DIR}/Drivers/BSP/Components/mpu9250/mpu9250.c"
    "${BASE_DIR}/Drivers/BSP/Components/usart/bsp_usart2.c"
    "${BASE_DIR}/Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.c"
    "${BASE_DIR}/Drivers/BSP/sx1276mb1las/sx1276mb1las.c"

    # HAL Drivers
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_spi.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_iwdg.c"

    # LoRaWAN Middleware
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/Region.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionAS923.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionAU915.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionCN470.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionCN779.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionCommon.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionEU433.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionEU868.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionIN865.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionKR920.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionUS915.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionRU864.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/region/RegionKZ865.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/LoRaMac.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac/LoRaMacCrypto.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Core/lora-test.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities/delay.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities/low_power_manager.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities/timeServer.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities/utilities.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities/trace.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities/queue.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Crypto/aes.c"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Crypto/cmac.c"
)

# 3. Create Firmware Target
add_executable(${PROJECT_NAME}.elf ${SRC_FILES})

# 4. Target C/C++ Compile Standards
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    C_STANDARD_REQUIRED ON
)

# 5. Compiler Flags & Optimizations
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -Os
    -Werror
    -ffunction-sections
    -fdata-sections
)

# 6. Global Global Defines
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    STM32L072xx
    USE_STM32L0XX_NUCLEO
    USE_HAL_DRIVER
    USE_SHT
    REGION_${REGION}
)

# 7. Header File Include Search Paths
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    "../inc"
    "${BASE_DIR}/Drivers/BSP/Components/Common"
    "${BASE_DIR}/Drivers/BSP/Components/filter"
    "${BASE_DIR}/Drivers/BSP/Components/flash_eraseprogram"
    "${BASE_DIR}/Drivers/BSP/Components/gpio_exti"
    "${BASE_DIR}/Drivers/BSP/Components/gps"
    "${BASE_DIR}/Drivers/BSP/Components/IIC"
    "${BASE_DIR}/Drivers/BSP/Components/iwdg"
    "${BASE_DIR}/Drivers/BSP/Components/mpu9250"
    "${BASE_DIR}/Drivers/BSP/Components/sx1276"
    "${BASE_DIR}/Drivers/BSP/Components/usart"
    "${BASE_DIR}/Drivers/BSP/STM32L0xx_Nucleo"
    "${BASE_DIR}/Drivers/BSP/sx1276mb1las"
    "${BASE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Include"
    "${BASE_DIR}/Drivers/CMSIS/Include"
    "${BASE_DIR}/Drivers/STM32L0xx_HAL_Driver/Inc"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Core"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Crypto"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Mac"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Phy"
    "${BASE_DIR}/Middlewares/Third_Party/Lora/Utilities"
    "${BASE_DIR}/src/subsystems"
)

# 8. Linker Commands & Libraries
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T "${LINKER_SCRIPT}"
    -Wl,--gc-sections
    -specs=nano.specs
    -specs=nosys.specs
)

# Fix for missing math library symbols (tanf, ceil, floor)
target_link_libraries(${PROJECT_NAME}.elf 
    PRIVATE 
    m
)

# 9. Hex File Conversion & Flash Tasks
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMENT "Building ${PROJECT_NAME}.hex"
)

add_custom_target(flash
    COMMAND openocd -f interface/stlink-v2.cfg -c "transport select hla_swd" -f target/stm32l0.cfg -c "program ${PROJECT_NAME}.hex verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing firmware to device..."
)
