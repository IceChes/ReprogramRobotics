

void setup() {
  Serial.begin(9600);
}
void loop() {
  if (Serial.available()) {
    digitalWrite(5, 1);
    digitalWrite(6, 0);
  }
}
