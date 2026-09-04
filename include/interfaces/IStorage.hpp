#pragma once

#include "core/TrackerConfig.hpp"
#include "core/LoRaCredentials.hpp"

namespace lgt92::interfaces {

class IStorage {
public:
    virtual ~IStorage() = default;

    virtual bool load_config(core::TrackerConfig& config) = 0;
    virtual bool save_config(const core::TrackerConfig& config) = 0;

    virtual bool load_credentials(core::LoRaCredentials& creds) = 0;
    virtual bool save_credentials(const core::LoRaCredentials& creds) = 0;

    virtual bool factory_reset() = 0;
};

} // namespace lgt92::interfaces

