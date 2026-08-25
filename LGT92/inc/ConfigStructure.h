#pragma once

#include <stdint.h>

// Packed attribute. GCC only.
#define PACKED __attribute__((packed))
#define CONFIG_VERSION    1

struct PACKED ConfigStructure {
    uint8_t configVersion;
    uint8_t fwVersionMajor;
    uint8_t fwVersionMinor;
    uint8_t fwVersionRevision;
};

struct PACKED ConfigData {
    union {
        ConfigStructure data;
        uint8_t bytes[sizeof(ConfigStructure)];
    };
    uint32_t checksumCRC32 = 0;
};