#pragma once
#include "imu.h"
#include "mixer.h"

struct ControlInput {
  int joyX;
  int joyY;
  bool up;
  bool down;
};

const char* controlTryArm(const ImuData& imu, const ControlInput& in);
void controlDisarm(const char* reason);
void controlStartLanding(const char* reason);
void controlResetPid();
MotorOutputs controlUpdate(const ImuData& imu, const ControlInput& in, float dt);

bool controlIsArmed();
bool controlIsLanding();
float controlGetThrust();
int controlState();