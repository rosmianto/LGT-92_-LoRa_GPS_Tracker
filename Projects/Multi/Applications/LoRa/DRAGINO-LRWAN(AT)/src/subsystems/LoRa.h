#pragma once

#include <stdint.h>

class LoRa {
  public:
	LoRa();
	bool init();
	bool stop();
	bool sendPayload(uint8_t *payload, uint8_t len);

  private:
};