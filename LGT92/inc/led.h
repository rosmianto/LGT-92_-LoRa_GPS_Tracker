#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void led_init();
void led_red_on();
void led_green_on();
void led_blue_on();
void led_red_off();
void led_green_off();
void led_blue_off();
void led_run_animation();

#ifdef __cplusplus
}
#endif