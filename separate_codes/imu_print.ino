
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Your measured gyroscope offsets
const float biasGX = 660.23;
const float biasGY = 324.25;
const float biasGZ = -267.45;

// Default sensor sensitivities
const float gyroSensitivity = 131.0;   // counts per deg/s
const float accelSensitivity = 16384.0; // counts per g

void setup() {
  Serial.begin(115200);

  // Use default I2C pins
  Wire.begin();

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("IMU connected!");
  } else {
    Serial.println("Connection failed!");
  }

  Serial.println("Accelerometer and Gyroscope readings:");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Correct gyroscope offsets
  float correctedGX = gx - biasGX;
  float correctedGY = gy - biasGY;
  float correctedGZ = gz - biasGZ;

  // Convert gyroscope readings to degrees per second
  float gyroX = correctedGX / gyroSensitivity;
  float gyroY = correctedGY / gyroSensitivity;
  float gyroZ = correctedGZ / gyroSensitivity;

  // Convert accelerometer readings to g
  float accelX = ax / accelSensitivity;
  float accelY = ay / accelSensitivity;
  float accelZ = az / accelSensitivity;

  // Print accelerometer readings
  Serial.print("Acc X: ");
  Serial.print(accelX);
  Serial.print(" g | Acc Y: ");
  Serial.print(accelY);
  Serial.print(" g | Acc Z: ");
  Serial.print(accelZ);
  Serial.print(" g || ");

  // Print gyroscope readings
  Serial.print("Gyro X: ");
  Serial.print(gyroX);
  Serial.print(" deg/s | Gyro Y: ");
  Serial.print(gyroY);
  Serial.print(" deg/s | Gyro Z: ");
  Serial.print(gyroZ);
  Serial.println(" deg/s");

  delay(100);
}
