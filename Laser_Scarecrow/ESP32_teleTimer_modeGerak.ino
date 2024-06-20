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

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "IoT";
const char* password = "qwerty1234";

// Token dan Chat ID Telegram BOT
#define BOTtoken "6999662252:AAF_sRNqOLQtgONZze8flKVgGOKX1exf0-c"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-4155576299"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 500;
unsigned long lastTimeBotRan;


// Variabel tambahan untuk melacak status LED sebelumnya
bool previousLEDStatus = false;


const long utcOffsetInSeconds = 28800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
LiquidCrystal_I2C lcd(0x27, 20, 4);

String weekDays[7] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu" };
String months[12] = { "Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember" };

char bufWaktu[40];
String formattedTime;
String weekDay;
int currentHour;
int currentMinute;
int currentSecond;
int monthDay;
int currentMonth;
String currentMonthName;
int currentYear;
int bt_up = 14;
int bt_down = 12;
int bt_select = 27;
int LED = 13;
int relay = 5;
int menu = 0;
int count = 0;
bool tekan = false;

int jam_timer_on;
int menit_timer_on;
bool timer_active = false;

int jam_timer_off;
int menit_timer_off;
bool timer_inactive = false;

uint32_t blink_interval = 400;
uint32_t blink_previousMillis = 0;
boolean blink_state = false;
boolean RTC_error = true;
boolean long_press_button = false;
bool setting_mode = false;
int menit_sebelum = 0;
bool ledState = LOW;

bool isMotionActive = false;

unsigned long previousMillisLCD = 0;
const long intervalLCD = 1000;  // LCD update interval
unsigned long currentMillis = millis();
Servo myservo1;
Servo myservo2;
AccelStepper stepper(motorInterfaceType, stepPin, dirPin);


enum STATES {
  set_jam_timer_on,
  set_menit_timer_on,
  set_onoff_timer_active,
  set_jam_timer_off,
  set_menit_timer_off,
  set_onoff_timer_inactive,
};

STATES state;

byte karakterDetik1[8] = { 0b00000, 0b00000, 0b00000, 0b00100, 0b00000, 0b00000, 0b00000, 0b00000 };
byte karakterDetik2[8] = { 0b00000, 0b00000, 0b00100, 0b01010, 0b00100, 0b00000, 0b00000, 0b00000 };
byte karakterDetik3[8] = { 0b00000, 0b00100, 0b01010, 0b10001, 0b01010, 0b00100, 0b00000, 0b00000 };

//////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0, 0);
    lcd.print("Menghubungkan WiFi...");
    delay(1000);
  }
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Berhasil");
  lcd.setCursor(0, 1);
  lcd.print("Terhubung ke WiFi");
  delay(2000);

  String welcome = "Hallo, bot terhubung dengan alat. Gunakan perintah dibawah ini:\n\n";
  welcome += "/on mengaktifkan Alat \n";
  welcome += "/off menonaktifkan Alat\n";
  welcome += "/status mengecek kondisi terkini Alat \n";
  bot.sendMessage(CHAT_ID, welcome, "");

  EEPROM.begin(512);
  pinMode(bt_up, INPUT_PULLUP);
  pinMode(bt_down, INPUT_PULLUP);
  pinMode(bt_select, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, ledState);
  pinMode(relay, OUTPUT);
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo1.setPeriodHertz(50);
  myservo2.setPeriodHertz(50);
  myservo1.attach(19);  // ganti dengan pin yang sesuai
  myservo2.attach(18);  // ganti dengan pin yang sesuai
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  lcd.command(0x40 | (0 << 3));
  for (byte i = 0; i < 8; i++)
    lcd.write(karakterDetik1[i]);

  lcd.command(0x40 | (1 << 3));
  for (byte i = 0; i < 8; i++)
    lcd.write(karakterDetik2[i]);

  lcd.command(0x40 | (2 << 3));
  for (byte i = 0; i < 8; i++)
    lcd.write(karakterDetik3[i]);

  lcd.command(0x40 | (3 << 3));
  for (byte i = 0; i < 8; i++)
    lcd.write(karakterDetik2[i]);

  timeClient.begin();
  get_alarm();
  lcd.clear();
  delay(15);
}

/////////////////////////////////////////////////////////////////////////////////////////
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

  if (digitalRead(bt_select) == LOW) {
    if (tekan == false) {
      tekan = true;
      count++;
      if (count < 7) {  // Total menu count
        if (count == 1) {
          setting_mode = true;
          get_alarm();
          lcd.clear();
          delay(15);
          state = set_jam_timer_on;
        } else if (count == 2) {
          state = set_menit_timer_on;
        } else if (count == 3) {
          state = set_onoff_timer_active;
        } else if (count == 4) {
          state = set_jam_timer_off;
        } else if (count == 5) {
          state = set_menit_timer_off;
        } else if (count == 6) {
          state = set_onoff_timer_inactive;
        }
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

  if (menu > 0 && menu < 4) {  // ON alarm setting
    display_set_AL_time();
  } else if (menu > 3) {  // OFF alarm setting
    display_set_AL_time_off();
  } else {
    tampil_lcd();
    timer();
    Telegram();
  }
}

void akses_waktu() {
  //mengakses waktu
  time_t epochTime = timeClient.getEpochTime();
  formattedTime = timeClient.getFormattedTime();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();
  currentSecond = timeClient.getSeconds();
  weekDay = weekDays[timeClient.getDay()];

  struct tm* ptm = gmtime((time_t*)&epochTime);
  monthDay = ptm->tm_mday;
  currentMonth = ptm->tm_mon + 1;
  currentMonthName = months[currentMonth - 1];
  currentYear = ptm->tm_year + 1900;

  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
}

void timer() {
  // Kontrol melalui timer
  if (timer_active == true && timer_inactive == true) {
    // Periksa apakah timer aktif
    if (setting_mode == false) {
      // Pastikan tidak dalam mode pengaturan timer
      if (currentHour == jam_timer_on && currentMinute == menit_timer_on) {
        // Aktifkan LED sesuai dengan waktu yang ditetapkan
        isMotionActive = true;
        ledState = HIGH;
        digitalWrite(LED, ledState);
        digitalWrite(relay, HIGH);
        // Kirim pesan ke Telegram
        bot.sendMessage(CHAT_ID, "Timer ON: Alat telah Aktif", "");

      } else if (currentHour == jam_timer_off && currentMinute == me nit_timer_off) {
        // Matikan LED sesuai dengan waktu yang ditetapkan
        isMotionActive = false;
        stopAllModes();
        ledState = LOW;
        digitalWrite(LED, ledState);
        digitalWrite(relay, LOW);
        // Kirim pesan ke Telegram
        bot.sendMessage(CHAT_ID, "Timer OFF: Alat telah berhenti Aktif", "");
      }
    }
  }
  if (timer_active && timer_inactive == false) {
    // Jika tidak ada perubahan dari MQTT dan timer tidak aktif, matikan LED
    isMotionActive = false;
    stopAllModes();
    ledState = LOW;
    digitalWrite(LED, ledState);
    digitalWrite(relay, LOW);
  }
}
void Telegram() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
/////////////////////////////////////////////////////////////////////////////////////////
void tampil_lcd() {
  lcd.setCursor(0, 0);
  lcd.print(weekDay);
  lcd.print(',');
  sprintf(bufWaktu, "%02d/%02d/%02d", monthDay, currentMonth, currentYear - 2000);
  lcd.setCursor(9, 0);
  lcd.print(bufWaktu);

  sprintf(bufWaktu, "%02d:%02d:%02d", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
  lcd.setCursor(9, 1);
  lcd.print(bufWaktu);
  lcd.setCursor(1, 1);
  lcd.print("Jam    :");
  lcd.setCursor(0, 1);
  lcd.print((char)(timeClient.getSeconds() % 4));
  lcd.setCursor(0, 2);
  lcd.print("Alat ON :");
  lcd.setCursor(0, 3);
  lcd.print("Alat OFF:");

  if (timer_active == true) {
    lcd.setCursor(9, 2);
    sprintf(bufWaktu, "%02d:%02d", jam_timer_on, menit_timer_on);
    lcd.print(bufWaktu);

  } else {
    lcd.setCursor(9, 2);
    lcd.print("__:__");
  }

  if (timer_inactive == true) {
    lcd.setCursor(9, 3);
    sprintf(bufWaktu, "%02d:%02d", jam_timer_off, menit_timer_off);
    lcd.print(bufWaktu);
  } else {
    lcd.setCursor(9, 3);
    lcd.print("__:__");
  }
}
/////////////////////////////////////////////////////////////////////////////////////////
void display_set_AL_time() {
  lcd.setCursor(3, 0);
  lcd.print("Set Timer ON:");
  if (timer_active == true) {
    lcd.setCursor(12, 1);
    lcd.print("SET");
  } else {
    lcd.setCursor(12, 1);
    lcd.print("OFF");
  }

  switch (state) {
    case set_jam_timer_on:
      lcd.setCursor(10, 1);
      lcd.print("h");
      if (long_press_button == false) {
        if (blink_state == 0) {
          lcd.setCursor(4, 1);
          display_position(jam_timer_on);
        } else {
          lcd.setCursor(4, 1);
          lcd.print("  ");
        }
      } else {
        lcd.setCursor(4, 1);
        display_position(jam_timer_on);
      }
      lcd.print(":");
      display_position(menit_timer_on);

      if (digitalRead(bt_up) == LOW) {
        long_press_button = true;
        jam_timer_on++;
        if (jam_timer_on > 23) jam_timer_on = 0;
        delay(150);
      } else if (digitalRead(bt_down) == LOW) {
        long_press_button = true;
        jam_timer_on--;
        if (jam_timer_on < 0) jam_timer_on = 23;
        delay(150);
      } else {
        long_press_button = false;
      }
      break;
    case set_menit_timer_on:
      lcd.setCursor(10, 1);
      lcd.print("m");
      lcd.setCursor(4, 1);
      display_position(jam_timer_on);
      lcd.print(":");
      if (long_press_button == false) {
        if (blink_state == 0) {
          lcd.setCursor(7, 1);
          display_position(menit_timer_on);
        } else {
          lcd.setCursor(7, 1);
          lcd.print("  ");
        }
      } else {
        lcd.setCursor(7, 1);
        display_position(menit_timer_on);
      }

      if (digitalRead(bt_up) == LOW) {
        long_press_button = true;
        menit_timer_on++;
        if (menit_timer_on > 59) menit_timer_on = 0;
        delay(150);
      } else if (digitalRead(bt_down) == LOW) {
        long_press_button = true;
        menit_timer_on--;
        if (menit_timer_on < 0) menit_timer_on = 59;
        delay(150);
      } else {
        long_press_button = false;
      }
      break;
    case set_onoff_timer_active:
      lcd.setCursor(7, 1);
      display_position(menit_timer_on);

      if (timer_active == true) {
        if (blink_state == 0) {
          lcd.setCursor(14, 1);
          lcd.print(" ");
          lcd.setCursor(12, 1);
          lcd.print("SET");
        } else {
          lcd.setCursor(12, 1);
          lcd.print("   ");
        }
      } else {
        if (blink_state == 0) {
          lcd.setCursor(12, 1);
          lcd.print("OFF");
        } else {
          lcd.setCursor(12, 1);
          lcd.print("   ");
        }
      }

      if (digitalRead(bt_up) == LOW) {
        timer_active = !timer_active;  // Toggle timer_active
        delay(200);
      } else if (digitalRead(bt_down) == LOW) {
        timer_active = !timer_active;  // Toggle timer_active
        delay(200);
      }
      break;
  }
  unsigned long blink_currentMillis = millis();
  if (blink_currentMillis - blink_previousMillis > blink_interval) {
    blink_previousMillis = blink_currentMillis;
    blink_state = !blink_state;  // Toggle blink_state
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void display_set_AL_time_off() {
  lcd.setCursor(3, 2);
  lcd.print("Set Timer OFF:");
  if (timer_inactive == true) {
    lcd.setCursor(12, 3);
    lcd.print("SET");
  } else {
    lcd.setCursor(12, 3);
    lcd.print("OFF");
  }

  switch (state) {
    case set_jam_timer_off:
      lcd.setCursor(10, 3);
      lcd.print("h");
      if (long_press_button == false) {
        if (blink_state == 0) {
          lcd.setCursor(4, 3);
          display_position(jam_timer_off);
        } else {
          lcd.setCursor(4, 3);
          lcd.print("  ");
        }
      } else {
        lcd.setCursor(4, 3);
        display_position(jam_timer_off);
      }
      lcd.print(":");
      display_position(menit_timer_off);

      if (digitalRead(bt_up) == LOW) {
        long_press_button = true;
        jam_timer_off++;
        if (jam_timer_off > 23) jam_timer_off = 0;
        delay(150);
      } else if (digitalRead(bt_down) == LOW) {
        long_press_button = true;
        jam_timer_off--;
        if (jam_timer_off < 0) jam_timer_off = 23;
        delay(150);
      } else {
        long_press_button = false;
      }
      break;
    case set_menit_timer_off:
      lcd.setCursor(10, 3);
      lcd.print("m");
      lcd.setCursor(4, 3);
      display_position(jam_timer_off);
      lcd.print(":");
      if (long_press_button == false) {
        if (blink_state == 0) {
          lcd.setCursor(7, 3);
          display_position(menit_timer_off);
        } else {
          lcd.setCursor(7, 3);
          lcd.print("  ");
        }
      } else {
        lcd.setCursor(7, 3);
        display_position(menit_timer_off);
      }

      if (digitalRead(bt_up) == LOW) {
        long_press_button = true;
        menit_timer_off++;
        if (menit_timer_off > 59) menit_timer_off = 0;
        delay(150);
      } else if (digitalRead(bt_down) == LOW) {
        long_press_button = true;
        menit_timer_off--;
        if (menit_timer_off < 0) menit_timer_off = 59;
        delay(150);
      } else {
        long_press_button = false;
      }
      break;
    case set_onoff_timer_inactive:
      lcd.setCursor(7, 3);
      display_position(menit_timer_off);

      if (timer_inactive == true) {
        if (blink_state == 0) {
          lcd.setCursor(14, 3);
          lcd.print(" ");
          lcd.setCursor(12, 3);
          lcd.print("SET");
        } else {
          lcd.setCursor(12, 3);
          lcd.print("   ");
        }
      } else {
        if (blink_state == 0) {
          lcd.setCursor(12, 3);
          lcd.print("OFF");
        } else {
          lcd.setCursor(12, 3);
          lcd.print("   ");
        }
      }

      if (digitalRead(bt_up) == LOW) {
        timer_inactive = !timer_inactive;  // Toggle timer_inactive
        delay(200);
      } else if (digitalRead(bt_down) == LOW) {
        timer_inactive = !timer_inactive;  // Toggle timer_inactive
        delay(200);
      }
      break;
  }
  unsigned long blink_currentMillis = millis();
  if (blink_currentMillis - blink_previousMillis > blink_interval) {
    blink_previousMillis = blink_currentMillis;
    blink_state = !blink_state;  // Toggle blink_state
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void set_alarm() {
  EEPROM.write(0, jam_timer_on);
  EEPROM.write(1, menit_timer_on);
  EEPROM.write(2, timer_active);
  EEPROM.write(3, jam_timer_off);
  EEPROM.write(4, menit_timer_off);
  EEPROM.write(5, timer_inactive);
  EEPROM.commit();
  delay(200);
}

/////////////////////////////////////////////////////////////////////////////////////////
void get_alarm() {
  jam_timer_on = EEPROM.read(0);
  if (jam_timer_on > 23) jam_timer_on = 0;
  menit_timer_on = EEPROM.read(1);
  if (menit_timer_on > 59) menit_timer_on = 0;
  timer_active = EEPROM.read(2);
  jam_timer_off = EEPROM.read(3);
  if (jam_timer_off > 23) jam_timer_off = 0;
  menit_timer_off = EEPROM.read(4);
  if (menit_timer_off > 59) menit_timer_off = 0;
  timer_inactive = EEPROM.read(5);
  delay(200);
}

/////////////////////////////////////////////////////////////////////////////////////////
void display_position(int digits) {
  if (digits < 10)
    lcd.print("0");
  lcd.print(digits);
}
/////////////////////////////////////////////////////////////////////////////////////////

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Selamat Datang, " + from_name + ".\n";
      welcome += "Gunakan perintah dibawah ini:.\n\n";
      welcome += "/on mengaktifkan Alat \n";
      welcome += "/off menonaktifkan Alat\n";
      welcome += "/status mengecek kondisi terkini Alat \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/on") {
      bot.sendMessage(chat_id, "Alat telah Aktif", "");
      isMotionActive = true;
      ledState = HIGH;
      digitalWrite(LED, ledState);
      digitalWrite(relay, HIGH);
    }

    if (text == "/off") {
      bot.sendMessage(chat_id, "Alat telah Non-Aktif", "");
      isMotionActive = false;
      stopAllModes();
      ledState = LOW;
      digitalWrite(LED, ledState);
      digitalWrite(relay, LOW);
    }

    if (text == "/status") {
      String statusMessage = "";

      if (digitalRead(LED)) {
        statusMessage = "Alat saat ini sedang Aktif\n";
      } else {
        statusMessage = "Alat saat ini Tidak Aktif\n";
      }

      statusMessage += "Status Timer:\n";
      statusMessage += "Alat ON: " + String(jam_timer_on) + ":" + String(menit_timer_on) + "\n";
      statusMessage += "Alat OFF: " + String(jam_timer_off) + ":" + String(menit_timer_off);

      bot.sendMessage(chat_id, statusMessage, "");
    }
  }
}

void mode_gerak1() {
  int servoAngles[] = { 90, 91, 90, 91 };
  int numServoAngles = sizeof(servoAngles) / sizeof(servoAngles[0]);
  static unsigned long previousServoMillis = 0;
  const long servoInterval = 500;
  unsigned long currentMillis = millis();

  for (int i = 0; i < numServoAngles; i++) {
    myservo1.write(servoAngles[i]);
    myservo2.write(servoAngles[i]);

    unsigned long servoCurrentMillis = millis();
    while (millis() - servoCurrentMillis < servoInterval) {
      if (digitalRead(LED) && digitalRead(relay) == LOW) return;
      updateLCD(currentMillis);
    }

    stepper.moveTo(0);
    stepper.runToPosition();
    while (stepper.distanceToGo() != 0) {
      if (digitalRead(LED) == LOW) return;
      updateLCD(currentMillis);
    }

    myservo1.write(servoAngles[i]);
    myservo2.write(servoAngles[i]);
    servoCurrentMillis = millis();
    while (millis() - servoCurrentMillis < servoInterval) {
      if (digitalRead(LED) && digitalRead(relay) == LOW) return;
      updateLCD(currentMillis);
    }

    stepper.moveTo(400);
    stepper.runToPosition();
    while (stepper.distanceToGo() != 0) {
      if (digitalRead(LED) && digitalRead(relay) == LOW) return;
      updateLCD(currentMillis);
    }
  }
}

void mode_gerak2() {
  int pos = 0;
  int targetPositions[] = { 0, 100, 200, 300, 400, 350, 250, 150, 50, 0 };
  int numTargetPositions = sizeof(targetPositions) / sizeof(targetPositions[0]);
  unsigned long currentMillis = millis();

  for (int j = 0; j < numTargetPositions; j++) {
    stepper.moveTo(targetPositions[j]);
    stepper.runToPosition();
    while (stepper.distanceToGo() != 0) {
      if (digitalRead(LED) && digitalRead(relay) == LOW) return;
      Telegram();
      updateLCD(currentMillis);
    }

    for (pos = 50; pos <= 90; pos += 1) {
      myservo1.write(pos);
      myservo2.write(pos);
      delay(15);
    }

    for (pos = 90; pos >= 50; pos -= 1) {
      myservo1.write(pos);
      myservo2.write(pos);
      delay(15);
    }

    delay(50);
    updateLCD(currentMillis);
  }
}

// Fungsi untuk menghentikan semua mode gerak
void stopAllModes() {
  // Hentikan servo 1 dan servo 2
  myservo1.write(90);
  myservo2.write(90);

  // Hentikan motor stepper
  stepper.setCurrentPosition(0);
  stepper.moveTo(0);
  stepper.runToPosition();
}

void updateLCD(unsigned long currentMillis) {

  if (currentMillis - previousMillisLCD >= intervalLCD) {
    previousMillisLCD = currentMillis;
    tampil_lcd();
  }
}