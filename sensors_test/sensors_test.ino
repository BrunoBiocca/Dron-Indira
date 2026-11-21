
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // Inicializar I2C
  Wire.begin(21, 22);

  // Inicializar MPU6050
  if (!mpu.begin()) {
    Serial.println("No se encontro el MPU6050");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 detectado correctamente!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);

  Serial.print("Aceleracion [m/s^2]: ");
  Serial.print(a.acceleration.x);
  Serial.print(", ");
  Serial.print(a.acceleration.y);
  Serial.print(", ");
  Serial.println(a.acceleration.z);

  Serial.print("Giroscopio [rad/s]: ");
  Serial.print(g.gyro.x);
  Serial.print(", ");
  Serial.print(g.gyro.y);
  Serial.print(", ");
  Serial.println(g.gyro.z);

  Serial.print("Temperatura [°C]: ");
  Serial.println(temp.temperature);

  Serial.println("-----------------------");

  delay(500);
}
