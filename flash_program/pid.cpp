#include "pid.h"
#include <Arduino.h>

PID::PID(float p, float i, float d, float iLimit, float outLimit) {
  kp = p;
  ki = i;
  kd = d;
  integralLimit = iLimit;
  outputLimit = outLimit;
  integral = 0;
}

float PID::update(float setpoint, float measured, float rate, float dt) {
  float error = setpoint - measured;

  integral += error * dt;
  integral = constrain(integral, -integralLimit, integralLimit); 

  float output = kp * error + ki * integral - kd * rate;
  return constrain(output, -outputLimit, outputLimit); //antiwindup
}

void PID::reset() {
  integral = 0;
}