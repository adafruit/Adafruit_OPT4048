# Adafruit OPT4048 Library [![Build Status](https://github.com/adafruit/Adafruit_OPT4048/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_OPT4048/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_OPT4048/html/index.html)

This is an Arduino library for the Adafruit OPT4048 breakout board, a high-speed, high-precision tristimulus XYZ color sensor.

## About the OPT4048

The [OPT4048](https://www.adafruit.com/products/6334) is a high-precision, tristimulus XYZ color sensor that provides accurate color measurements by using an integrated CIE1931 XYZ filter technology. 

Key features:
* Four-channel sensor (X, Y, Z, clear)
* High-precision color measurements with CIE1931 filter technology
* Wide dynamic range with auto-ranging capability
* Programmable interrupt with high and low thresholds
* Adjustable conversion time (600μs to 800ms per channel)
* Measures color temperature, illuminance, and chromaticity
* I²C interface
* 1.7V to 3.6V operating voltage

## Dependencies

This library depends on the [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) library for I²C communication.

## Installation

You can install this library through the Arduino Library Manager. Search for "Adafruit OPT4048" and install the latest version.

To manually install:
1. Download the .zip file from GitHub
2. In the Arduino IDE: Sketch -> Include Library -> Add .ZIP Library

## Hardware

* [Adafruit OPT4048 - High Speed Tristimulus XYZ Color Sensor Breakout](https://www.adafruit.com/products/6334)

## Examples

The library includes several examples demonstrating various features:

* **opt4048_basictest**: Basic usage with continuous readings
* **opt4048_fulltest**: Demonstrates all sensor configurations
* **opt4048_intpin**: Using the interrupt pin for data-ready notifications
* **opt4048_oneshot**: One-shot measurement mode for low power applications

## Library Features

* Initialize the sensor with custom I²C address and Wire interface
* Configure measurement settings (range, conversion time, operating mode)
* Set up and use the interrupt system
* Read raw channel data from all four sensors
* Calculate CIE color coordinates (x, y) and illuminance (lux)
* Determine color temperature in Kelvin

## Documentation

For more information on using this library, check out the [examples](/examples) folder.

Full documentation of the OPT4048 sensor can be found in the [datasheet](https://www.ti.com/lit/ds/symlink/opt4048.pdf).

## License

This library is released under an MIT license. See the included LICENSE file for details.
