#define WRITE_MOTOR_1 0xC9
#define WRITE_MOTOR_2 0xCA
#define BRAKE_MOTOR_1 0xCB
#define BRAKE_MOTOR_2 0xCC
#define COAST_MOTOR_1 0xCD
#define COAST_MOTOR_2 0xCE
#define SMOOTHING_SET_SAMPLES 0xCF
#define SMOOTHING_OFF 0xD0
#define CURVE_SET_INTENSITY 0xD1
#define CURVE_OFF 0xD2
#define EMERGENCY_STOP 0xD3

#define MOTOR_1_PIN_A 9
#define MOTOR_1_PIN_B 10
#define MOTOR_2_PIN_A 5
#define MOTOR_2_PIN_B 6

#define TIMEOUT 100

bool brake = false;

byte data = 0xCF;

int smooth_factor = 0;
int curve_factor = 1;

unsigned long timer;
unsigned long last_success;

bool motor_1_writing = true;
bool motor_2_writing = false;

float mapped_data;

enum setting_entry {
  UNDEF,
  SMOOTH_SET,
  CURVE_SET
};


setting_entry setting = UNDEF;

void setup() {
  Serial.begin(9600);
}

void loop() {
  timer = millis();

  last_success = timer;
  data = 200;


  if (motor_1_writing && data < 250) {
    mapped_data = map(data, 0, 200, -100, 100);
    if (mapped_data > 5) {
      //Serial.println(mapped_data);
      Serial.println();
    }
  }
}
