#pragma once

#include <stdint.h>
#include <Interface/ConfigStorageInterface.h>
#include <ConfigStructure.h>

class ConfigManager {

public:
    ConfigManager(ConfigStorageInterface &stg);
    bool init();
    bool loadConfig();
    bool storeConfig();
    ConfigData cfg;

private:
    ConfigData _factory;  // The default factory settings
    ConfigStorageInterface &_stg;
    uint32_t calculateCRC32(const uint8_t *data, int len);
};