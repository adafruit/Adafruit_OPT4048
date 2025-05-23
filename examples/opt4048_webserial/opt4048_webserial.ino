/*!
 * @file opt4048_webserial.ino
 * 
 * This example reads color data from the OPT4048 sensor and outputs it
 * in a format suitable for displaying on a web page using Web Serial API.
 * 
 * It continuously measures CIE x,y coordinates, lux, and color temperature.
 * 
 * This sketch works with the web interface in the /webserial directory,
 * which can be accessed at: https://adafruit.github.io/Adafruit_OPT4048/webserial/
 * Code for web app is in gh-pages branch: https://github.com/adafruit/Adafruit_OPT4048/tree/gh-pages
 */

#include <Wire.h>
#include "Adafruit_OPT4048.h"

// Create sensor object
Adafruit_OPT4048 sensor;

// Set how often to read data (in milliseconds)
const unsigned long READ_INTERVAL = 100;
unsigned long lastReadTime = 0;

void setup() {
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  
  // Wait briefly for serial to connect (not needed for all boards)
  delay(100);

  Serial.println(F("Adafruit OPT4048 WebSerial Example"));
  Serial.println(F("This sketch works with the OPT4048 CIE Color Plotter web page"));

  // Initialize the sensor
  if (!sensor.begin()) {
    Serial.println(F("Failed to find OPT4048 chip"));
    while (1) {
      delay(10);
    }
  }
  
  Serial.println(F("OPT4048 sensor found!"));

  // Set sensor configuration
  sensor.setRange(OPT4048_RANGE_AUTO);           // Auto-range for best results across lighting conditions
  sensor.setConversionTime(OPT4048_CONVERSION_TIME_100MS); // 100ms conversion time
  sensor.setMode(OPT4048_MODE_CONTINUOUS);       // Continuous mode
}

void loop() {
  // Only read at the specified interval
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;
    
    // Calculate and display CIE chromaticity coordinates and lux
    double CIEx, CIEy, lux;
    if (sensor.getCIE(&CIEx, &CIEy, &lux)) {
      // Print the values in a format that can be easily parsed by the web page
      Serial.println(F("---CIE Data---"));
      Serial.print(F("CIE x: ")); Serial.println(CIEx, 8);
      Serial.print(F("CIE y: ")); Serial.println(CIEy, 8);
      Serial.print(F("Lux: ")); Serial.println(lux, 4);

      // Calculate and display color temperature
      double colorTemp = sensor.calculateColorTemperature(CIEx, CIEy);
      Serial.print(F("Color Temperature: "));
      Serial.print(colorTemp, 2);
      Serial.println(F(" K"));
      Serial.println(F("-------------"));
    } else {
      Serial.println(F("Error reading sensor data"));
    }
  }

  // Check for any incoming serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Process any commands here if needed
  }
}