#define TRIGGER_PIN 10
#define ECHO_PIN 9
#define RELAY_PIN 7

void setup() {
  Serial.begin(9600);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  long duration, distance;

  // Mengirim sinyal ultrasonik
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Menerima waktu pantulan ultrasonik
  duration = pulseIn(ECHO_PIN, HIGH);

  // Menghitung jarak
  distance = duration * 0.034 / 2;

  // Menampilkan jarak pada Serial Monitor
  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Mengendalikan relay
  if (distance > 60) {
    // Jarak lebih dari 60 cm, nyalakan relay
    digitalWrite(RELAY_PIN, HIGH);
  } else if (distance < 35) {
    // Jarak kurang dari 20 cm, matikan relay
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(1000);  // Delay untuk stabilitas pembacaan
}
