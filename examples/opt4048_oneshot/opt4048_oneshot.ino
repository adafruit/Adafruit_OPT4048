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

Adafruit_OPT4048 sensor;

// we'll track time between data reads
uint32_t timestamp = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Wait for serial monitor to open
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("Adafruit OPT4048 Tristimulus XYZ Color Sensor Test"));

  // Initialize the sensor
  if (!sensor.begin()) {
    Serial.println(F("Failed to find OPT4048 chip"));
    while (1) {
      delay(10);
    }
  }
  
  Serial.println(F("OPT4048 sensor found!"));

  sensor.setRange(OPT4048_RANGE_AUTO);  // Set range to auto
  sensor.setConversionTime(OPT4048_CONVERSION_TIME_200MS); // Set conversion time to 200ms
  sensor.setMode(OPT4048_MODE_AUTO_ONESHOT);  // Set operating mode to auto-range one shot

  timestamp = millis();
}


void loop() {
  if (sensor.getMode() == OPT4048_MODE_POWERDOWN) {
    // ok we finished the reading!
    double CIEx, CIEy, lux;
  
    if (! sensor.getCIE(&CIEx, &CIEy, &lux)) {
      Serial.println(F("Error reading sensor data"));
    } else {
      Serial.println(F("\nCIE Coordinates:"));
      Serial.print(F("CIE x: ")); Serial.println(CIEx, 8);
      Serial.print(F("CIE y: ")); Serial.println(CIEy, 8);
      Serial.print(F("Lux: ")); Serial.println(lux, 4);
    
      // Calculate and display color temperature
      double colorTemp = sensor.calculateColorTemperature(CIEx, CIEy);
      Serial.print(F("Color Temperature: "));
      Serial.print(colorTemp, 2);
      Serial.println(F(" K"));
      Serial.print("Time since last read: "); 
      Serial.print(millis() - timestamp);
      Serial.println("ms");
      timestamp = millis();
    }

    // start a new reading!
    sensor.setMode(OPT4048_MODE_AUTO_ONESHOT);  // Set operating mode to auto-range one shot
  }

  delay(10); // a small delay
}
