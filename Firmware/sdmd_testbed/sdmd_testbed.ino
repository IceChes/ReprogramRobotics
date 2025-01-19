
void setup() {
  Serial.begin(9600);
}

void loop() {
  if(Serial.read() == 74){
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);
  }
}
