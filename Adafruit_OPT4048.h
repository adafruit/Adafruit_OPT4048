/*!
 * @file Adafruit_OPT4048.h
 *
 * Arduino library for the OPT4048 High Speed High Precision Tristimulus XYZ
 * Color Sensor.
 *
 * This is a library for the Adafruit OPT4048 breakout
 * ----> https://www.adafruit.com/products/6334
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * MIT license, all text here must be included in any redistribution.
 *
 */

#ifndef ADAFRUIT_OPT4048_H
#define ADAFRUIT_OPT4048_H

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Wire.h>

#include "Arduino.h"

#define OPT4048_DEFAULT_ADDR \
  0x44 //!< Default I2C address (ADDR pin connected to GND)

/**
 * @brief Available range settings for the OPT4048 sensor
 *
 * Full-scale light level ranges as described in datasheet page 29.
 */
typedef enum {
  OPT4048_RANGE_2K_LUX = 0,   ///< 2.2 klux
  OPT4048_RANGE_4K_LUX = 1,   ///< 4.5 klux
  OPT4048_RANGE_9K_LUX = 2,   ///< 9 klux
  OPT4048_RANGE_18K_LUX = 3,  ///< 18 klux
  OPT4048_RANGE_36K_LUX = 4,  ///< 36 klux
  OPT4048_RANGE_72K_LUX = 5,  ///< 72 klux
  OPT4048_RANGE_144K_LUX = 6, ///< 144 klux
  OPT4048_RANGE_AUTO = 12     ///< Auto-range
} opt4048_range_t;

/**
 * @brief Available conversion time settings for the OPT4048 sensor
 *
 * These control the device conversion time per channel as described in
 * datasheet page 29.
 */
typedef enum {
  OPT4048_CONVERSION_TIME_600US = 0,  ///< 600 microseconds
  OPT4048_CONVERSION_TIME_1MS = 1,    ///< 1 millisecond
  OPT4048_CONVERSION_TIME_1_8MS = 2,  ///< 1.8 milliseconds
  OPT4048_CONVERSION_TIME_3_4MS = 3,  ///< 3.4 milliseconds
  OPT4048_CONVERSION_TIME_6_5MS = 4,  ///< 6.5 milliseconds
  OPT4048_CONVERSION_TIME_12_7MS = 5, ///< 12.7 milliseconds
  OPT4048_CONVERSION_TIME_25MS = 6,   ///< 25 milliseconds
  OPT4048_CONVERSION_TIME_50MS = 7,   ///< 50 milliseconds
  OPT4048_CONVERSION_TIME_100MS = 8,  ///< 100 milliseconds
  OPT4048_CONVERSION_TIME_200MS = 9,  ///< 200 milliseconds
  OPT4048_CONVERSION_TIME_400MS = 10, ///< 400 milliseconds
  OPT4048_CONVERSION_TIME_800MS = 11  ///< 800 milliseconds
} opt4048_conversion_time_t;

/**
 * @brief Available operating mode settings for the OPT4048 sensor
 *
 * Controls the device mode of operation as described in datasheet page 29.
 */
typedef enum {
  OPT4048_MODE_POWERDOWN = 0,    ///< Power-down mode
  OPT4048_MODE_AUTO_ONESHOT = 1, ///< Forced auto-range one-shot mode
  OPT4048_MODE_ONESHOT = 2,      ///< One-shot mode
  OPT4048_MODE_CONTINUOUS = 3    ///< Continuous mode
} opt4048_mode_t;

/**
 * @brief Available fault count settings for the OPT4048 sensor
 *
 * Controls how many consecutive fault events are needed to trigger an
 * interrupt.
 */
typedef enum {
  OPT4048_FAULT_COUNT_1 = 0, ///< 1 fault count (default)
  OPT4048_FAULT_COUNT_2 = 1, ///< 2 consecutive fault counts
  OPT4048_FAULT_COUNT_4 = 2, ///< 4 consecutive fault counts
  OPT4048_FAULT_COUNT_8 = 3  ///< 8 consecutive fault counts
} opt4048_fault_count_t;

/**
 * @brief Interrupt configuration settings for the OPT4048 sensor
 *
 * Controls the interrupt mechanism after end of conversion.
 */
typedef enum {
  OPT4048_INT_CFG_SMBUS_ALERT = 0,     ///< SMBUS Alert
  OPT4048_INT_CFG_DATA_READY_NEXT = 1, ///< INT Pin data ready for next channel
  OPT4048_INT_CFG_DATA_READY_ALL = 3   ///< INT Pin data ready for all channels
} opt4048_int_cfg_t;

// Register addresses
#define OPT4048_REG_CH0_MSB 0x00        //!< X channel MSB register
#define OPT4048_REG_CH0_LSB 0x01        //!< X channel LSB register
#define OPT4048_REG_CH1_MSB 0x02        //!< Y channel MSB register
#define OPT4048_REG_CH1_LSB 0x03        //!< Y channel LSB register
#define OPT4048_REG_CH2_MSB 0x04        //!< Z channel MSB register
#define OPT4048_REG_CH2_LSB 0x05        //!< Z channel LSB register
#define OPT4048_REG_CH3_MSB 0x06        //!< W channel MSB register
#define OPT4048_REG_CH3_LSB 0x07        //!< W channel LSB register
#define OPT4048_REG_THRESHOLD_LOW 0x08  //!< Low threshold register
#define OPT4048_REG_THRESHOLD_HIGH 0x09 //!< High threshold register
#define OPT4048_REG_CONFIG 0x0A         //!< Configuration register
#define OPT4048_REG_THRESHOLD_CFG 0x0B  //!< Threshold configuration register
#define OPT4048_REG_STATUS 0x0C         //!< Status register
#define OPT4048_REG_DEVICE_ID 0x11      //!< Device ID register

// Status register (0x0C) bit flags
#define OPT4048_FLAG_L 0x01 //!< Flag low - measurement smaller than threshold
#define OPT4048_FLAG_H 0x02 //!< Flag high - measurement larger than threshold
#define OPT4048_FLAG_CONVERSION_READY 0x04 //!< Conversion ready
#define OPT4048_FLAG_OVERLOAD 0x08         //!< Overflow condition

/**
  @brief  Class that stores state and functions for interacting with the OPT4048
  sensor.
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
  bool begin(uint8_t addr = OPT4048_DEFAULT_ADDR, TwoWire* wire = &Wire);

  /**
   * @brief Read all four channels, verify CRC, and return raw ADC code values
   *
   * @param ch0 Pointer to store channel 0 (X) value
   * @param ch1 Pointer to store channel 1 (Y) value
   * @param ch2 Pointer to store channel 2 (Z) value
   * @param ch3 Pointer to store channel 3 (W) value
   * @return true if successful, false otherwise
   */
  bool getChannelsRaw(uint32_t* ch0, uint32_t* ch1, uint32_t* ch2,
                      uint32_t* ch3);

  bool setThresholdLow(uint32_t thl);
  uint32_t getThresholdLow(void);
  bool setThresholdHigh(uint32_t thh);
  uint32_t getThresholdHigh(void);
  bool setQuickWake(bool enable);
  bool getQuickWake(void);
  bool setRange(opt4048_range_t range);
  opt4048_range_t getRange(void);
  bool setConversionTime(opt4048_conversion_time_t convTime);
  opt4048_conversion_time_t getConversionTime(void);
  bool setMode(opt4048_mode_t mode);
  opt4048_mode_t getMode(void);
  bool setInterruptLatch(bool latch);
  bool getInterruptLatch(void);
  bool setInterruptPolarity(bool activeHigh);
  bool getInterruptPolarity(void);
  bool setFaultCount(opt4048_fault_count_t count);
  opt4048_fault_count_t getFaultCount(void);
  bool setThresholdChannel(uint8_t channel);
  uint8_t getThresholdChannel(void);
  bool setInterruptDirection(bool activeHigh);
  bool getInterruptDirection(void);
  bool setInterruptConfig(opt4048_int_cfg_t config);
  opt4048_int_cfg_t getInterruptConfig(void);
  uint8_t getFlags(void);
  bool getCIE(double* CIEx, double* CIEy, double* lux);

  /**
   * @brief Calculate the correlated color temperature (CCT) in Kelvin
   *
   * Uses the McCamy's approximation formula to calculate CCT from CIE 1931 x,y
   * coordinates. This is accurate for color temperatures between 2000K and
   * 30000K.
   *
   * @param CIEx The CIE x chromaticity coordinate
   * @param CIEy The CIE y chromaticity coordinate
   * @return The calculated color temperature in Kelvin
   */
  double calculateColorTemperature(double CIEx, double CIEy);

 private:
  Adafruit_I2CDevice* i2c_dev;
  void encodeValue(uint32_t value, uint8_t* exp, uint32_t* mant);
};

#endif // ADAFRUIT_OPT4048_H
