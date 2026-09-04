set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/gcc/gcc-stm32l072cz.ld")
set(STARTUP_SRC "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/gcc/startup_stm32l072xx.s")
set(SYSTEM_SRC "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c")

add_executable(lgt92_firmware.elf
    main.cpp
    Stm32Adapters.cpp
    ${STARTUP_SRC}
    ${SYSTEM_SRC}
)

target_include_directories(lgt92_firmware.elf PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/inc"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Include"
    "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include"
    "${CMAKE_SOURCE_DIR}/Drivers/STM32L0xx_HAL_Driver/Inc"
)

target_compile_definitions(lgt92_firmware.elf PRIVATE
    STM32L072xx
    USE_HAL_DRIVER
)

target_link_options(lgt92_firmware.elf PRIVATE
    "-T${LINKER_SCRIPT}"
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/lgt92_firmware.map"
    "-Wl,--gc-sections"
    "-lm"
)

target_link_libraries(lgt92_firmware.elf PRIVATE
    lgt92_core
)

add_custom_command(TARGET lgt92_firmware.elf POST_BUILD
    COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:lgt92_firmware.elf> ${CMAKE_CURRENT_BINARY_DIR}/lgt92_firmware.hex
    COMMAND ${ARM_OBJCOPY} -O binary $<TARGET_FILE:lgt92_firmware.elf> ${CMAKE_CURRENT_BINARY_DIR}/lgt92_firmware.bin
    COMMAND ${ARM_SIZE} --format=berkeley $<TARGET_FILE:lgt92_firmware.elf>
    COMMENT "Generated Intel HEX and binary images for STM32L072"
)
