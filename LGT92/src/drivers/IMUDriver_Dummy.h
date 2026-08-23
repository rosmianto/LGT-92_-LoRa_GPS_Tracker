#pragma once

#include <Interface/IMUDriverInterface.h>

class IMUDriver_Dummy : public IMUDriverInterface {

public:
  bool init();
  Accel getAccelerationData();
  Gyro getGyroData();
  Mag getMagnetometerData();
};