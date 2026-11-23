#pragma once

struct MotorOutputs {
  float fl;
  float fr;
  float rl;
  float rr;
};

MotorOutputs mixerCompute(float thrust, float rollCmd, float pitchCmd, float yawCmd);