#pragma once

#include "lwgps/lwgps.h"
#include <string_view>
#include <cstddef>
#include <cstdint>

namespace lgt92::adapters {

struct GpsFixData {
    bool        has_fix{false};
    double      latitude{0.0};     // Decimal degrees (pos = N, neg = S)
    double      longitude{0.0};    // Decimal degrees (pos = E, neg = W)
    float       altitude_m{0.0f};  // Meters above sea level
    float       hdop{99.9f};       // Horizontal dilution of precision
    float       speed_kmh{0.0f};   // Speed over ground
    uint8_t     satellites_in_use{0};
};

class LwGpsAdapter {
public:
    LwGpsAdapter();
    ~LwGpsAdapter() = default;

    // Feeds raw NMEA stream (buffer or char)
    bool feed(std::string_view nmea_chunk) noexcept;
    bool feed(char byte) noexcept;

    [[nodiscard]] bool has_valid_fix() const noexcept;
    [[nodiscard]] GpsFixData get_fix_data() const noexcept;
    void reset() noexcept;

private:
    lwgps_t gps_{};
};

} // namespace lgt92::adapters

