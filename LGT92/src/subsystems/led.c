#include "stm32l0xx_hal.h"
#include <bsp.h>
#include <delay.h>
#include <led.h>
#include <stm32l0xx_hw_conf.h>

// LED0-> Green
// LED1-> Blue
// LED3-> Red
void led_init() { BSP_powerLED_Init(); }

void led_red_on() { LED3_1; }

void led_green_on() { LED0_1; }

void led_blue_on() { LED1_1; }

void led_red_off() { LED3_0; }

void led_green_off() { LED0_0; }

void led_blue_off() { LED1_0; }

void led_run_animation() {
  led_green_on();
  DelayMs(200);
  led_green_off();
  led_blue_on();
  DelayMs(200);
  led_blue_off();
  led_red_on();
  DelayMs(200);
  led_red_off();
}