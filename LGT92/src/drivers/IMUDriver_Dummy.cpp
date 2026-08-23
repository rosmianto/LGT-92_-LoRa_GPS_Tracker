#include "IMUDriver_Dummy.h"

bool IMUDriver_Dummy::init() { return true; }

Accel IMUDriver_Dummy::getAccelerationData() {
  Accel accelData = {0.1, 1.0, 9.8};

  return accelData;
}
Gyro IMUDriver_Dummy::getGyroData() {
  Gyro gyroData = {12.0, 12.0, 12.9};

  return gyroData;
}

Mag IMUDriver_Dummy::getMagnetometerData() {
  Mag magData = {1.2, 2.3, 3.4};

  return magData;
}