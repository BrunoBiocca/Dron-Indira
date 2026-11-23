#pragma once

struct ImuData {
  float roll;
  float pitch;
  float gx;
  float gy;
  float gz;
};

void imuBegin();
bool imuUpdate();
ImuData imuGet();
void imuCalibrateLevel();