#ifndef MTS4Z_H
#define MTS4Z_H

#include <Wire.h>

#define MTS4Z_ADDR     0x41
#define TEMP_CMD_REG   0x04
#define TEMP_CFG_REG   0x05
#define TEMP_LSB_REG   0x00
#define TEMP_MSB_REG   0x01
#define STATUS_REG     0x03
#define RESET_REG      0x17

class MTS4Z {
  public:
    MTS4Z(TwoWire &wire);

    bool begin();
    float readTemperature();
    void calibrate(float referenceTemp);
    float getCalibratedTemperature();
    void setTemperatureOffset(float offset);  // New method to set the offset

  private:
    TwoWire &_wire;
    float temp_offset = 0.0;  // Calibration offset
    bool calibrated_flag = false;

    void reset();
    void configure();
};

#endif // MTS4Z_H
