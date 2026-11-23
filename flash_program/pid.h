#pragma once

class PID {
public:
  PID(float p, float i, float d, float iLimit, float outLimit);
  float update(float setpoint, float measured, float rate, float dt);
  void reset();

private:
  float kp, ki, kd;
  float integralLimit;
  float outputLimit;
  float integral;
};