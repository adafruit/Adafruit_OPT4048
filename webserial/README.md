# OPT4048 CIE Color Plotter

This web interface allows you to visualize color measurements from an Adafruit OPT4048 color sensor in real-time using the Web Serial API.

## How to Use

1. **Upload the Arduino sketch**: First, upload the `opt4048_webserial.ino` sketch from the examples folder to your Arduino board.

2. **Connect to this web page**: You can access it at:
   - https://adafruit.github.io/Adafruit_OPT4048/webserial/
   - Or host the page locally for development

3. **Connect to your Arduino**: Click the "Connect to Arduino" button and select your Arduino from the popup menu.

4. **View measurements**: The sensor readings will appear on the CIE chromaticity diagram, showing you exactly where the measured color falls in the CIE color space.

## Features

- Displays CIE x,y coordinates in real-time
- Plots the color point on a standard CIE 1931 chromaticity diagram
- Shows lux (brightness) and color temperature (CCT) values
- Provides approximate RGB color visualization
- Monitors serial output for debugging

## Browser Compatibility

This interface uses the Web Serial API, which is currently supported in:
- Google Chrome (version 89+)
- Microsoft Edge (version 89+)
- Opera (version 75+)

It is **not** supported in Firefox or Safari due to their Web Serial API implementation status.

## About the OPT4048 Sensor

The OPT4048 is a high-precision tristimulus XYZ color sensor by Texas Instruments. The Adafruit breakout board makes it easy to interface with this sensor using I2C.

This sensor provides accurate color measurements in XYZ color space, which can be converted to standard CIE 1931 xy chromaticity coordinates.

## File Structure

- `index.html` - The main webpage
- `script.js` - JavaScript code for communication and visualization
- `cie1931_diagram.svg` - SVG image of the CIE 1931 chromaticity diagram

## License

MIT license, all text here must be included in any redistribution