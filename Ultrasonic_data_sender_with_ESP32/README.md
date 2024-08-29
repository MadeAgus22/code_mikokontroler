# Ultrasonic Sensor Data Sender with ESP32

This project demonstrates how to use an ESP32 to measure distance using an ultrasonic sensor and send the data to a server via an HTTP POST request.

## Components

- ESP32
- Ultrasonic sensor (HC-SR04)
- Jumper wires
- Wi-Fi network

## Wiring

| Ultrasonic Sensor Pin | ESP32 Pin   |
|-----------------------|-------------|
| VCC                   | 3V3         |
| GND                   | GND         |
| TRIG                  | GPIO 12     |
| ECHO                  | GPIO 14     |

## Code Overview

The code connects the ESP32 to a Wi-Fi network and measures the distance using an ultrasonic sensor. The measured distance is then sent to a server through an HTTP POST request.

### Libraries Used

- `WiFi.h`: For connecting the ESP32 to a Wi-Fi network.
- `HTTPClient.h`: For making HTTP POST requests.

### Code Breakdown

1. **Wi-Fi Connection:**
   - The ESP32 connects to the specified Wi-Fi network using the `WiFi.begin()` method.
   - The connection is checked in a loop until successful.

2. **Ultrasonic Sensor Measurement:**
   - The trigger pin is set low for a short time to clear the sensor.
   - A short high pulse is sent to the trigger pin to initiate the measurement.
   - The duration of the echo is measured and converted to a distance.

3. **Sending Data to Server:**
   - The ESP32 checks if it is connected to Wi-Fi.
   - An HTTP POST request is made to the server with the measured distance as part of the data payload.
   - The server's HTTP response code is printed to the Serial Monitor for debugging.

### Customization

- **Wi-Fi Credentials:**
  - Replace `ssid` and `password` with your Wi-Fi network name and password.

- **Server URL:**
  - Update the URL in `http.begin("http://192.168.xxx.xxx:8080/customer")` to match your server's IP address and endpoint.

- **POST Data:**
  - Modify `postData` to match the required format for your server's API. In the current code, `names=` is used as the key.

### Example Usage

1. Power the ESP32 and ensure it is connected to the Wi-Fi network.
2. The ESP32 will continuously measure the distance and send the data to the specified server endpoint every 10 seconds.
3. Monitor the Serial output for debugging information, including the HTTP response code.

### Notes

- Ensure your server is running and accessible from the ESP32's network.
- The delay between measurements and POST requests is set to 10 seconds. Adjust this as needed.

