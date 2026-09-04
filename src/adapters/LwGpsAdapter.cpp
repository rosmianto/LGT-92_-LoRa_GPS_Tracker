#include "adapters/LwGpsAdapter.hpp"

namespace lgt92::adapters {

LwGpsAdapter::LwGpsAdapter() {
    reset();
}

void LwGpsAdapter::reset() noexcept {
    lwgps_init(&gps_);
}

bool LwGpsAdapter::feed(std::string_view nmea_chunk) noexcept {
    if (nmea_chunk.empty()) return false;
    return lwgps_process(&gps_, nmea_chunk.data(), nmea_chunk.size()) != 0;
}

bool LwGpsAdapter::feed(char byte) noexcept {
    return lwgps_process(&gps_, &byte, 1) != 0;
}

bool LwGpsAdapter::has_valid_fix() const noexcept {
    return gps_.is_valid != 0 && gps_.fix > 0;
}

GpsFixData LwGpsAdapter::get_fix_data() const noexcept {
    GpsFixData data{};
    data.has_fix = has_valid_fix();
    data.latitude = static_cast<double>(gps_.latitude);
    data.longitude = static_cast<double>(gps_.longitude);
    data.altitude_m = static_cast<float>(gps_.altitude);
    data.hdop = static_cast<float>(gps_.dop_h);
    data.speed_kmh = static_cast<float>(gps_.speed * 1.852f); // knots to km/h
    data.satellites_in_use = gps_.sats_in_use;
    return data;
}

} // namespace lgt92::adapters

