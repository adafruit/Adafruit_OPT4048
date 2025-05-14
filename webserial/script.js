// Global variables
let port;
let reader;
let writer;
let readTimeout;
let keepReading = false;
let decoder = new TextDecoder();
let lineBuffer = '';

// DOM Elements
const connectButton = document.getElementById('connect-button');
const disconnectButton = document.getElementById('disconnect-button');
const clearButton = document.getElementById('clear-button');
const statusDisplay = document.getElementById('status');
const serialLog = document.getElementById('serial-log');
const dataPoint = document.getElementById('data-point');
const cieXDisplay = document.getElementById('cie-x');
const cieYDisplay = document.getElementById('cie-y');
const luxDisplay = document.getElementById('lux');
const cctDisplay = document.getElementById('cct');
const colorSample = document.getElementById('color-sample');
const debugCoordinates = document.getElementById('debug-coordinates');

// Check if Web Serial API is supported
if ('serial' in navigator) {
  connectButton.addEventListener('click', connectToArduino);
  disconnectButton.addEventListener('click', disconnectFromArduino);
  clearButton.addEventListener('click', clearLog);
  
  // Add click event to CIE diagram to toggle debug view
  document.getElementById('cie-diagram').addEventListener('click', function() {
    if (debugCoordinates.style.display === 'block') {
      debugCoordinates.style.display = 'none';
    } else if (dataPoint.style.display === 'block') {
      debugCoordinates.style.display = 'block';
    }
  });
} else {
  statusDisplay.textContent = 'Web Serial API not supported in this browser. Try Chrome or Edge.';
  connectButton.disabled = true;
}

// Connect to Arduino via Web Serial
async function connectToArduino() {
  try {
    // Request a port and open a connection
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });

    // Set up the reader and writer
    reader = port.readable.getReader();
    writer = port.writable.getWriter();

    // Enable/disable buttons
    connectButton.disabled = true;
    disconnectButton.disabled = false;
    statusDisplay.textContent = 'Connected to Arduino';
    addToLog('Connected to Arduino', 'status');

    // Start reading data
    keepReading = true;
    readSerialData();
  } catch (error) {
    console.error('Error connecting to Arduino:', error);
    addToLog(`Error connecting: ${error.message}`, 'error');
    statusDisplay.textContent = 'Connection failed';
  }
}

// Disconnect from Arduino
async function disconnectFromArduino() {
  if (reader) {
    keepReading = false;
    clearTimeout(readTimeout);
    
    try {
      await reader.cancel();
      await reader.releaseLock();
      reader = null;
    } catch (error) {
      console.error('Error releasing reader:', error);
    }
  }

  if (writer) {
    try {
      await writer.close();
      writer = null;
    } catch (error) {
      console.error('Error releasing writer:', error);
    }
  }

  if (port) {
    try {
      await port.close();
      port = null;
    } catch (error) {
      console.error('Error closing port:', error);
    }
  }

  // Update UI
  connectButton.disabled = false;
  disconnectButton.disabled = true;
  statusDisplay.textContent = 'Disconnected';
  addToLog('Disconnected from Arduino', 'status');
  hideDataPoint();
}

// Read data from the serial port
async function readSerialData() {
  while (port && keepReading) {
    try {
      const { value, done } = await reader.read();
      
      if (done) {
        // Reader has been canceled
        break;
      }
      
      // Process the received data
      processSerialData(decoder.decode(value));
    } catch (error) {
      console.error('Error reading data:', error);
      addToLog(`Error reading data: ${error.message}`, 'error');
      break;
    }
  }

  // If we exited the loop without being explicitly disconnected
  if (keepReading) {
    disconnectFromArduino();
  }
}

// Process data received from Arduino
function processSerialData(data) {
  // Add received data to the buffer
  lineBuffer += data;
  
  // Process complete lines
  let lineEnd;
  while ((lineEnd = lineBuffer.indexOf('\n')) !== -1) {
    const line = lineBuffer.substring(0, lineEnd).trim();
    lineBuffer = lineBuffer.substring(lineEnd + 1);
    
    if (line) {
      addToLog(line);
      parseDataFromLine(line);
    }
  }
}

// Parse data from a line received from Arduino
function parseDataFromLine(line) {
  // Look for CIE x value
  const cieXMatch = line.match(/CIE x: ([\d.]+)/);
  if (cieXMatch) {
    const cieX = parseFloat(cieXMatch[1]);
    cieXDisplay.textContent = cieX.toFixed(6);
  }
  
  // Look for CIE y value
  const cieYMatch = line.match(/CIE y: ([\d.]+)/);
  if (cieYMatch) {
    const cieY = parseFloat(cieYMatch[1]);
    cieYDisplay.textContent = cieY.toFixed(6);
    
    // If we have both x and y, update the plot
    if (cieXMatch) {
      const cieX = parseFloat(cieXMatch[1]);
      updateCIEPlot(cieX, cieY);
    }
  }
  
  // Look for Lux value
  const luxMatch = line.match(/Lux: ([\d.]+)/);
  if (luxMatch) {
    const lux = parseFloat(luxMatch[1]);
    luxDisplay.textContent = lux.toFixed(2);
  }
  
  // Look for Color Temperature value
  const cctMatch = line.match(/Color Temperature: ([\d.]+)/);
  if (cctMatch) {
    const cct = parseFloat(cctMatch[1]);
    cctDisplay.textContent = cct.toFixed(0);
  }
}

// Initialize debug coordinates
document.addEventListener('DOMContentLoaded', function() {
  debugCoordinates.textContent = 'Waiting for color data...';
});

// Update the CIE plot with new data point
function updateCIEPlot(x, y) {
  console.log(`Plotting CIE coordinates: x=${x}, y=${y}`); // Debug log
  
  // Get the dimensions of the CIE diagram container
  const cieDiagram = document.getElementById('cie-diagram');
  
  // Ensure we're only working with valid x,y coordinates
  if (isNaN(x) || isNaN(y) || x < 0 || y < 0 || x > 1 || y > 1) {
    console.warn(`Invalid CIE coordinates: x=${x}, y=${y}`);
    debugCoordinates.textContent = `Invalid coordinates: x=${x}, y=${y}`;
    return;
  }
  
  // Adjust coordinates to fit the visible area of the CIE diagram
  // CIE diagram typically has coordinates: x [0-0.8], y [0-0.9]
  const xMax = 0.8;
  const yMax = 0.9;
  
  // Get actual dimensions of the CIE diagram image
  const cieImage = document.querySelector('#cie-diagram img');
  const imgWidth = cieImage.clientWidth;
  const imgHeight = cieImage.clientHeight;
  
  // Calculate percentage positions within the SVG viewBox
  const xPercent = (x / xMax) * 100; // Scale to percentage of max x (0.8)
  const yPercent = (1 - (y / yMax)) * 100; // Invert y-axis and scale to percentage of max y (0.9)
  
  console.log(`Plotting at: left=${xPercent}%, top=${yPercent}%`); // Debug log
  
  // Set the data point position
  dataPoint.style.left = `${xPercent}%`;
  dataPoint.style.top = `${yPercent}%`;
  dataPoint.style.display = 'block';
  
  // Show debug coordinates for troubleshooting
  debugCoordinates.textContent = `CIE: (${x.toFixed(4)}, ${y.toFixed(4)}) → Position: (${Math.round(xPercent)}%, ${Math.round(yPercent)}%)`;
  
  // Update the color sample with an approximate RGB color
  updateColorSample(x, y);
}

// Convert CIE XYZ to RGB for color approximation
function updateColorSample(x, y) {
  // Calculate XYZ from xyY (assuming Y=1 for relative luminance)
  const Y = 1.0;
  const X = (x * Y) / y;
  const Z = ((1 - x - y) * Y) / y;
  
  // XYZ to RGB conversion (sRGB)
  // Using the standard D65 transformation matrix
  let r = X * 3.2406 - Y * 1.5372 - Z * 0.4986;
  let g = -X * 0.9689 + Y * 1.8758 + Z * 0.0415;
  let b = X * 0.0557 - Y * 0.2040 + Z * 1.0570;
  
  // Apply gamma correction
  r = r <= 0.0031308 ? 12.92 * r : 1.055 * Math.pow(r, 1/2.4) - 0.055;
  g = g <= 0.0031308 ? 12.92 * g : 1.055 * Math.pow(g, 1/2.4) - 0.055;
  b = b <= 0.0031308 ? 12.92 * b : 1.055 * Math.pow(b, 1/2.4) - 0.055;
  
  // Clamp RGB values between 0 and 1
  r = Math.min(Math.max(0, r), 1);
  g = Math.min(Math.max(0, g), 1);
  b = Math.min(Math.max(0, b), 1);
  
  // Convert to 8-bit color values
  const ri = Math.round(r * 255);
  const gi = Math.round(g * 255);
  const bi = Math.round(b * 255);
  
  // Set the background color of the sample
  colorSample.style.backgroundColor = `rgb(${ri}, ${gi}, ${bi})`;
}

// Hide the data point and reset all displays
function hideDataPoint() {
  dataPoint.style.display = 'none';
  debugCoordinates.textContent = 'Waiting for color data...';
  cieXDisplay.textContent = '-';
  cieYDisplay.textContent = '-';
  luxDisplay.textContent = '-';
  cctDisplay.textContent = '-';
  colorSample.style.backgroundColor = 'transparent';
}

// Add a message to the serial log
function addToLog(message, type = 'data') {
  const entry = document.createElement('div');
  entry.textContent = message;
  entry.className = `log-entry ${type}`;
  serialLog.appendChild(entry);
  serialLog.scrollTop = serialLog.scrollHeight;
}

// Clear the serial log
function clearLog() {
  serialLog.innerHTML = '';
}

// Send a command to the Arduino
async function sendCommand(command) {
  if (writer) {
    try {
      const encoder = new TextEncoder();
      await writer.write(encoder.encode(command + '\n'));
      addToLog(`Sent: ${command}`, 'command');
    } catch (error) {
      console.error('Error sending command:', error);
      addToLog(`Error sending command: ${error.message}`, 'error');
    }
  }
}