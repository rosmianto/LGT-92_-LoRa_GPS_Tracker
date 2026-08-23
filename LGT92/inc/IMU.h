#pragma once

#include <Fusion.h>
#include <Interface/IMUDriverInterface.h>

class IMU {

public:
  IMU(IMUDriverInterface &sensor);
  bool init();
  FusionEuler getImuOrientation();

private:
  FusionAhrs _ahrs;
  IMUDriverInterface &_sensor;
};