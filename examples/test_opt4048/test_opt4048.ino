/*
  test_opt4048.ino
  Example sketch for OPT4048 ambient light sensor
*/

#include <Wire.h>
#include "Adafruit_OPT4048.h"

Adafruit_OPT4048 sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("OPT4048 Test");
  if (!sensor.begin()) {
    Serial.println("Failed to find OPT4048 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("OPT4048 found!");
}

void loop() {
  // Nothing to do (testing only initialization)
  delay(1000);
}