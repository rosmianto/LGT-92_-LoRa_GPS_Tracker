#include "hw_conf.h"
#include "hw_msp.h"
#include "stm32l0xx_hal.h"
#include <battery.h>

#define LORAWAN_MAX_BAT 254

// I don't know why this macro is defined inside stm32l0xx_hw.c
#define VREFINT_CAL ((uint16_t *)((uint32_t)0x1FF80078))

uint16_t battery_get_voltage_mV() {

  uint16_t battery_ADC_value = 0;
  uint16_t vref_ADC_value = 0;

  HAL_GPIO_WritePin(battery_CONTROL_PORT, battery_CONTROL_PIN, GPIO_PIN_RESET);

  battery_ADC_value = HW_AdcReadChannel(ADC_Channel_battery);
  vref_ADC_value = HW_AdcReadChannel(ADC_CHANNEL_VREFINT);

  HAL_GPIO_WritePin(battery_CONTROL_PORT, battery_CONTROL_PIN, GPIO_PIN_SET);

  // Calculate VREF voltage in mV
  uint32_t vdd_mv = (VDDA_VREFINT_CAL * (*VREFINT_CAL)) / vref_ADC_value;

  // Calculate Battery voltage in mV
  uint16_t batt_mv = (battery_ADC_value * vdd_mv) / 4095;

  return batt_mv * (47 + 10) / 47; // Divider resistors correction.
}

uint8_t battery_get_voltage_byte() {
  uint16_t batt_mv = battery_get_voltage_mV();

  // Upper boundary check
  if (batt_mv > VDD_BAT) {
    return LORAWAN_MAX_BAT;
  }

  // Lower boundary check
  if (batt_mv < VDD_MIN) {
    return 0;
  }

  uint8_t batt_level =
      (batt_mv - VDD_MIN) * LORAWAN_MAX_BAT / (VDD_BAT - VDD_MIN);

  return batt_level;

  // TODO: I don't know. This code assume CR3202 external battery.
  // But LGT92 either use 2xAA batteries or Li-Ion.
  // So the battery level mapping should be different (VDD_BAT and VDD_MIN).
}