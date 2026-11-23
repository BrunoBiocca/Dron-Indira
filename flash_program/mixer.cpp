#include "mixer.h"
#include <Arduino.h>

MotorOutputs mixerCompute(float thrust, float rollCmd, float pitchCmd, float yawCmd) {
  MotorOutputs m;

  m.fl = thrust + rollCmd - pitchCmd + yawCmd;
  m.fr = thrust - rollCmd - pitchCmd - yawCmd;
  m.rl = thrust + rollCmd + pitchCmd - yawCmd;
  m.rr = thrust - rollCmd + pitchCmd + yawCmd;

  m.fl = constrain(m.fl, 0.0f, 100.0f);
  m.fr = constrain(m.fr, 0.0f, 100.0f);
  m.rl = constrain(m.rl, 0.0f, 100.0f);
  m.rr = constrain(m.rr, 0.0f, 100.0f);

  return m;
}