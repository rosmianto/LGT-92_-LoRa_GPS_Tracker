#pragma once

#include <stdint.h>

// Packed attribute. GCC only.
#define PACKED __attribute__((packed))
#define CONFIG_VERSION    1

// enum class GPSModel : uint8_t {
//     L70RL       = 0,
//     L76L        = 1,
//     UBLOX_MAX7  = 2,
//     UBLOX_MAX8  = 3,
//     L76K        = 4
// };

struct PACKED ConfigStructure {
    uint8_t configVersion;
    uint8_t fwVersionMajor;
    uint8_t fwVersionMinor;
    uint8_t fwVersionRevision;
    // GPSModel gpsModel;
    uint16_t hwVersion;

};

struct PACKED ConfigData {
    union {
        ConfigStructure data;
        uint8_t bytes[sizeof(ConfigStructure)];
    };
    uint32_t checksumCRC32 = 0;
};