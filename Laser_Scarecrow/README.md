# Laser Scarecrow Project

This project uses an ESP32 to control a laser-based scarecrow system. The system integrates various components including servos, a stepper motor, a relay, and an LCD display. It can be controlled remotely via Telegram and allows for timer-based activation.

## Components

- **ESP32 DevKitC**
- **2 x MG90S Servos**
- **NEMA17 Stepper Motor**
- **TB6600 Stepper Motor Driver**
- **Relay**
- **20x4 LCD Display**
- **Various Buttons**

## Libraries Used

- `WiFi.h`
- `WiFiClientSecure.h`
- `Wire.h`
- `LiquidCrystal_I2C.h`
- `NTPClient.h`
- `WiFiUdp.h`
- `EEPROM.h`
- `UniversalTelegramBot.h`
- `ArduinoJson.h`
- `ESP32Servo.h`
- `AccelStepper.h`

## Configuration

Before running the code, make sure to replace the following placeholders with your actual credentials:

- **Wi-Fi Credentials**
  ```cpp
  const char* ssid = "YOUR_SSID";
  const char* password = "YOUR_PASSWORD";
