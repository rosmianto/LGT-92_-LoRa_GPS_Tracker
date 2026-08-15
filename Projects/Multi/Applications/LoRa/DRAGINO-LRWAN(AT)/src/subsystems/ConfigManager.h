#pragma once

#include <stdint.h>

struct __attribute__((packed)) deviceConfig {
	uint8_t devEUI;
	uint16_t networkKey;
	uint8_t appPort;
};

class ConfigManager {

  public:
	bool read();
	bool write();

	struct deviceConfig cfg;
};