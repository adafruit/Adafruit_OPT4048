/*
  This is a library for the OPT4048 ambient light sensor
  Designed to work with Adafruit BusIO: https://github.com/adafruit/Adafruit_BusIO

  Written by Adafruit Industries, 2025.
*/

#ifndef ADAFRUIT_OPT4048_H
#define ADAFRUIT_OPT4048_H

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>

// Default I2C address (ADDR pin connected to GND)
#define OPT4048_DEFAULT_ADDR        0x44

// Register addresses
#define OPT4048_REG_CH0_MSB         0x00
#define OPT4048_REG_CH0_LSB         0x01
#define OPT4048_REG_CH1_MSB         0x02
#define OPT4048_REG_CH1_LSB         0x03
#define OPT4048_REG_CH2_MSB         0x04
#define OPT4048_REG_CH2_LSB         0x05
#define OPT4048_REG_CH3_MSB         0x06
#define OPT4048_REG_CH3_LSB         0x07
#define OPT4048_REG_THRESHOLD_LOW   0x08
#define OPT4048_REG_THRESHOLD_HIGH  0x09
#define OPT4048_REG_CONFIG          0x0A
#define OPT4048_REG_THRESHOLD_CFG   0x0B
#define OPT4048_REG_STATUS          0x0C
#define OPT4048_REG_DEVICE_ID       0x11

/**
  @brief  Class that stores state and functions for interacting with the OPT4048 sensor.
*/
class Adafruit_OPT4048 {
public:
  Adafruit_OPT4048();
  ~Adafruit_OPT4048();

  /**
   * @brief  Initialize the sensor with given I2C address and Wire instance
   *
   * @param  addr I2C address, defaults to OPT4048_DEFAULT_ADDR
   * @param  wire Pointer to TwoWire instance, defaults to &Wire
   * @return true on success, false on failure
   */
  bool begin(uint8_t addr = OPT4048_DEFAULT_ADDR, TwoWire *wire = &Wire);
  /** Read all four channels, verify CRC, and return raw ADC code values */
  bool getChannels(float *ch0, float *ch1, float *ch2, float *ch3);

private:
  Adafruit_I2CDevice *i2c_dev;
};

#endif // ADAFRUIT_OPT4048_H