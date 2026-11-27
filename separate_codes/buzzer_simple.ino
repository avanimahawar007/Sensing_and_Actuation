#define BUZZER_PIN 6

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("Actuator Test Started");
}

void loop() {
  Serial.println("BUZZER ON");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(2000);

  digitalWrite(BUZZER_PIN, LOW);
  delay(1000);
}