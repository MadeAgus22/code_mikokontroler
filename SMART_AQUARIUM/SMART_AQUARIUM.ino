#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>               // Perpustakaan untuk komunikasi I2C
#include <LiquidCrystal_I2C.h>  // Perpustakaan untuk LCD I2C

#define SERVO_PIN 14   // Pin output servo
const int buzpin = 15;  // Pin buzzer

Servo servo;
bool buttonState = false;
bool buttonPrevState = false;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Set the LCD address
const int lcdAddress = 0x27;  //  I2C LCD 16x2

// Set the LCD dimensions
const int lcdColumns = 16;  
const int lcdRows = 2;      

// Initialize the LCD
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

//Ultrasonic HC-SR04
const int trigPin = 12;
const int echoPin = 13;   
long duration;
float distanceCm;
float distanceInch;
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

//sensor PH 4502C
const int ph_Pin = A0;
float Po = 0;
float PH_step;
int nilai_analog_PH;
double TeganganPH;

//untuk kalibrasi
float PH4 = 0.242;
float PH7 = 0.211;


// Replace with your network credentials
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

// Initialize Telegram BOT
#define BOTtoken "YOUR BOT TOKEN"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "YOUR ID CHAT"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Hallo, " + from_name + ".\n";
      welcome += "Gunakan perintah-perintah berikut untuk:\n\n";
      welcome += "/makan untuk memberi pakan. \n";
      welcome += "/cek untuk mengecek kesediaan pakan dan kualitas air. \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/makan") {
      bot.sendMessage(chat_id, "Terimakasih, " + from_name + "\n" + "Smart Aquarium sudah memberi makan ikan", "");
      buttonState = HIGH;
      servo.write(0);    // Menggerakkan servo ke posisi 90 derajat
      delay(1000);       // Menunggu selama 1 detik
      servo.write(180);  // Menggerakkan servo kembali ke posisi 0 derajat
      delay(1000);       // Menunggu selama 1 detik
    }

    if (text == "/cek") {
      float temperatureC = sensors.getTempCByIndex(0);
      String status;
      if (distanceCm >= 2 && distanceCm <= 7) {
        status = "Penuh";
      } else if (distanceCm > 7 && distanceCm <= 10) {
        status = "Setengah";
      } else if (distanceCm > 10) {
        status = "Hampir habis";
      }
      String cek = "Hallo, " + from_name + ".\n";
      cek += "Berikut kondisi Aquarium saat ini:\n\n";

      cek += "Suhu Air: ";
      cek += float(temperatureC);
      cek += " *C\n";

      cek += "Nilai pH: ";
      cek += float(Po);
      cek += "\n";

      cek += "Ketersediaan Pakan: ";
      cek += String(status);
      cek += " \n";

      bot.sendMessage(chat_id, cek, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  configTime(0, 0, "pool.ntp.org");  // get UTC time via NTP
  client.setTrustAnchors(&cert);     // Add root certificate for api.telegram.org

  servo.attach(SERVO_PIN);
  pinMode(buzpin, OUTPUT);

  // Start the DS18B20 sensor
  sensors.begin();
  //Initialize the Ultrasonic HC-SR04
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input

  pinMode(ph_Pin, INPUT);  //Sets the ph Pin as an Input

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;

  // Determine the status based on the measured distance
  String status;
  if (distanceCm >= 2 && distanceCm <= 7) {
    status = "Penuh";
  } else if (distanceCm > 7 && distanceCm <= 10) {
    status = "Setengah";
  } else if (distanceCm > 10) {
    status = "Hampir habis";
  }

  //Pembacaan Ph Sensor
  int nilai_analog_PH = analogRead(ph_Pin);
  Serial.print("nilai ADC PH: ");
  Serial.println(nilai_analog_PH);
  TeganganPH = 1 / 1024.0 * nilai_analog_PH;
  Serial.print("Tegangan: ");
  Serial.println(TeganganPH, 3);

  PH_step = (PH4 - PH7) / 3;
  Po = 7.0 + ((PH7 - TeganganPH) / PH_step);

  Serial.print("Nilai PH air: ");
  Serial.println(Po, 2);
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  // Print temperature to Serial Monitor
  Serial.print(temperatureC);
  Serial.println("ºC");

  // Clear the LCD screen
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Smart");
  lcd.setCursor(4, 1);
  lcd.print("Aquarium");
  delay(500);

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Nilai PH Air: ");
  lcd.setCursor(6, 1);
  lcd.print(Po, 2);
  delay(2000);

  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Suhu: ");
  lcd.setCursor(4, 1);
  lcd.print(String(temperatureC) + " C");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kesediaan Pakan: ");
  lcd.setCursor(2, 1);
  lcd.print(status);
  if (distanceCm > 10) {
    tone(buzpin, 1000);
    delay(1000);
    noTone(buzpin);
    delay(1000);
    tone(buzpin, 1000);
    delay(1000);
    noTone(buzpin);
    delay(1000);
    tone(buzpin, 1000);
    delay(1000);
    noTone(buzpin);
    delay(1000);
  } else {
    tone(buzpin, 0);
  }
  delay(2000);

  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Smart");
  lcd.setCursor(4, 1);
  lcd.print("Aquarium");
  delay(10);


  // lcd.clear();
  // int i;
  // lcd.setCursor(0, 0);
  // lcd.print("   Nilai PH Air: " + String(Po, 2));

  // lcd.setCursor(0, 1);  //
  // lcd.print("   Suhu: " + String(temperatureC) + "*C" + " | " + "Pakan: " + String(status) + ".");

  // for (i = 0; i < 22; i++) {
  //   lcd.scrollDisplayLeft();
  //   delay(500);  
  // }

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
