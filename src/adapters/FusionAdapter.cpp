#include "adapters/FusionAdapter.hpp"

namespace lgt92::adapters {

FusionAdapter::FusionAdapter(float sample_rate_hz)
    : sample_rate_hz_(sample_rate_hz > 0.0f ? sample_rate_hz : 10.0f) {
    reset();
}

void FusionAdapter::reset() noexcept {
    FusionAhrsInitialise(&ahrs_);
    FusionAhrsSettings settings = fusionAhrsDefaultSettings;
    settings.convention = FusionConventionNwu; // North-West-Up
    settings.sampleRate = sample_rate_hz_;
    settings.gain = 0.5f;
    settings.accelerationRejection = 10.0f;
    settings.magneticRejection = 20.0f;
    settings.rejectionTimeout = 5.0f;
    FusionAhrsSetSettings(&ahrs_, &settings);
}

void FusionAdapter::update_6axis(const Vector3f& gyro_dps, const Vector3f& accel_g) noexcept {
    FusionVector gyro = {.axis = {gyro_dps.x, gyro_dps.y, gyro_dps.z}};
    FusionVector accel = {.axis = {accel_g.x, accel_g.y, accel_g.z}};
    FusionAhrsUpdateNoMagnetometer(&ahrs_, gyro, accel);
}

void FusionAdapter::update_9axis(const Vector3f& gyro_dps, const Vector3f& accel_g, const Vector3f& mag) noexcept {
    FusionVector gyro = {.axis = {gyro_dps.x, gyro_dps.y, gyro_dps.z}};
    FusionVector accel = {.axis = {accel_g.x, accel_g.y, accel_g.z}};
    FusionVector m = {.axis = {mag.x, mag.y, mag.z}};
    FusionAhrsUpdate(&ahrs_, gyro, accel, m);
}

EulerAngles FusionAdapter::get_euler_angles() const noexcept {
    FusionQuaternion q = FusionAhrsGetQuaternion(&ahrs_);
    FusionEuler euler = FusionQuaternionToEuler(q);

    return EulerAngles{
        .roll = euler.angle.roll,
        .pitch = euler.angle.pitch,
        .yaw = euler.angle.yaw
    };
}

} // namespace lgt92::adapters

