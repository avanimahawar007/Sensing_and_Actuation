#include <Wire.h>

// ====== CHANGE THESE (GPIO numbers) ======
#define SDA_GP 16
#define SCL_GP 17
// =========================================

// Own I2C bus on the pins above (replaces the default Wire)
arduino::MbedI2C bus((PinName)SDA_GP, (PinName)SCL_GP);

const uint8_t ADDR = 0x68;
const int N = 500;              // samples per pose (~5 s)

void writeReg(uint8_t reg, uint8_t val) {
  bus.beginTransmission(ADDR);
  bus.write(reg);
  bus.write(val);
  bus.endTransmission();
}

// Returns ax, ay, az, gx, gy, gz as raw counts
bool readRaw(int16_t *r) {
  bus.beginTransmission(ADDR);
  bus.write(0x3B);
  if (bus.endTransmission(false) != 0) return false;
  if (bus.requestFrom((int)ADDR, 14) != 14) return false;
  int16_t v[7];
  for (int i = 0; i < 7; i++) {
    uint8_t hi = bus.read();
    uint8_t lo = bus.read();
    v[i] = (int16_t)((hi << 8) | lo);
  }
  r[0] = v[0]; r[1] = v[1]; r[2] = v[2];   // accel
  r[3] = v[4]; r[4] = v[5]; r[5] = v[6];   // gyro (v[3] is temperature)
  return true;
}

void capture(double *avg) {
  double sum[6] = {0};
  int got = 0;
  while (got < N) {
    int16_t r[6];
    if (readRaw(r)) {
      for (int i = 0; i < 6; i++) sum[i] += r[i];
      got++;
    }
    delay(10);
  }
  for (int i = 0; i < 6; i++) avg[i] = sum[i] / N;
}

void waitForKey() {
  while (Serial.available()) Serial.read();
  while (!Serial.available()) delay(10);
  delay(50);
  while (Serial.available()) Serial.read();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  bus.begin();
  bus.setClock(400000);

  writeReg(0x6B, 0x00);       // wake
  writeReg(0x1C, 0x00);       // +-2 g
  writeReg(0x1B, 0x00);       // +-250 dps
  writeReg(0x1A, 0x03);       // ~44 Hz filter
  delay(100);

  int16_t test[6];
  if (!readRaw(test)) {
    Serial.print("Cannot read MPU-6050. Check wiring (SDA=GP");
    Serial.print(SDA_GP); Serial.print(", SCL=GP"); Serial.print(SCL_GP);
    Serial.println(", 3V3, GND).");
    while (true) delay(1000);
  }

  const char *names[6] = {
    "Z UP   (chip face up, lying flat)",
    "Z DOWN (flipped upside down)",
    "X UP   (X arrow pointing at the ceiling)",
    "X DOWN (X arrow pointing at the floor)",
    "Y UP   (Y arrow pointing at the ceiling)",
    "Y DOWN (Y arrow pointing at the floor)"
  };

  Serial.println("MPU-6050 calibration. Warm up ~5 minutes first.");
  Serial.println("For each pose: place it, hold it STILL, type any character + Enter.");

  double pose[6][6];
  for (int p = 0; p < 6; p++) {
    Serial.print("\nPose "); Serial.print(p + 1); Serial.print("/6: ");
    Serial.println(names[p]);
    Serial.println("Type a character and press Enter when it is still...");
    waitForKey();
    Serial.println("Capturing... don't touch it.");
    capture(pose[p]);
    Serial.println("Done.");
  }

  // Pose order: 0=Zup 1=Zdown 2=Xup 3=Xdown 4=Yup 5=Ydown
  double bias[3], scale[3];
  bias[0] = (pose[2][0] + pose[3][0]) / 2;  scale[0] = (pose[2][0] - pose[3][0]) / 2;
  bias[1] = (pose[4][1] + pose[5][1]) / 2;  scale[1] = (pose[4][1] - pose[5][1]) / 2;
  bias[2] = (pose[0][2] + pose[1][2]) / 2;  scale[2] = (pose[0][2] - pose[1][2]) / 2;

  double gbias[3] = {0, 0, 0};
  for (int p = 0; p < 6; p++)
    for (int i = 0; i < 3; i++) gbias[i] += pose[p][3 + i] / 6.0;

  Serial.println("\n===== PASTE THESE INTO YOUR CODE =====");
  Serial.print("float ACC_BIAS[3]  = {");
  Serial.print(bias[0], 2); Serial.print(", "); Serial.print(bias[1], 2); Serial.print(", "); Serial.print(bias[2], 2);
  Serial.println("};");
  Serial.print("float ACC_SCALE[3] = {");
  Serial.print(scale[0], 2); Serial.print(", "); Serial.print(scale[1], 2); Serial.print(", "); Serial.print(scale[2], 2);
  Serial.println("};");
  Serial.print("float GYRO_BIAS[3] = {");
  Serial.print(gbias[0], 2); Serial.print(", "); Serial.print(gbias[1], 2); Serial.print(", "); Serial.print(gbias[2], 2);
  Serial.println("};");
  Serial.println("(Scales should be near 16384. Far off means a pose wasn't still or square.)");
}

void loop() {}