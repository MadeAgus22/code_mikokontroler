# Smart Aquarium

The Smart Aquarium project uses an ESP8266 to monitor and control water quality and fish feeding. This system employs various sensors to measure water temperature, pH levels, and food availability, as well as a servo motor to control feeding. Additionally, the project includes a Telegram bot for user communication, providing information and receiving commands.

## Components

- ESP8266
- DS18B20 Temperature Sensor
- pH Sensor 4502C
- HC-SR04 Ultrasonic Sensor
- Servo Motor
- Buzzer
- I2C 16x2 LCD Display
- Jumper Wires

## Connections

- **Servo Motor**: D5 (GPIO 14)
- **Buzzer**: D8 (GPIO 15)
- **DS18B20 Temperature Sensor**: D4 (GPIO 2)
- **HC-SR04 Ultrasonic Sensor**: Trig - D6 (GPIO 12), Echo - D7 (GPIO 13)
- **pH Sensor 4502C**: A0 (Analog Pin)
- **I2C 16x2 LCD Display**: SDA - D2 (GPIO 4), SCL - D1 (GPIO 5)

## Installation

### Install Libraries

Ensure you have installed the following libraries via the Arduino IDE Library Manager:

- ESP8266WiFi
- WiFiClientSecure
- UniversalTelegramBot
- ArduinoJson
- Servo
- OneWire
- DallasTemperature
- LiquidCrystal_I2C

### Upload the Code to ESP8266

1. Open the `smart_aquarium.ino` file in Arduino IDE.
2. Replace `YOUR SSID`, `YOUR PASSWORD`, `YOUR BOT TOKEN`, and `YOUR ID CHAT` with your WiFi credentials and Telegram bot information.
3. Select the `NodeMCU 1.0 (ESP-12E Module)` board and the appropriate port.
4. Click `Upload`.

## Usage

Once the code is uploaded and the device is connected to WiFi, you can use the following commands through the Telegram bot to interact with the Smart Aquarium:

- `/start`: Displays a welcome message and the list of available commands.
- `/makan`: Activates the servo to feed the fish.
- `/cek`: Checks the aquarium's condition, including water temperature, pH level, and food availability.

## Features

- **Temperature Monitoring**: Uses the DS18B20 sensor to measure the water temperature.
- **pH Level Monitoring**: Utilizes the 4502C pH sensor to check the water pH level.
- **Food Availability Monitoring**: Employs the HC-SR04 ultrasonic sensor to determine the amount of food available.
- **Feeding Mechanism**: Controls a servo motor to dispense fish food.
- **User Notifications**: Sends updates and receives commands via a Telegram bot.
- **LCD Display**: Shows real-time status of the aquarium on an I2C 16x2 LCD display.
- **Buzzer Alert**: Alerts when the food level is low.


