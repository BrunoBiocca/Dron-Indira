#include "imu.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <Preferences.h>

static Preferences prefs;
static float rollOffset = 0, pitchOffset = 0;
static float rollF = 0, pitchF = 0;

static const uint8_t MPU_ADDR = 0x68;
static const int PIN_SDA = 21;
static const int PIN_SCL = 22;
static const unsigned long PERIOD_US = 5000; //5 ms sampling rate

static Adafruit_MPU6050 mpu; //creates the mpu object
static float gxOff = 0, gyOff = 0, gzOff = 0;
static ImuData data = {0, 0, 0, 0, 0};
static unsigned long prevTime = 0;

static bool readRaw(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) return false;

  ax = a.acceleration.x / 9.80665f;
  ay = a.acceleration.y / 9.80665f;
  az = a.acceleration.z / 9.80665f;

  gx = g.gyro.x * RAD_TO_DEG;
  gy = g.gyro.y * RAD_TO_DEG;
  gz = g.gyro.z * RAD_TO_DEG;
  return true;
}

void imuBegin() {
  prefs.begin("imu", false);
  rollOffset = prefs.getFloat("rollOff", 0);
  pitchOffset = prefs.getFloat("pitchOff", 0);

  Wire.begin(PIN_SDA, PIN_SCL);

  if (!mpu.begin(MPU_ADDR, &Wire)) {
    Serial.println("ERROR: MPU6050 not found. Check wiring.");
    while (true) delay(1000);
  }
  Wire.setClock(400000);

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

  Serial.println("Calibrating gyro, keep the sensor still...");
  const int N = 500;
  float sx = 0, sy = 0, sz = 0;
  for (int i = 0; i < N; i++) {
    float ax, ay, az, gx, gy, gz;
    if (readRaw(ax, ay, az, gx, gy, gz)) {
      sx += gx;
      sy += gy;
      sz += gz;
    }
    delay(3);
  }
  gxOff = sx / N;
  gyOff = sy / N;
  gzOff = sz / N;
  Serial.println("Calibration done.");

  prevTime = micros();
}

bool imuUpdate() {
  unsigned long now = micros();
  if (now - prevTime < PERIOD_US) return false;

  float dt = (now - prevTime) / 1e6f;
  prevTime = now;

  float ax, ay, az, gx, gy, gz;
  if (!readRaw(ax, ay, az, gx, gy, gz)) return false;

  gx -= gxOff;
  gy -= gyOff;
  gz -= gzOff;

  float rollAcc = atan2f(ay, az) * RAD_TO_DEG;
  float pitchAcc = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

  rollF = 0.98f * (rollF + gx * dt) + 0.02f * rollAcc;
  pitchF = 0.98f * (pitchF + gy * dt) + 0.02f * pitchAcc;

  data.roll = rollF - rollOffset;
  data.pitch = pitchF - pitchOffset;

  data.gx = gx;
  data.gy = gy;
  data.gz = gz;

  return true;
}

ImuData imuGet() {
  return data;
}

void imuCalibrateLevel() {
  Serial.println("Calibrating level: keep the drone flat and still...");

  unsigned long start = millis();
  while (millis() - start < 2000) imuUpdate();

  float sumRoll = 0, sumPitch = 0;
  int n = 0;
  start = millis();
  while (millis() - start < 1000) {
    if (imuUpdate()) {
      sumRoll += rollF;
      sumPitch += pitchF;
      n++;
    }
  }
  if (n == 0) return;

  rollOffset = sumRoll / n;
  pitchOffset = sumPitch / n;
  prefs.putFloat("rollOff", rollOffset);
  prefs.putFloat("pitchOff", pitchOffset);

  Serial.printf("Offsets saved: roll %.2f  pitch %.2f\n", rollOffset, pitchOffset);
}