#include <Arduino.h>
#include "MTS4Z.h"

MTS4Z::MTS4Z(TwoWire &wire) : _wire(wire) {}

bool MTS4Z::begin() {
  _wire.beginTransmission(MTS4Z_ADDR);
  if (_wire.endTransmission() != 0) {
    Serial.println("❌ MTS4Z not found!");
    return false;
  }
  reset();
  configure();
  return true;
}

float MTS4Z::readTemperature() {
  _wire.beginTransmission(MTS4Z_ADDR);
  _wire.write(TEMP_MSB_REG);
  _wire.endTransmission(false);
  _wire.requestFrom(MTS4Z_ADDR, 2);

  if (_wire.available() == 2) {
    uint8_t msb = _wire.read();
    uint8_t lsb = _wire.read();
    int16_t raw = (msb << 8) | lsb;
    float temp = raw / 256.0;

    // Apply temperature offset (calibration correction)
    temp += temp_offset;

    return temp;
  }
  Serial.println("❌ Failed to read temperature.");
  return 0.0;
}

void MTS4Z::calibrate(float referenceTemp) {
  float rawTemp = readTemperature();
  temp_offset = referenceTemp - rawTemp;
  calibrated_flag = true;

  Serial.print("✅ Calibrated. Offset = ");
  Serial.println(temp_offset, 2);
}

float MTS4Z::getCalibratedTemperature() {
  float temp = readTemperature();
  if (calibrated_flag) {
    temp += temp_offset;
  }
  return temp;
}

void MTS4Z::setTemperatureOffset(float offset) {
  temp_offset = offset;  // Set the offset value
}

void MTS4Z::reset() {
  _wire.beginTransmission(MTS4Z_ADDR);
  _wire.write(RESET_REG);
  _wire.write(0x6A);
  _wire.endTransmission();
  delay(100);
}

void MTS4Z::configure() {
  _wire.beginTransmission(MTS4Z_ADDR);
  _wire.write(TEMP_CMD_REG);
  _wire.write(0x00);
  _wire.endTransmission();

  _wire.beginTransmission(MTS4Z_ADDR);
  _wire.write(TEMP_CFG_REG);
  _wire.write(0x68);
  _wire.endTransmission();
}
