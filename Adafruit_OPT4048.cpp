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
 * @param addr I2C 7-bit address of the sensor (default OPT4048_DEFAULT_ADDR).
 * @param wire Pointer to TwoWire instance (default &Wire).
 * @return true if initialization was successful, false otherwise.
 */
bool Adafruit_OPT4048::begin(uint8_t addr, TwoWire *wire) {
  // Clean up old instance if reinitializing
  if (i2c_dev) {
    delete i2c_dev;
    i2c_dev = nullptr;
  }
  // Create I2C device
  i2c_dev = new Adafruit_I2CDevice(addr, wire);
  if (!i2c_dev || !i2c_dev->begin()) {
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
  return true;
}
/**
 * @brief Read all four channels, verify CRC, and return raw ADC code values.
 *
 * Reads registers for channels 0-3 in one burst, checks the CRC bits for each,
 * and computes the 20-bit ADC code = mantissa << exponent.
 *
 * @param ch0 Pointer to store channel 0 ADC code.
 * @param ch1 Pointer to store channel 1 ADC code.
 * @param ch2 Pointer to store channel 2 ADC code.
 * @param ch3 Pointer to store channel 3 ADC code.
 * @return true if read succeeds and all CRC checks pass, false otherwise.
 */
bool Adafruit_OPT4048::getChannels(float *ch0, float *ch1, float *ch2, float *ch3) {
  if (!i2c_dev) {
    return false;
  }
  uint8_t buf[16];
  uint8_t reg = OPT4048_REG_CH0_MSB;
  if (!i2c_dev->write_then_read(&reg, 1, buf, sizeof(buf))) {
    return false;
  }
  for (uint8_t ch = 0; ch < 4; ch++) {
    uint16_t msb = ((uint16_t)buf[4 * ch] << 8) | buf[4 * ch + 1];
    uint16_t lsb = ((uint16_t)buf[4 * ch + 2] << 8) | buf[4 * ch + 3];
    uint8_t exp = (msb >> 12) & 0x0F;
    uint32_t mant = ((msb & 0x0FFF) << 8) | ((lsb >> 8) & 0xFF);
    uint32_t code = mant << exp;
    uint8_t crc = lsb & 0x0F;
    // Compute CRC bits
    uint8_t x0 = 0;
    for (uint8_t i = 0; i < 4; i++) {
      x0 ^= (exp >> i) & 1;
    }
    for (uint8_t i = 0; i < 20; i++) {
      x0 ^= (mant >> i) & 1;
    }
    for (uint8_t i = 0; i < 4; i++) {
      x0 ^= (crc >> i) & 1;
    }
    uint8_t x1 = ((crc >> 1) & 1) ^ ((crc >> 3) & 1);
    for (uint8_t i = 1; i < 20; i += 2) {
      x1 ^= (mant >> i) & 1;
    }
    x1 ^= (exp >> 1) & 1;
    x1 ^= (exp >> 3) & 1;
    uint8_t x2 = ((crc >> 3) & 1);
    for (uint8_t i = 3; i < 20; i += 4) {
      x2 ^= (mant >> i) & 1;
    }
    x2 ^= (exp >> 3) & 1;
    uint8_t x3 = ((mant >> 3) & 1) ^ ((mant >> 11) & 1) ^ ((mant >> 19) & 1);
    // Verify CRC
    if (((crc & 1) != x0) || (((crc >> 1) & 1) != x1) || (((crc >> 2) & 1) != x2) || (((crc >> 3) & 1) != x3)) {
      return false;
    }
    // Assign output
    switch (ch) {
      case 0: *ch0 = (float)code; break;
      case 1: *ch1 = (float)code; break;
      case 2: *ch2 = (float)code; break;
      case 3: *ch3 = (float)code; break;
    }
  }
  return true;
}