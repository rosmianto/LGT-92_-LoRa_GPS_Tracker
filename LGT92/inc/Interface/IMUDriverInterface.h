#pragma once

struct Accel {
  float x, y, z;
};

struct Gyro {
  float x, y, z;
};

struct Mag {
  float x, y, z;
};

class IMUDriverInterface {
public:
  virtual bool init() = 0;
  virtual Accel getAccelerationData() = 0;
  virtual Gyro getGyroData() = 0;
  virtual Mag getMagnetometerData() = 0;
};