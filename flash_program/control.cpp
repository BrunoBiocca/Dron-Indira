#include "control.h"
#include "pid.h"
#include <Arduino.h>

static const float MAX_ANGLE = 20.0;
static const int STICK_DEADZONE = 10;
static const float START_THRUST = 50.0;
static const float THRUST_MIN_FLY = 20.0;
static const float THRUST_MAX_FLY = 80.0;
static const float THRUST_RATE = 20.0;
static const float LANDING_RATE = 15.0;
static const float MIN_THRUST = 15.0;
static const float ARM_MAX_TILT = 15.0;
static const float TILT_CUTOFF = 60.0;
static const unsigned long ARM_MIN_UPTIME_MS = 5000;

static PID rollPid(2.0, 0.0, 0.1, 50.0, 100.0);
static PID pitchPid(2.0, 0.0, 0.1, 50.0, 100.0);

static bool armed = false;
static bool landing = false;
static float thrust = 0;

static float stickToUnit(int value) {
  int a = abs(value);
  if (a < STICK_DEADZONE) return 0;
  float u = (a - STICK_DEADZONE) / (100.0f - STICK_DEADZONE);
  return value < 0 ? -u : u;
}

void controlDisarm(const char* reason) {
  if (armed) {
    Serial.print("DISARMED: ");
    Serial.println(reason);
  }
  armed = false;
  landing = false;
  thrust = 0;
  rollPid.reset();
  pitchPid.reset();
}

void controlStartLanding(const char* reason) {
  if (!armed || landing) return;
  landing = true;
  Serial.print("LANDING: ");
  Serial.println(reason);
}

void controlResetPid() {
  rollPid.reset();
  pitchPid.reset();
}

const char* controlTryArm(const ImuData& imu, const ControlInput& in) {
  if (armed) return "already armed";
  if (millis() < ARM_MIN_UPTIME_MS) return "sensor still settling";
  if (in.up || in.down) return "release UP/DOWN first";
  if (abs(in.joyX) >= STICK_DEADZONE || abs(in.joyY) >= STICK_DEADZONE) return "center the joystick first";
  if (fabsf(imu.roll) > ARM_MAX_TILT || fabsf(imu.pitch) > ARM_MAX_TILT) return "drone is not level";

  controlResetPid();
  thrust = START_THRUST;
  landing = false;
  armed = true;
  Serial.println("ARMED");
  return nullptr;
}

MotorOutputs controlUpdate(const ImuData& imu, const ControlInput& in, float dt) {
  MotorOutputs off = {0, 0, 0, 0};
  if (!armed) return off;

  if (fabsf(imu.roll) > TILT_CUTOFF || fabsf(imu.pitch) > TILT_CUTOFF) {
    controlDisarm("tilt limit exceeded");
    return off;
  }

  if (landing) {
    thrust -= LANDING_RATE * dt;
    if (thrust < MIN_THRUST) {
      controlDisarm("landing complete");
      return off;
    }
  } else {
    int vertical = (in.up ? 1 : 0) - (in.down ? 1 : 0);
    thrust += vertical * THRUST_RATE * dt;
    thrust = constrain(thrust, THRUST_MIN_FLY, THRUST_MAX_FLY);
  }

  float rollTarget = landing ? 0 : stickToUnit(in.joyX) * MAX_ANGLE;
  float pitchTarget = landing ? 0 : stickToUnit(in.joyY) * MAX_ANGLE;

  float rollCmd = rollPid.update(rollTarget, imu.roll, imu.gx, dt);
  float pitchCmd = pitchPid.update(pitchTarget, imu.pitch, imu.gy, dt);

  return mixerCompute(thrust, rollCmd, pitchCmd, 0);
}

bool controlIsArmed() { return armed; }
bool controlIsLanding() { return landing; }
float controlGetThrust() { return thrust; }
int controlState() { return !armed ? 0 : (landing ? 2 : 1); }