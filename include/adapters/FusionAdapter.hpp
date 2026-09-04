#pragma once

#include "Fusion.h"

namespace lgt92::adapters {

struct EulerAngles {
    float roll{0.0f};   // Degrees (-180 to +180)
    float pitch{0.0f};  // Degrees (-90 to +90)
    float yaw{0.0f};    // Degrees (0 to 360)
};

struct Vector3f {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

class FusionAdapter {
public:
    explicit FusionAdapter(float sample_rate_hz = 10.0f);
    ~FusionAdapter() = default;

    void reset() noexcept;

    // Update with 6-axis IMU data (gyroscope in deg/sec, accelerometer in g)
    void update_6axis(const Vector3f& gyro_dps, const Vector3f& accel_g) noexcept;

    // Update with 9-axis IMU data (gyroscope in deg/sec, accelerometer in g, magnetometer)
    void update_9axis(const Vector3f& gyro_dps, const Vector3f& accel_g, const Vector3f& mag) noexcept;

    [[nodiscard]] EulerAngles get_euler_angles() const noexcept;

private:
    FusionAhrs ahrs_{};
    float sample_rate_hz_{10.0f};
};

} // namespace lgt92::adapters

