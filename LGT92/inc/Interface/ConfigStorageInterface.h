#pragma once

#include <ConfigStructure.h>

class ConfigStorageInterface {
public:
    virtual bool init() = 0;
    virtual bool write(const ConfigData &data) = 0;
    virtual bool read(const ConfigData *data) = 0;
};