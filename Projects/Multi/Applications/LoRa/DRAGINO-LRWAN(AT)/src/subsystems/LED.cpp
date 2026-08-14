#include "LED.h"
#include <stm32l0xx_hw_conf.h>

void LED::ledRedOn() { LED3_1; }
void LED::ledRedOff() { LED3_0; }
void LED::ledGreenOn() { LED0_1; }
void LED::ledGreenOff() { LED0_0; }
void LED::ledBlueOn() { LED1_1; }
void LED::ledBlueOff() { LED1_0; }