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

- **Telegram Bot Token and Chat ID**
  ```cpp
  #define BOTtoken "YOUR_BOT_TOKEN"
  #define CHAT_ID "YOUR_ID_CHAT"
  
## Code Overview

- **Libraries and Definitions**
  
 ```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "AccelStepper.h"

#define dirPin 16
#define stepPin 17
#define motorInterfaceType 1

- **Global Variabels**

 ```cpp
bool previousLEDStatus = false;
const long utcOffsetInSeconds = 28800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
LiquidCrystal_I2C lcd(0x27, 20, 4);
String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
String months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

- **Setup Function**

 ```cpp
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected to WiFi");
  delay(2000);

  String welcome = "Hello, bot connected to the device. Use the following commands:\n\n";
  welcome += "/on to activate the device\n";
  welcome += "/off to deactivate the device\n";
  welcome += "/status to check the current status\n";
  bot.sendMessage(CHAT_ID, welcome, "");

  EEPROM.begin(512);
  pinMode(bt_up, INPUT_PULLUP);
  pinMode(bt_down, INPUT_PULLUP);
  pinMode(bt_select, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, ledState);
  pinMode(relay, OUTPUT);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo1.setPeriodHertz(50);
  myservo2.setPeriodHertz(50);
  myservo1.attach(19);
  myservo2.attach(18);
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  lcd.command(0x40 | (0 << 3));
  for (byte i = 0; i < 8; i++)
    lcd.write(karakterDetik1[i]);
  // Add remaining custom character setup here...

  timeClient.begin();
  get_alarm();
  lcd.clear();
  delay(15);
}

- **Main Loop**
 ```cpp
void loop() {
  timeClient.update();
  akses_waktu();

  if (isMotionActive && digitalRead(LED) && digitalRead(relay) == HIGH) {
    mode_gerak1();
    if (digitalRead(LED) && digitalRead(relay) == LOW) {
      stopAllModes();
      return;
    }
    mode_gerak2();
    if (digitalRead(LED) && digitalRead(relay) == LOW) {
      stopAllModes();
      return;
    }
  }

  // Menu Navigation
  if (digitalRead(bt_select) == LOW) {
    if (!tekan) {
      tekan = true;
      count++;
      if (count < 7) {
        // Add state change logic here...
      } else {
        setting_mode = false;
        set_alarm();
        lcd.clear();
        delay(15);
        count = 0;
      }
      menu = count;
    }
    delay(200);
  } else {
    tekan = false;
  }

  if (menu > 0 && menu < 4) {
    display_set_AL_time();
  } else if (menu > 3) {
    display_set_AL_time_off();
  } else {
    tampil_lcd();
    timer();
    Telegram();
  }
}

**Functions**

akses_waktu(): Updates and accesses current time.
timer(): Controls device activation based on the timer.
Telegram(): Handles incoming messages from Telegram.
tampil_lcd(): Displays the current time and timer settings on the LCD.
display_set_AL_time(): Allows setting of the ON timer.
display_set_AL_time_off(): Allows setting of the OFF timer.
set_alarm(): Saves alarm settings to EEPROM.
get_alarm(): Retrieves alarm settings from EEPROM.
display_position(int digits): Displays the given digits with leading zero.

-**Telegram Bot Handling**
 ```cpp
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands:\n\n";
      welcome += "/on to activate the device\n";
      welcome += "/off to deactivate the device\n";
      welcome += "/status to check the current status\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    // Add handling for other commands here...
  }
}

  
