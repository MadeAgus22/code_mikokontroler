#include <WiFi.h>
#include <HTTPClient.h>

#define TRIGGER_PIN  12
#define ECHO_PIN     14

const char* ssid = "Your SSID";
const char* password = "Your PASWORD";

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  long duration, distance;
  
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;  // Convert to cm

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin("http://192.168.xxx.xxx:8080/customer");  // Ganti dengan URL endpoint API Anda
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "names=" + String(distance); // names = diganti sesuai post data
    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(10000);  // Kirim data setiap 10 detik
}
