/*!
 * @file Adafruit_OPT4048.cpp
 *
 * @mainpage Adafruit OPT4048 High Speed High Precision Tristimulus XYZ Color
 * Sensor
 *
 * @section intro_sec Introduction
 *
 * This is a library for the Adafruit OPT4048 breakout board
 * ----> https://www.adafruit.com/products/6334
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * MIT license, all text here must be included in any redistribution
 */

#include "Adafruit_OPT4048.h"

/**
 * @brief Construct a new Adafruit_OPT4048 object.
 */
Adafruit_OPT4048::Adafruit_OPT4048() {
  i2c_dev = nullptr;
}

/**
 * @brief Destroy the Adafruit_OPT4048 object, frees I2C device.
 */
Adafruit_OPT4048::~Adafruit_OPT4048() {
  if (i2c_dev) {
    delete i2c_dev;
    i2c_dev = nullptr;
  }
}

/**
 * @brief Initialize the OPT4048 sensor over I2C.
 *
 * Deletes any existing I2C device instance, then creates a new one.
 *
 * @return true if initialization was successful, false otherwise.
 */
bool Adafruit_OPT4048::begin(uint8_t addr, TwoWire* wire) {
  // Clean up old instance if reinitializing
  if (i2c_dev) {
    delete i2c_dev;
    i2c_dev = nullptr;
  }

  // Create I2C device
  i2c_dev = new Adafruit_I2CDevice(addr, wire);
  if (!i2c_dev) {
    return false;
  }

  if (!i2c_dev->begin()) {
    return false;
  }

  // Verify device ID to ensure correct chip is connected
  {
    Adafruit_BusIO_Register idreg(i2c_dev, OPT4048_REG_DEVICE_ID, 2, MSBFIRST);
    uint16_t id = idreg.read();

    // Default reset device ID is 0x0821
    if (id != 0x0821) {
      return false;
    }
  }

  // Set interrupt direction to default (high threshold active)
  // Even though this is the device default, we set it explicitly for clarity
  setInterruptDirection(true);

  // Set interrupt configuration to "data ready for all channels"
  setInterruptConfig(OPT4048_INT_CFG_DATA_READY_ALL);

  setInterruptLatch(true);    // Use latched interrupts
  setInterruptPolarity(true); // Use active-high interrupts
  setInterruptConfig(OPT4048_INT_CFG_DATA_READY_ALL);

  return true;
}

/**
 * @brief Read all four channels, verify CRC, and return raw ADC code values.
 *
 * Reads registers for channels 0-3 in one burst, checks the CRC bits for each,
 * and computes the 20-bit ADC code = mantissa << exponent.
 *
 * @return true if read succeeds and all CRC checks pass, false otherwise.
 */
bool Adafruit_OPT4048::getChannelsRaw(uint32_t* ch0, uint32_t* ch1,
                                      uint32_t* ch2, uint32_t* ch3) {
  if (!i2c_dev) {
    return false;
  }
  uint8_t buf[16];
  uint8_t reg = OPT4048_REG_CH0_MSB;
  if (!i2c_dev->write_then_read(&reg, 1, buf, sizeof(buf))) {
    return false;
  }

  for (uint8_t ch = 0; ch < 4; ch++) {
    uint8_t exp = (uint16_t)buf[4 * ch] >> 4;
    uint16_t msb = (((uint16_t)(buf[4 * ch] & 0xF)) << 8) | buf[4 * ch + 1];
    uint16_t lsb = ((uint16_t)buf[4 * ch + 2]);
    uint8_t counter = buf[4 * ch + 3] >> 4;
    uint8_t crc = buf[4 * ch + 3] & 0xF;

    uint32_t mant = ((uint32_t)msb << 8) | lsb;

    /*
// Debug output for each channel
Serial.print(F("DEBUG: CH"));
Serial.print(ch);
Serial.print(F(": MSB=0x"));
Serial.print(msb, HEX);
Serial.print(F(", LSB=0x"));
Serial.print(lsb, HEX);
Serial.print(F(", count="));
Serial.print(counter);
Serial.print(F(", crc="));
Serial.print(crc);
Serial.print(F(", exp="));
Serial.print(exp);
Serial.print(F(", mant=0x"));
Serial.println(mant, HEX);
*/

    // Implementing CRC check based on the formula from the datasheet:
    // CRC bits for each channel:
    // R[19:0]=(RESULT_MSB_CH0[11:0]<<8)+RESULT_LSB_CH0[7:0]
    // X[0]=XOR(EXPONENT_CH0[3:0],R[19:0],COUNTER_CHx[3:0]) - XOR of all bits
    // X[1]=XOR(COUNTER_CHx[1],COUNTER_CHx[3],R[1],R[3],R[5],R[7],R[9],R[11],R[13],R[15],R[17],R[19],E[1],E[3])
    // X[2]=XOR(COUNTER_CHx[3],R[3],R[7],R[11],R[15],R[19],E[3])
    // X[3]=XOR(R[3],R[11],R[19])

    // Note: COUNTER_CHx[3:0] is the CRC itself, which creates a circular
    // reference We need to include it in our calculations to match the hardware
    // implementation

    // Initialize CRC variables
    uint8_t x0 = 0; // CRC bit 0
    uint8_t x1 = 0; // CRC bit 1
    uint8_t x2 = 0; // CRC bit 2
    uint8_t x3 = 0; // CRC bit 3

    // Calculate each CRC bit according to the datasheet formula:
    // Calculate bit 0 (x0):
    // X[0]=XOR(EXPONENT_CH0[3:0],R[19:0],COUNTER_CHx[3:0])
    x0 = 0;

    // XOR all exponent bits
    for (uint8_t i = 0; i < 4; i++) {
      x0 ^= (exp >> i) & 1;
    }

    // XOR all mantissa bits
    for (uint8_t i = 0; i < 20; i++) {
      x0 ^= (mant >> i) & 1;
    }

    // XOR all counter (CRC) bits
    for (uint8_t i = 0; i < 4; i++) {
      x0 ^= (counter >> i) & 1;
    }

    // Calculate bit 1 (x1) per datasheet:
    // X[1]=XOR(COUNTER_CHx[1],COUNTER_CHx[3],R[1],R[3],R[5],R[7],R[9],R[11],R[13],R[15],R[17],R[19],E[1],E[3])
    x1 = 0;
    // Include counter bits 1 and 3
    x1 ^= (counter >> 1) & 1; // COUNTER_CHx[1]
    x1 ^= (counter >> 3) & 1; // COUNTER_CHx[3]

    // Include odd-indexed mantissa bits
    for (uint8_t i = 1; i < 20; i += 2) {
      x1 ^= (mant >> i) & 1;
    }

    // Include exponent bits 1 and 3
    x1 ^= (exp >> 1) & 1; // E[1]
    x1 ^= (exp >> 3) & 1; // E[3]

    // Calculate bit 2 (x2) per datasheet:
    // X[2]=XOR(COUNTER_CHx[3],R[3],R[7],R[11],R[15],R[19],E[3])
    x2 = 0;
    // Include counter bit 3
    x2 ^= (counter >> 3) & 1; // COUNTER_CHx[3]

    // Include mantissa bits at positions 3,7,11,15,19
    for (uint8_t i = 3; i < 20; i += 4) {
      x2 ^= (mant >> i) & 1;
    }

    // Include exponent bit 3
    x2 ^= (exp >> 3) & 1; // E[3]

    // Calculate bit 3 (x3) per datasheet:
    // X[3]=XOR(R[3],R[11],R[19])
    x3 = 0;
    // XOR mantissa bits at positions 3, 11, 19
    x3 ^= (mant >> 3) & 1;  // R[3]
    x3 ^= (mant >> 11) & 1; // R[11]
    x3 ^= (mant >> 19) & 1; // R[19]

    // Combine bits to form the CRC
    uint8_t calculated_crc = (x3 << 3) | (x2 << 2) | (x1 << 1) | x0;

    // Verify CRC
    if (crc != calculated_crc) {
      // Serial.print(F("DEBUG: CRC check failed for channel "));
      // Serial.println(ch);
      return false;
    }

    // Convert to 20-bit mantissa << exponent format
    // This is safe because the sensor only uses exponents 0-6 in actual
    // measurements (even when auto-range mode (12) is enabled in the
    // configuration register)
    uint32_t output = (uint32_t)mant << (uint32_t)exp;

    // Assign output
    switch (ch) {
      case 0:
        *ch0 = output;
        break;
      case 1:
        *ch1 = output;
        break;
      case 2:
        *ch2 = output;
        break;
      case 3:
        *ch3 = output;
        break;
    }
  }
  // Serial.println(F("DEBUG: All channel reads successful"));
  return true;
}

/**
 * @brief Get the current low threshold value
 *
 * Reads the low threshold register (0x08) and converts the exponent/mantissa
 * format to a single 32-bit value.
 *
 * As per page 18 of the datasheet, threshold calculation is:
 * ADC_CODES_TL = THRESHOLD_L_RESULT << (8 + THRESHOLD_L_EXPONENT)
 *
 * @return The current low threshold value
 */
uint32_t Adafruit_OPT4048::getThresholdLow(void) {
  if (!i2c_dev) {
    return 0;
  }

  // Create the register object for the threshold register
  Adafruit_BusIO_Register threshold_reg(i2c_dev, OPT4048_REG_THRESHOLD_LOW, 2,
                                        MSBFIRST);

  // Create register bits for the exponent (top 4 bits) and mantissa (lower 12
  // bits)
  Adafruit_BusIO_RegisterBits exponent_bits(&threshold_reg, 4, 12);
  Adafruit_BusIO_RegisterBits mantissa_bits(&threshold_reg, 12, 0);

  // Read the exponent and mantissa
  uint8_t exponent = exponent_bits.read();
  uint32_t mantissa = mantissa_bits.read();

  // Calculate ADC code value by applying the exponent as a bit shift
  // ADD 8 to the exponent as per datasheet equations 12-13
  return mantissa << (8 + exponent);
}

/**
 * @brief Set the low threshold value for interrupt generation
 *
 * Configures the low threshold register (0x08) with the given value in
 * the sensor's exponent/mantissa format.
 *
 * Value is stored as THRESHOLD_L_RESULT and THRESHOLD_L_EXPONENT where:
 * ADC_CODES_TL = THRESHOLD_L_RESULT << (8 + THRESHOLD_L_EXPONENT)
 *
 * @param thl The low threshold value
 * @return true if successful, false otherwise
 */
bool Adafruit_OPT4048::setThresholdLow(uint32_t thl) {
  if (!i2c_dev) {
    return false;
  }

  // Find the appropriate exponent and mantissa values that represent the
  // threshold In this case, we need to find the smallest exponent that allows
  // mantissa to fit in 12 bits
  uint8_t exponent = 0;
  uint32_t mantissa = thl;

  // The mantissa needs to fit in 12 bits, so we start by shifting right
  // to determine how many shifts we need (which gives us the exponent)
  // Note that the threshold registers already have 8 added to exponent
  // internally so we first subtract 8 from our target exponent
  if (mantissa > 0xFFF) { // If value won't fit in 12 bits
    while (mantissa > 0xFFF && exponent < 15) {
      mantissa >>= 1;
      exponent++;
    }
    if (mantissa > 0xFFF) { // If still won't fit with max exponent, clamp
      mantissa = 0xFFF;
      exponent =
          15 - 8; // Max exponent (15) minus the 8 that's added internally
    }
  }

  // Create the register object for the threshold register
  Adafruit_BusIO_Register threshold_reg(i2c_dev, OPT4048_REG_THRESHOLD_LOW, 2,
                                        MSBFIRST);

  // Create register bits for the exponent (top 4 bits) and mantissa (lower 12
  // bits)
  Adafruit_BusIO_RegisterBits exponent_bits(&threshold_reg, 4, 12);
  Adafruit_BusIO_RegisterBits mantissa_bits(&threshold_reg, 12, 0);

  // Write the exponent and mantissa to the register
  exponent_bits.write(exponent);
  mantissa_bits.write(mantissa);

  return true;
}

/**
 * @brief Get the current high threshold value
 *
 * Reads the high threshold register (0x09) and converts the exponent/mantissa
 * format to a single 32-bit value.
 *
 * As per page 18 of the datasheet, threshold calculation is:
 * ADC_CODES_TH = THRESHOLD_H_RESULT << (8 + THRESHOLD_H_EXPONENT)
 *
 * @return The current high threshold value
 */
uint32_t Adafruit_OPT4048::getThresholdHigh(void) {
  if (!i2c_dev) {
    return 0;
  }

  // Create the register object for the threshold register
  Adafruit_BusIO_Register threshold_reg(i2c_dev, OPT4048_REG_THRESHOLD_HIGH, 2,
                                        MSBFIRST);

  // Create register bits for the exponent (top 4 bits) and mantissa (lower 12
  // bits)
  Adafruit_BusIO_RegisterBits exponent_bits(&threshold_reg, 4, 12);
  Adafruit_BusIO_RegisterBits mantissa_bits(&threshold_reg, 12, 0);

  // Read the exponent and mantissa
  uint8_t exponent = exponent_bits.read();
  uint32_t mantissa = mantissa_bits.read();

  // Calculate ADC code value by applying the exponent as a bit shift
  // ADD 8 to the exponent as per datasheet equations 10-11
  return mantissa << (8 + exponent);
}

/**
 * @brief Set the high threshold value for interrupt generation
 *
 * Configures the high threshold register (0x09) with the given value in
 * the sensor's exponent/mantissa format.
 *
 * Value is stored as THRESHOLD_H_RESULT and THRESHOLD_H_EXPONENT where:
 * ADC_CODES_TH = THRESHOLD_H_RESULT << (8 + THRESHOLD_H_EXPONENT)
 *
 * @param thh The high threshold value
 * @return true if successful, false otherwise
 */
bool Adafruit_OPT4048::setThresholdHigh(uint32_t thh) {
  if (!i2c_dev) {
    return false;
  }

  // Find the appropriate exponent and mantissa values that represent the
  // threshold In this case, we need to find the smallest exponent that allows
  // mantissa to fit in 12 bits
  uint8_t exponent = 0;
  uint32_t mantissa = thh;

  // The mantissa needs to fit in 12 bits, so we start by shifting right
  // to determine how many shifts we need (which gives us the exponent)
  // Note that the threshold registers already have 8 added to exponent
  // internally so we first subtract 8 from our target exponent
  if (mantissa > 0xFFF) { // If value won't fit in 12 bits
    while (mantissa > 0xFFF && exponent < 15) {
      mantissa >>= 1;
      exponent++;
    }
    if (mantissa > 0xFFF) { // If still won't fit with max exponent, clamp
      mantissa = 0xFFF;
      exponent =
          15 - 8; // Max exponent (15) minus the 8 that's added internally
    }
  }

  // Create the register object for the threshold register
  Adafruit_BusIO_Register threshold_reg(i2c_dev, OPT4048_REG_THRESHOLD_HIGH, 2,
                                        MSBFIRST);

  // Create register bits for the exponent (top 4 bits) and mantissa (lower 12
  // bits)
  Adafruit_BusIO_RegisterBits exponent_bits(&threshold_reg, 4, 12);
  Adafruit_BusIO_RegisterBits mantissa_bits(&threshold_reg, 12, 0);

  // Write the exponent and mantissa to the register
  exponent_bits.write(exponent);
  mantissa_bits.write(mantissa);

  return true;
}

/**
 * @brief Enable or disable Quick Wake-up feature
 *
 * Controls the QWAKE bit (bit 15) in the configuration register (0x0A).
 * When enabled, the sensor doesn't power down completely in one-shot mode,
 * allowing faster wake-up from standby with lower power consumption.
 *
 * From the datasheet (page 29):
 * "Quick Wake-up from Standby in one shot mode by not powering down all
 * circuits. Applicable only in One-shot mode and helps get out of standby mode
 * faster with penalty in power consumption compared to full standby mode."
 *
 * @param enable True to enable Quick Wake, false to disable
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setQuickWake(bool enable) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bit for the QWAKE bit (bit 15)
  Adafruit_BusIO_RegisterBits qwake_bit(&config_reg, 1, 15);

  // Set the QWAKE bit according to the enable parameter
  return qwake_bit.write(enable);
}

/**
 * @brief Get the current state of the Quick Wake feature
 *
 * Reads the QWAKE bit (bit 15) from the configuration register (0x0A)
 * to determine if Quick Wake is enabled or disabled.
 *
 * @return True if Quick Wake is enabled, false if disabled
 */
bool Adafruit_OPT4048::getQuickWake(void) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bit for the QWAKE bit (bit 15)
  Adafruit_BusIO_RegisterBits qwake_bit(&config_reg, 1, 15);

  // Read the QWAKE bit
  return qwake_bit.read();
}

/**
 * @brief Set the range for light measurements
 *
 * Controls the RANGE field (bits 10-13) in the configuration register (0x0A).
 * Allows setting a fixed range or enabling automatic range selection.
 *
 * From the datasheet (page 29):
 * "Controls the full-scale light level range of the device. The format of this
 * register is same as the EXPONENT register for all values from 0 to 6.
 * 0: 2.2klux
 * 1: 4.5kux
 * 2: 9klux
 * 3: 18klux
 * 4: 36klux
 * 5: 72klux
 * 6: 144klux
 * 12: Auto-Range"
 *
 * @param range The range setting to use from opt4048_range_t enum
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setRange(opt4048_range_t range) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the RANGE field (bits 10-13)
  Adafruit_BusIO_RegisterBits range_bits(&config_reg, 4, 10);

  // Set the RANGE field according to the range parameter
  return range_bits.write(range);
}

/**
 * @brief Get the current range setting
 *
 * Reads the RANGE field (bits 10-13) from the configuration register (0x0A)
 * to determine the current range setting.
 *
 * @return The current range setting as opt4048_range_t enum value
 */
opt4048_range_t Adafruit_OPT4048::getRange(void) {
  if (!i2c_dev) {
    return OPT4048_RANGE_AUTO; // Default to auto-range if no device
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the RANGE field (bits 10-13)
  Adafruit_BusIO_RegisterBits range_bits(&config_reg, 4, 10);

  // Read the RANGE field and return as enum value
  return (opt4048_range_t)range_bits.read();
}

/**
 * @brief Set the conversion time per channel
 *
 * Controls the CONVERSION_TIME field (bits 6-9) in the configuration register
 * (0x0A). This sets how long each channel will take to convert, ranging from
 * 600 microseconds to 800 milliseconds per channel.
 *
 * @param convTime The conversion time setting from opt4048_conversion_time_t
 * enum
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setConversionTime(opt4048_conversion_time_t convTime) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the CONVERSION_TIME field (bits 6-9)
  Adafruit_BusIO_RegisterBits convTime_bits(&config_reg, 4, 6);

  // Set the CONVERSION_TIME field according to the convTime parameter
  return convTime_bits.write(convTime);
}

/**
 * @brief Get the current conversion time setting
 *
 * Reads the CONVERSION_TIME field (bits 6-9) from the configuration register
 * (0x0A) to determine the current conversion time setting.
 *
 * @return The current conversion time setting as opt4048_conversion_time_t enum
 * value
 */
opt4048_conversion_time_t Adafruit_OPT4048::getConversionTime(void) {
  if (!i2c_dev) {
    return OPT4048_CONVERSION_TIME_100MS; // Default to 100ms if no device
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the CONVERSION_TIME field (bits 6-9)
  Adafruit_BusIO_RegisterBits convTime_bits(&config_reg, 4, 6);

  // Read the CONVERSION_TIME field and return as enum value
  return (opt4048_conversion_time_t)convTime_bits.read();
}

/**
 * @brief Set the operating mode of the sensor
 *
 * Controls the OPERATING_MODE field (bits 4-5) in the configuration register
 * (0x0A). This sets the device's operating mode: power-down, one-shot, or
 * continuous.
 *
 * @param mode The operating mode setting from opt4048_mode_t enum
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setMode(opt4048_mode_t mode) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the OPERATING_MODE field (bits 4-5)
  Adafruit_BusIO_RegisterBits mode_bits(&config_reg, 2, 4);

  // Set the OPERATING_MODE field according to the mode parameter
  return mode_bits.write(mode);
}

/**
 * @brief Get the current operating mode setting
 *
 * Reads the OPERATING_MODE field (bits 4-5) from the configuration register
 * (0x0A) to determine the current operating mode.
 *
 * @return The current operating mode as opt4048_mode_t enum value
 */
opt4048_mode_t Adafruit_OPT4048::getMode(void) {
  if (!i2c_dev) {
    return OPT4048_MODE_POWERDOWN; // Default to power-down if no device
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the OPERATING_MODE field (bits 4-5)
  Adafruit_BusIO_RegisterBits mode_bits(&config_reg, 2, 4);

  // Read the OPERATING_MODE field and return as enum value
  return (opt4048_mode_t)mode_bits.read();
}

/**
 * @brief Set the interrupt latch mode
 *
 * Controls the LATCH bit (bit 3) in the configuration register (0x0A).
 * This sets whether interrupts are latched or transparent.
 *
 * When latched (true), the interrupt pin remains active until the flag
 * registers are read, regardless of whether the interrupt condition still
 * exists.
 *
 * When transparent/non-latched (false), the interrupt pin state is updated with
 * each measurement and reflects the current comparison result.
 *
 * @param latch True for latched mode, false for transparent mode
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setInterruptLatch(bool latch) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bit for the LATCH bit (bit 3)
  Adafruit_BusIO_RegisterBits latch_bit(&config_reg, 1, 3);

  // Set the LATCH bit according to the latch parameter
  return latch_bit.write(latch);
}

/**
 * @brief Get the current interrupt latch mode
 *
 * Reads the LATCH bit (bit 3) from the configuration register (0x0A)
 * to determine the current latch mode.
 *
 * @return True if interrupts are latched, false if transparent
 */
bool Adafruit_OPT4048::getInterruptLatch(void) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bit for the LATCH bit (bit 3)
  Adafruit_BusIO_RegisterBits latch_bit(&config_reg, 1, 3);

  // Read the LATCH bit
  return latch_bit.read();
}

/**
 * @brief Set the interrupt pin polarity
 *
 * Controls the INT_POL bit (bit 2) in the configuration register (0x0A).
 * This sets the active state of the interrupt pin.
 *
 * @param activeHigh True for active-high (1 = interrupt active),
 *                   false for active-low (0 = interrupt active)
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setInterruptPolarity(bool activeHigh) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bit for the INT_POL bit (bit 2)
  Adafruit_BusIO_RegisterBits polarity_bit(&config_reg, 1, 2);

  // Set the INT_POL bit according to the activeHigh parameter
  return polarity_bit.write(activeHigh);
}

/**
 * @brief Get the current interrupt pin polarity
 *
 * Reads the INT_POL bit (bit 2) from the configuration register (0x0A)
 * to determine the current interrupt polarity.
 *
 * @return True if interrupts are active-high, false if active-low
 */
bool Adafruit_OPT4048::getInterruptPolarity(void) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bit for the INT_POL bit (bit 2)
  Adafruit_BusIO_RegisterBits polarity_bit(&config_reg, 1, 2);

  // Read the INT_POL bit
  return polarity_bit.read();
}

/**
 * @brief Set the fault count for interrupt generation
 *
 * Controls the FAULT_COUNT field (bits 0-1) in the configuration register
 * (0x0A). This sets how many consecutive measurements must be above/below
 * thresholds before an interrupt is triggered.
 *
 * @param count The fault count setting from opt4048_fault_count_t enum
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setFaultCount(opt4048_fault_count_t count) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the FAULT_COUNT field (bits 0-1)
  Adafruit_BusIO_RegisterBits fault_count_bits(&config_reg, 2, 0);

  // Set the FAULT_COUNT field according to the count parameter
  return fault_count_bits.write(count);
}

/**
 * @brief Get the current fault count setting
 *
 * Reads the FAULT_COUNT field (bits 0-1) from the configuration register (0x0A)
 * to determine the current fault count setting.
 *
 * @return The current fault count setting as opt4048_fault_count_t enum value
 */
opt4048_fault_count_t Adafruit_OPT4048::getFaultCount(void) {
  if (!i2c_dev) {
    return OPT4048_FAULT_COUNT_1; // Default to 1 fault count if no device
  }

  // Create the register object for the configuration register
  Adafruit_BusIO_Register config_reg(i2c_dev, OPT4048_REG_CONFIG, 2, MSBFIRST);

  // Create register bits for the FAULT_COUNT field (bits 0-1)
  Adafruit_BusIO_RegisterBits fault_count_bits(&config_reg, 2, 0);

  // Read the FAULT_COUNT field and return as enum value
  return (opt4048_fault_count_t)fault_count_bits.read();
}

/**
 * @brief Set the channel to be used for threshold comparison
 *
 * Controls the THRESHOLD_CH_SEL field (bits 5-6) in the threshold configuration
 * register (0x0B). This sets which channel's ADC code is compared against the
 * thresholds.
 *
 * @param channel Channel number (0-3) to use for threshold comparison:
 *                0 = Channel 0 (X)
 *                1 = Channel 1 (Y)
 *                2 = Channel 2 (Z)
 *                3 = Channel 3 (W)
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setThresholdChannel(uint8_t channel) {
  if (!i2c_dev || channel > 3) {
    return false;
  }

  // Create the register object for the threshold configuration register
  Adafruit_BusIO_Register thresh_cfg_reg(i2c_dev, OPT4048_REG_THRESHOLD_CFG, 2,
                                         MSBFIRST);

  // Create register bits for the THRESHOLD_CH_SEL field (bits 5-6)
  Adafruit_BusIO_RegisterBits thresh_ch_sel_bits(&thresh_cfg_reg, 2, 5);

  // Set the THRESHOLD_CH_SEL field according to the channel parameter
  return thresh_ch_sel_bits.write(channel);
}

/**
 * @brief Get the channel currently used for threshold comparison
 *
 * Reads the THRESHOLD_CH_SEL field (bits 5-6) from the threshold configuration
 * register (0x0B) to determine which channel is being used for threshold
 * comparison.
 *
 * @return The channel number (0-3) currently used for threshold comparison:
 *         0 = Channel 0 (X)
 *         1 = Channel 1 (Y)
 *         2 = Channel 2 (Z)
 *         3 = Channel 3 (W)
 */
uint8_t Adafruit_OPT4048::getThresholdChannel(void) {
  if (!i2c_dev) {
    return 0; // Default to channel 0 if no device
  }

  // Create the register object for the threshold configuration register
  Adafruit_BusIO_Register thresh_cfg_reg(i2c_dev, OPT4048_REG_THRESHOLD_CFG, 2,
                                         MSBFIRST);

  // Create register bits for the THRESHOLD_CH_SEL field (bits 5-6)
  Adafruit_BusIO_RegisterBits thresh_ch_sel_bits(&thresh_cfg_reg, 2, 5);

  // Read the THRESHOLD_CH_SEL field
  return thresh_ch_sel_bits.read();
}

/**
 * @brief Set the direction of the interrupt generation
 *
 * Controls the INT_DIR bit (bit 4) in the threshold configuration register
 * (0x0B). This sets whether an interrupt is generated when the measured value
 * is below the low threshold or above the high threshold.
 *
 * @param thresholdHighActive True for interrupt when measurement > high
 * threshold, false for interrupt when measurement < low threshold
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setInterruptDirection(bool thresholdHighActive) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the threshold configuration register
  Adafruit_BusIO_Register thresh_cfg_reg(i2c_dev, OPT4048_REG_THRESHOLD_CFG, 2,
                                         MSBFIRST);

  // Create register bit for the INT_DIR bit (bit 4)
  Adafruit_BusIO_RegisterBits int_dir_bit(&thresh_cfg_reg, 1, 4);

  // Set the INT_DIR bit according to the thresholdHighActive parameter
  return int_dir_bit.write(thresholdHighActive);
}

/**
 * @brief Get the current interrupt direction setting
 *
 * Reads the INT_DIR bit (bit 4) from the threshold configuration register
 * (0x0B) to determine the current interrupt direction.
 *
 * @return True if interrupts are generated when measurement > high threshold,
 *         false if interrupts are generated when measurement < low threshold
 */
bool Adafruit_OPT4048::getInterruptDirection(void) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the threshold configuration register
  Adafruit_BusIO_Register thresh_cfg_reg(i2c_dev, OPT4048_REG_THRESHOLD_CFG, 2,
                                         MSBFIRST);

  // Create register bit for the INT_DIR bit (bit 4)
  Adafruit_BusIO_RegisterBits int_dir_bit(&thresh_cfg_reg, 1, 4);

  // Read the INT_DIR bit
  return int_dir_bit.read();
}

/**
 * @brief Set the interrupt configuration
 *
 * Controls the INT_CFG field (bits 2-3) in the threshold configuration register
 * (0x0B). This sets the interrupt mechanism after end of conversion.
 *
 * @param config The interrupt configuration setting from opt4048_int_cfg_t enum
 * @return True if successful, false otherwise
 */
bool Adafruit_OPT4048::setInterruptConfig(opt4048_int_cfg_t config) {
  if (!i2c_dev) {
    return false;
  }

  // Create the register object for the threshold configuration register
  Adafruit_BusIO_Register thresh_cfg_reg(i2c_dev, OPT4048_REG_THRESHOLD_CFG, 2,
                                         MSBFIRST);

  // Create register bits for the INT_CFG field (bits 2-3)
  Adafruit_BusIO_RegisterBits int_cfg_bits(&thresh_cfg_reg, 2, 2);

  // Set the INT_CFG field according to the config parameter
  return int_cfg_bits.write(config);
}

/**
 * @brief Get the current interrupt configuration
 *
 * Reads the INT_CFG field (bits 2-3) from the threshold configuration register
 * (0x0B) to determine the current interrupt configuration.
 *
 * @return The current interrupt configuration as opt4048_int_cfg_t enum value
 */
opt4048_int_cfg_t Adafruit_OPT4048::getInterruptConfig(void) {
  if (!i2c_dev) {
    return OPT4048_INT_CFG_SMBUS_ALERT; // Default to SMBUS Alert if no device
  }

  // Create the register object for the threshold configuration register
  Adafruit_BusIO_Register thresh_cfg_reg(i2c_dev, OPT4048_REG_THRESHOLD_CFG, 2,
                                         MSBFIRST);

  // Create register bits for the INT_CFG field (bits 2-3)
  Adafruit_BusIO_RegisterBits int_cfg_bits(&thresh_cfg_reg, 2, 2);

  // Read the INT_CFG field and return as enum value
  return (opt4048_int_cfg_t)int_cfg_bits.read();
}

/**
 * @brief Get the current status flags
 *
 * Reads the status register (0x0C) to determine the current state of various
 * flags. Reading this register also clears latched interrupt flags.
 *
 * @return 8-bit value where:
 *   - bit 0 (0x01): FLAG_L - Flag low (measurement below threshold)
 *   - bit 1 (0x02): FLAG_H - Flag high (measurement above threshold)
 *   - bit 2 (0x04): CONVERSION_READY_FLAG - Conversion complete
 *   - bit 3 (0x08): OVERLOAD_FLAG - Overflow condition
 */
uint8_t Adafruit_OPT4048::getFlags(void) {
  if (!i2c_dev) {
    return 0;
  }

  // Create the register object for the status register
  Adafruit_BusIO_Register status_reg(i2c_dev, OPT4048_REG_STATUS, 2, MSBFIRST);

  // Read the status register and return the lower byte (contains all flag bits)
  uint16_t status = status_reg.read();
  return status & 0x0F; // Mask to get only the lower 4 bits with the flags
}

/**
 * @brief Calculate CIE chromaticity coordinates and lux from raw sensor values
 *
 * Reads all four channels and calculates CIE x and y chromaticity coordinates
 * and illuminance (lux) using a matrix transformation.
 *
 * @param CIEx Pointer to store the calculated CIE x coordinate
 * @param CIEy Pointer to store the calculated CIE y coordinate
 * @param lux Pointer to store the calculated illuminance in lux
 * @return True if calculation succeeded, false otherwise
 */
bool Adafruit_OPT4048::getCIE(double* CIEx, double* CIEy, double* lux) {
  if (!i2c_dev || !CIEx || !CIEy || !lux) {
    return false;
  }

  // Read all four channels
  uint32_t ch0, ch1, ch2, ch3;
  if (!getChannelsRaw(&ch0, &ch1, &ch2, &ch3)) {
    return false;
  }

  // Matrix multiplication coefficients (from datasheet)
  const double m0x = 2.34892992e-04;
  const double m0y = -1.89652390e-05;
  const double m0z = 1.20811684e-05;
  const double m0l = 0;

  const double m1x = 4.07467441e-05;
  const double m1y = 1.98958202e-04;
  const double m1z = -1.58848115e-05;
  const double m1l = 2.15e-3;

  const double m2x = 9.28619404e-05;
  const double m2y = -1.69739553e-05;
  const double m2z = 6.74021520e-04;
  const double m2l = 0;

  const double m3x = 0;
  const double m3y = 0;
  const double m3z = 0;
  const double m3l = 0;

  // The equation from the datasheet is a matrix multiplication:
  // [ch0 ch1 ch2 ch3] * [m0x m0y m0z m0l] = [X Y Z Lux]
  //                     [m1x m1y m1z m1l]
  //                     [m2x m2y m2z m2l]
  //                     [m3x m3y m3z m3l]
  double X = ch0 * m0x + ch1 * m1x + ch2 * m2x + ch3 * m3x;
  double Y = ch0 * m0y + ch1 * m1y + ch2 * m2y + ch3 * m3y;
  double Z = ch0 * m0z + ch1 * m1z + ch2 * m2z + ch3 * m3z;
  double L = ch0 * m0l + ch1 * m1l + ch2 * m2l + ch3 * m3l;

  // Set illuminance in lux
  *lux = L;

  // Calculate CIE x, y chromaticity coordinates
  double sum = X + Y + Z;
  if (sum <= 0) {
    // Avoid division by zero
    *CIEx = 0;
    *CIEy = 0;
    *lux = 0;
    return false;
  }

  *CIEx = X / sum;
  *CIEy = Y / sum;

  return true;
}

/**
 * @brief Calculate the correlated color temperature (CCT) in Kelvin
 *
 * Uses McCamy's approximation formula to calculate CCT from CIE 1931 x,y
 * coordinates. This is accurate for color temperatures between 2000K and
 * 30000K.
 *
 * Formula:
 * n = (x - 0.3320) / (0.1858 - y)
 * CCT = 437 * n^3 + 3601 * n^2 + 6861 * n + 5517
 *
 * @return The calculated color temperature in Kelvin
 */
double Adafruit_OPT4048::calculateColorTemperature(double CIEx, double CIEy) {
  // Check for invalid coordinates
  if (CIEx == 0 && CIEy == 0) {
    return 0.0;
  }

  // Calculate using McCamy's formula from spreadsheet
  // n = (x - 0.3320) / (0.1858 - y)
  double n = (CIEx - 0.3320) / (0.1858 - CIEy);

  // CCT = 437 * n^3 + 3601 * n^2 + 6861 * n + 5517
  double cct = (437.0 * n * n * n) + (3601.0 * n * n) + (6861.0 * n) + 5517.0;

  return cct;
}
