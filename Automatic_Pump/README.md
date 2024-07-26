# Ultrasonic Sensor and Relay Control

This project uses an ultrasonic sensor to measure distance and control a relay based on the measured distance. The relay is activated when the distance is greater than 60 cm and deactivated when the distance is less than 35 cm.

## Components

- Ultrasonic sensor HC-SR04
- Relay module
- Arduino Uno
- Jumper wires

## Wiring

- Connect the TRIGGER_PIN (pin 10) to the trigger pin of the ultrasonic sensor.
- Connect the ECHO_PIN (pin 9) to the echo pin of the ultrasonic sensor.
- Connect the RELAY_PIN (pin 7) to the input pin of the relay module.
- Connect the VCC and GND pins of the ultrasonic sensor and relay module to the 5V and GND pins of the Arduino.
