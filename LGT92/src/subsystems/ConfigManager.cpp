#include <ConfigManager.h>
#include <version.h>

ConfigManager::ConfigManager(ConfigStorageInterface &stg) : _stg(stg) {
    _factory = {
        .data = {
            .configVersion = CONFIG_VERSION,
            .fwVersionMajor = FW_VERSION_MAJOR,
            .fwVersionMinor = FW_VERSION_MINOR,
            .fwVersionRevision = FW_VERSION_REVISION
        }
    };

    cfg = _factory;
}

bool ConfigManager::init() {
    return _stg.init();
}

bool ConfigManager::loadConfig() {
    if (_stg.read(&cfg) == false) {
        return false;
    }

    // Check for data integrity
    uint32_t calculatedChecksum = calculateCRC32(cfg.bytes, sizeof(cfg.bytes));
    if (cfg.checksumCRC32 != calculatedChecksum) {
        return false;
    }

    // Validate the Config Version first
    if (cfg.data.configVersion != _factory.data.configVersion) {
        // Different config version means wiping out existing configs

        // First we set everything to factory values
        cfg = _factory;

        // Then we write those values back to memory
        return storeConfig();
    }

    return true;
}

bool ConfigManager::storeConfig() {
    uint32_t calculatedChecksum = calculateCRC32(cfg.bytes, sizeof(cfg.bytes));
    cfg.checksumCRC32 = calculatedChecksum;

    return _stg.write(cfg);
}