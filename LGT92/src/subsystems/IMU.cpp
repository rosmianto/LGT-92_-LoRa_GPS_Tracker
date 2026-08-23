#include <IMU.h>

IMU::IMU(IMUDriverInterface &sensor) : _sensor(sensor) {}

bool IMU::init() {
  if (_sensor.init() == false) {
    return false;
  }

  // Initialize AHRS Fusion
  FusionAhrsSettings settings = fusionAhrsDefaultSettings;
  FusionAhrsSetSettings(&_ahrs, &settings);

  return true;
}

FusionEuler IMU::getImuOrientation() {
  // We're using Fusion library to calculate AHRS

  FusionVector ahrsGyro = {};
  FusionVector ahrsAccel = {};
  Gyro imuGyro = {};
  Accel imuAccel = {};

  // Sample rate at 100Hz
  // TODO: This loop isn't guaranteed to run at 100Hz, use delay() later
  for (int i = 0; i < 50; i++) {
    imuGyro = _sensor.getGyroData();
    imuAccel = _sensor.getAccelerationData();

    ahrsGyro.axis.x = imuGyro.x;
    ahrsGyro.axis.y = imuGyro.y;
    ahrsGyro.axis.z = imuGyro.z;

    ahrsAccel.axis.x = imuAccel.x;
    ahrsAccel.axis.y = imuAccel.y;
    ahrsAccel.axis.z = imuAccel.z;

    FusionAhrsUpdateNoMagnetometer(&_ahrs, ahrsGyro, ahrsAccel);

    // TODO: Probably add delay here to make this loop 100Hz
  }

  const FusionEuler euler =
      FusionQuaternionToEuler(FusionAhrsGetQuaternion(&_ahrs));

  return euler;
}
