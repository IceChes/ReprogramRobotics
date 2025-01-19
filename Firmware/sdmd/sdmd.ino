#define midpoint 50 
#define lowpoint 1
#define highpoint 100
#define maxinput 200

#define maxoutput 255

#define motor1pin1 9
#define motor1pin2 10
#define motor2pin1 5
#define motor2pin2 6

#define safety 100

bool brake = false;

byte data = 255;
byte motor1Data = 255;
byte motor2Data = 255;

int motor1Write = 0;
int motor2Write = 0;

unsigned long timer;
unsigned long lastSuccess;

enum motorState {
  BRAKE,
  FORWARD,
  BACKWARD,
  OFF
};

motorState motor1CurrentState = OFF;
motorState motor2CurrentState = OFF;

void setup() {
  Serial.begin(9600);
}

void loop() {
  timer = millis();
  if (Serial.available()) {

    lastSuccess = timer;
    data = Serial.read();

    if (data <= highpoint) {
      motor1Data = data;

      if (motor1Data < midpoint && motor1Data > lowpoint) {
        motor1CurrentState = BACKWARD;
      }
      else if (motor1Data > midpoint && motor1Data < highpoint) {
        motor1CurrentState = FORWARD;
      }
      else if (motor1Data == lowpoint or motor1Data == midpoint) {
        if (brake == true){
          motor1CurrentState = BRAKE;
        }
        else{
          motor1CurrentState = OFF;
        }
      }
    }
    else if (data > highpoint) {
      motor2Data = map(data, 100, 200, 0, highpoint);

      if (motor2Data < midpoint && motor2Data > lowpoint) {
        motor2CurrentState = BACKWARD;
      }
      else if (motor2Data > midpoint && motor2Data < highpoint) {
        motor2CurrentState = FORWARD;
      }
      else if (motor2Data == lowpoint or motor2Data == midpoint) {
        motor2CurrentState = BRAKE;
      }
    }
    switch (motor1CurrentState) {
      case BRAKE:
        digitalWrite(motor1pin1, 1);
        digitalWrite(motor1pin2, 1);
        break;
      case OFF:
        digitalWrite(motor1pin1, 0);
        digitalWrite(motor1pin2, 0);
        break;
      case FORWARD:
        analogWrite(motor1pin1, map(motor1Data, midpoint, highpoint, 0, maxoutput));
        digitalWrite(motor1pin2, 0);
        break;
      case BACKWARD:
        digitalWrite(motor1pin1, 0);
        analogWrite(motor1pin2, map(motor1Data, highpoint, midpoint, 0, maxoutput));
        break;
    }
    switch (motor2CurrentState) {
      case BRAKE:
        digitalWrite(motor2pin1, 1);
        digitalWrite(motor2pin2, 1);
        break;
      case OFF:
        digitalWrite(motor2pin1, 0);
        digitalWrite(motor2pin2, 0);
        break;
      case FORWARD:
        analogWrite(motor2pin1, map(motor2Data, 50, 100, 0, maxoutput));
        digitalWrite(motor2pin2, 0);
        break;
      case BACKWARD:
        digitalWrite(motor2pin1, 0);
        analogWrite(motor2pin2, map(motor2Data, 100, 50, 0, maxoutput));
        break;
    }
  }
  else {

    if (timer - lastSuccess > safety) {
      digitalWrite(motor2pin1, 0);
      digitalWrite(motor2pin2, 0);
      digitalWrite(motor1pin1, 0);
      digitalWrite(motor1pin2, 0);
    }
  }
}
