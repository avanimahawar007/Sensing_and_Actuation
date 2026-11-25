#include <Wire.h>

// ====== CHANGE THESE (GPIO numbers) ======
#define SDA_GP 16
#define SCL_GP 17
// =========================================

// ====== PASTE YOUR CALIBRATION RESULTS HERE ======
float ACC_BIAS[3]  = {-125.72, -37.61, 1144.05};
float ACC_SCALE[3] = {16371.10, 16366.18, 16833.11};
float GYRO_BIAS[3] = {660.23, 324.25, -267.45};
// =================================================

// ====== LIMIT LINES (mode 3 only) ======
#define HI_LIM 1.1
#define LO_LIM 0.90
// =======================================

// What to plot:
//   1 = calibrated accelerometer (g) on x, y, z
//   2 = calibrated gyro (deg/s) on x, y, z
//   3 = total acceleration magnitude, raw vs calibrated, with limit lines
#define PLOT_MODE 3

arduino::MbedI2C bus((PinName)SDA_GP, (PinName)SCL_GP);
const uint8_t ADDR = 0x68;

void writeReg(uint8_t reg, uint8_t val) {
  bus.beginTransmission(ADDR);
  bus.write(reg);
  bus.write(val);
  bus.endTransmission();
}

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
  r[0] = v[0]; r[1] = v[1]; r[2] = v[2];
  r[3] = v[4]; r[4] = v[5]; r[5] = v[6];
  return true;
}

void setup() {
  Serial.begin(115200);
  bus.begin();
  bus.setClock(400000);
  writeReg(0x6B, 0x00);
  writeReg(0x1C, 0x00);
  writeReg(0x1B, 0x00);
  writeReg(0x1A, 0x03);
  delay(100);
}

void loop() {
  int16_t r[6];
  if (!readRaw(r)) { delay(10); return; }

  float a[3], g[3], aRaw[3];
  for (int i = 0; i < 3; i++) {
    a[i]    = (r[i] - ACC_BIAS[i]) / ACC_SCALE[i];
    aRaw[i] = r[i] / 16384.0f;
    g[i]    = (r[3 + i] - GYRO_BIAS[i]) / 131.0f;
  }

#if PLOT_MODE == 1
  Serial.print("ax:"); Serial.print(a[0]);
  Serial.print(",ay:"); Serial.print(a[1]);
  Serial.print(",az:"); Serial.println(a[2]);
#elif PLOT_MODE == 2
  Serial.print("gx:"); Serial.print(g[0]);
  Serial.print(",gy:"); Serial.print(g[1]);
  Serial.print(",gz:"); Serial.println(g[2]);
#else
  float magRaw = sqrt(aRaw[0]*aRaw[0] + aRaw[1]*aRaw[1] + aRaw[2]*aRaw[2]);
  float magCal = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
  Serial.print("raw_mag:"); Serial.print(magRaw, 4);
  Serial.print(",cal_mag:"); Serial.print(magCal, 4);
  Serial.print(",ideal:1.0");
  Serial.print(",hi:"); Serial.print(HI_LIM, 3);
  Serial.print(",lo:"); Serial.println(LO_LIM, 3);
#endif

  delay(20);   // ~50 Hz
}