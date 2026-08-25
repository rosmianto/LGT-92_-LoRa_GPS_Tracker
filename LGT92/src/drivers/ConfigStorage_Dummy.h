#pragma once

#include <Interface/ConfigStorageInterface.h>
#include <ConfigManager.h>

class ConfigStorage_Dummy : public ConfigStorageInterface {
public:
    bool init();
    bool write(const ConfigData &data);
    bool read(const ConfigData *data);
};