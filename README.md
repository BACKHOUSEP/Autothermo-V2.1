# Autothermo-V2.1
Overview
This code sketch runs on an ESP32 device connected to an MTS4Z temperature sensor via I2C and manages:

Reading temperature data from the sensor with smoothing and calibration,

Connecting to WiFi,

Communicating with a remote REST API to check the device’s registration and patient assignment,

Sending periodic temperature monitoring data to a remote server.
The MTS4Z.cpp and MTS4Z.h are classess that help extract temperature data from MTS4Z sensor based on the datasheet.

Key Functionalities
Sensor Setup and Calibration:

Uses the MTS4Z temperature sensor connected on I2C bus 0 (pins GPIO8 = SDA, GPIO9 = SCL).

Initializes the sensor and applies an offset calibration based on predefined reference thermometer readings versus raw MTS4Z readings.

Applies exponential smoothing to temperature measurements to reduce noise.

WiFi Connection:

Connects to the specified WiFi network (ssid and password are hardcoded).

Waits until the device is connected to WiFi before proceeding.

Device and Assignment Check:

On startup and periodically, queries a remote API endpoint for device status using the device serial number.

If a valid device ID is returned, the code then fetches the current assignment details linked to the device from another API endpoint. Assignment includes patient ID, bed number, and hospital ID.

Stores these assignment details locally for use in sending monitoring data.

Tracks if the device is successfully assigned to a patient.

Data Monitoring and Transmission:

In the main loop, every 5 seconds:

Reads the temperature sensor value and applies smoothing.

Prints the smoothed temperature to the serial console.

If a patient assignment exists, sends the monitoring data to the server using an HTTP POST request.

The POST includes hospital ID, patient ID, device ID, and temperature data.

SpO2 and heart rate fields are set to zero (this code only handles temperature).

Error Handling and Status Feedback:

Provides serial debug output on sensor initialization success/failure, WiFi connection status, HTTP request success/failure, and server responses.

If device assignment is missing or network is disconnected, skips data sending and retries status check.

Summary
The code integrates a temperature sensor with calibration and smoothing.

Connects the device to a WiFi network.

Uses HTTP GET requests to validate device registration and retrieve patient assignment from a backend.

Periodically reads temperature data and sends it with patient/device metadata via HTTP POST to a remote monitoring API.

Provides serial debug logs for monitoring system status and troubleshooting.
