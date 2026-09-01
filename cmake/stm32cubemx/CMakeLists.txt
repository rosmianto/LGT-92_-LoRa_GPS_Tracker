cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
    USE_STM32L0XX_NUCLEO 
    USE_HAL_DRIVER 
    REGION_AS923
	USE_HAL_DRIVER 
	STM32L072xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32L0xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/filter
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/flash_eraseprogram
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/gpio_exti
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/gps
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/IIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/iwdg
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/mpu9250
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/sx1276
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/usart
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/STM32L0xx_Nucleo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/sx1276mb1las
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32L0xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Crypto
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Phy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/version.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/temp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/drivers/ConfigStorage_Dummy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/subsystems/ATParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/subsystems/ConfigManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/drivers/IMUDriver_Dummy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/subsystems/battery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/subsystems/gps_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/subsystems/IMU.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/subsystems/led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/at.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/bsp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/command.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/debug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/hw_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/hw_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/hw_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/lora.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/stm32l0xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/stm32l0xx_hw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/stm32l0xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/tiny_sscanf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/tiny_vsnprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LGT92/src/vcom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32l072xx.s
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/filter/filter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/flash_eraseprogram/flash_eraseprogram.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/gpio_exti/gpio_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/gps/gps.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/IIC/IIC.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/iwdg/iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/mpu9250/mpu9250.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/sx1276/sx1276.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/usart/bsp_usart2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/sx1276mb1las/sx1276mb1las.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/Region.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionAS923.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionAU915.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionCN470.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionCN779.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionCommon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionEU433.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionEU868.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionIN865.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionKR920.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionRU864.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/region/RegionUS915.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/LoRaMac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Mac/LoRaMacCrypto.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Core/lora-test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities/delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities/low_power_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities/timeServer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities/trace.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Utilities/utilities.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Crypto/aes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/LoRaWAN/Crypto/cmac.c
    )

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_iwdg.c
)

# Drivers Midllewares



# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
