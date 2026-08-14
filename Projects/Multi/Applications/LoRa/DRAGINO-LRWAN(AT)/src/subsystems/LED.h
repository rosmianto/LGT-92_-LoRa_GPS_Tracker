#pragma once

class LED {

  public:
	// LED3 -> Red
	// LED0 -> Green
	// LED1 -> Blue
	static void ledRedOn();
	static void ledRedOff();
	static void ledGreenOn();
	static void ledGreenOff();
	static void ledBlueOn();
	static void ledBlueOff();
};
