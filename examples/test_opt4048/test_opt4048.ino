/*!
 * @file test_opt4048.ino
 *
 * A basic demo for using the OPT4048 tristimulus XYZ color sensor
 * 
 * This example reads the sensor values from all four channels (X, Y, Z, W),
 * demonstrates setting and getting threshold values, and displays the results.
 */

#include <Wire.h>
#include "Adafruit_OPT4048.h"

// Create sensor object
Adafruit_OPT4048 sensor;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Wait for serial monitor to open
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("Adafruit OPT4048 Tristimulus XYZ Color Sensor Test");

  // Initialize the sensor
  if (!sensor.begin()) {
    Serial.println("Failed to find OPT4048 chip");
    while (1) {
      delay(10);
    }
  }
  
  Serial.println("OPT4048 sensor found!");

  // Set low and high thresholds for interrupts
  uint32_t lowThreshold = 1000;
  uint32_t highThreshold = 10000;

  Serial.print("Setting low threshold to: ");
  Serial.println(lowThreshold);
  sensor.setThresholdLow(lowThreshold);

  Serial.print("Setting high threshold to: ");
  Serial.println(highThreshold);
  sensor.setThresholdHigh(highThreshold);

  // Read back the thresholds to verify
  uint32_t readLowThreshold = sensor.getThresholdLow();
  uint32_t readHighThreshold = sensor.getThresholdHigh();

  Serial.print("Read back low threshold: ");
  Serial.println(readLowThreshold);
  Serial.print("Read back high threshold: ");
  Serial.println(readHighThreshold);

  // Enable Quick Wake feature
  Serial.println("\nEnabling Quick Wake feature...");
  sensor.setQuickWake(true);

  // Read back Quick Wake status
  bool quickWakeStatus = sensor.getQuickWake();
  Serial.print("Quick Wake status: ");
  Serial.println(quickWakeStatus ? "Enabled" : "Disabled");

  // Set range to auto
  Serial.println("\nSetting range to Auto...");
  sensor.setRange(OPT4048_RANGE_AUTO);

  // Read back range setting
  opt4048_range_t currentRange = sensor.getRange();
  Serial.print("Current range setting: ");
  switch (currentRange) {
    case OPT4048_RANGE_2K_LUX:   Serial.println("2.2 klux"); break;
    case OPT4048_RANGE_4K_LUX:   Serial.println("4.5 klux"); break;
    case OPT4048_RANGE_9K_LUX:   Serial.println("9 klux"); break;
    case OPT4048_RANGE_18K_LUX:  Serial.println("18 klux"); break;
    case OPT4048_RANGE_36K_LUX:  Serial.println("36 klux"); break;
    case OPT4048_RANGE_72K_LUX:  Serial.println("72 klux"); break;
    case OPT4048_RANGE_144K_LUX: Serial.println("144 klux"); break;
    case OPT4048_RANGE_AUTO:     Serial.println("Auto"); break;
    default:                      Serial.println("Unknown"); break;
  }

  // Set conversion time to 100ms
  Serial.println("\nSetting conversion time to 100ms...");
  sensor.setConversionTime(OPT4048_CONVERSION_TIME_100MS);

  // Read back conversion time setting
  opt4048_conversion_time_t currentConvTime = sensor.getConversionTime();
  Serial.print("Current conversion time setting: ");
  switch (currentConvTime) {
    case OPT4048_CONVERSION_TIME_600US:  Serial.println("600 microseconds"); break;
    case OPT4048_CONVERSION_TIME_1MS:    Serial.println("1 millisecond"); break;
    case OPT4048_CONVERSION_TIME_1_8MS:  Serial.println("1.8 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_3_4MS:  Serial.println("3.4 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_6_5MS:  Serial.println("6.5 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_12_7MS: Serial.println("12.7 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_25MS:   Serial.println("25 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_50MS:   Serial.println("50 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_100MS:  Serial.println("100 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_200MS:  Serial.println("200 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_400MS:  Serial.println("400 milliseconds"); break;
    case OPT4048_CONVERSION_TIME_800MS:  Serial.println("800 milliseconds"); break;
    default:                              Serial.println("Unknown"); break;
  }

  // Set operating mode to continuous
  Serial.println("\nSetting operating mode to Continuous...");
  sensor.setMode(OPT4048_MODE_CONTINUOUS);

  // Read back operating mode setting
  opt4048_mode_t currentMode = sensor.getMode();
  Serial.print("Current operating mode: ");
  switch (currentMode) {
    case OPT4048_MODE_POWERDOWN:    Serial.println("Power-down"); break;
    case OPT4048_MODE_AUTO_ONESHOT: Serial.println("Forced auto-range one-shot"); break;
    case OPT4048_MODE_ONESHOT:      Serial.println("One-shot"); break;
    case OPT4048_MODE_CONTINUOUS:   Serial.println("Continuous"); break;
    default:                         Serial.println("Unknown"); break;
  }

  // Configure interrupt settings
  Serial.println("\nConfiguring interrupt settings...");
  sensor.setInterruptLatch(true);     // Use latched interrupts
  sensor.setInterruptPolarity(true);  // Use active-high interrupts

  // Read back interrupt settings
  bool latchMode = sensor.getInterruptLatch();
  bool polarityMode = sensor.getInterruptPolarity();

  Serial.print("Interrupt latch mode: ");
  Serial.println(latchMode ? "Latched" : "Transparent");

  Serial.print("Interrupt polarity: ");
  Serial.println(polarityMode ? "Active-high" : "Active-low");

  // Configure fault count
  Serial.println("\nSetting fault count to 4 consecutive faults...");
  sensor.setFaultCount(OPT4048_FAULT_COUNT_4);

  // Read back fault count setting
  opt4048_fault_count_t faultCount = sensor.getFaultCount();
  Serial.print("Fault count setting: ");
  switch (faultCount) {
    case OPT4048_FAULT_COUNT_1: Serial.println("1 fault count"); break;
    case OPT4048_FAULT_COUNT_2: Serial.println("2 consecutive fault counts"); break;
    case OPT4048_FAULT_COUNT_4: Serial.println("4 consecutive fault counts"); break;
    case OPT4048_FAULT_COUNT_8: Serial.println("8 consecutive fault counts"); break;
    default:                     Serial.println("Unknown"); break;
  }

  // Configure threshold channel
  Serial.println("\nSetting threshold channel to Channel 1 (Y)...");
  sensor.setThresholdChannel(1);

  // Read back threshold channel setting
  uint8_t threshChannel = sensor.getThresholdChannel();
  Serial.print("Threshold channel setting: Channel ");
  Serial.print(threshChannel);
  switch (threshChannel) {
    case 0: Serial.println(" (X)"); break;
    case 1: Serial.println(" (Y)"); break;
    case 2: Serial.println(" (Z)"); break;
    case 3: Serial.println(" (W)"); break;
    default: Serial.println(" (Unknown)"); break;
  }

  // Configure interrupt configuration
  Serial.println("\nSetting interrupt configuration to data ready for all channels...");
  sensor.setInterruptConfig(OPT4048_INT_CFG_DATA_READY_ALL);

  // Read back interrupt configuration setting
  opt4048_int_cfg_t intConfig = sensor.getInterruptConfig();
  Serial.print("Interrupt configuration: ");
  switch (intConfig) {
    case OPT4048_INT_CFG_SMBUS_ALERT: Serial.println("SMBUS Alert"); break;
    case OPT4048_INT_CFG_DATA_READY_NEXT: Serial.println("INT Pin data ready for next channel"); break;
    case OPT4048_INT_CFG_DATA_READY_ALL: Serial.println("INT Pin data ready for all channels"); break;
    default: Serial.println("Unknown"); break;
  }
  Serial.println();
}

void loop() {
  uint32_t x, y, z, w;

  // Read all four channels from the sensor (raw ADC values)
  if (sensor.getChannelsRaw(&x, &y, &z, &w)) {
    Serial.println("Channel readings (raw values):");
    Serial.print("X (CH0): "); Serial.println(x);
    Serial.print("Y (CH1): "); Serial.println(y);
    Serial.print("Z (CH2): "); Serial.println(z);
    Serial.print("W (CH3): "); Serial.println(w);

    // Read and print status flags
    uint8_t flags = sensor.getFlags();
    Serial.println("\nStatus Flags:");
    if (flags & OPT4048_FLAG_L) {
      Serial.println("- Measurement below low threshold");
    }
    if (flags & OPT4048_FLAG_H) {
      Serial.println("- Measurement above high threshold");
    }
    if (flags & OPT4048_FLAG_CONVERSION_READY) {
      Serial.println("- Conversion complete");
    }
    if (flags & OPT4048_FLAG_OVERLOAD) {
      Serial.println("- Overflow condition detected");
    }
    if (flags == 0) {
      Serial.println("- No flags set");
    }

    // Calculate and display CIE chromaticity coordinates and lux
    double CIEx, CIEy, lux;
    if (sensor.getCIE(&CIEx, &CIEy, &lux)) {
      Serial.println("\nCIE Coordinates:");
      Serial.print("CIE x: "); Serial.println(CIEx, 8);
      Serial.print("CIE y: "); Serial.println(CIEy, 8);
      Serial.print("Lux: "); Serial.println(lux, 4);
    } else {
      Serial.println("\nError calculating CIE coordinates");
    }
    Serial.println();
  } else {
    Serial.println("Error reading sensor data");
  }

  delay(1000); // Read once per second
}