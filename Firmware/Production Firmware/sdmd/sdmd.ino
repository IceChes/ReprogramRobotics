//Version 2.0.0

#define WRITE_MOTOR_1 0xC9  //writing motors: write motor command followed by data value between 0 and 200
#define WRITE_MOTOR_2 0xCA
#define BRAKE_MOTOR_1 0xCB  //stops motor
#define BRAKE_MOTOR_2 0xCC
#define COAST_MOTOR_1 0xCD  //disables output
#define COAST_MOTOR_2 0xCE
#define CURVE_SET_INTENSITY 0xCF  //curve set: curve set command followed by exponent value
#define CURVE_OFF 0xD0            //disable curving
#define BRAKE_ON 0xD1             //enables braking
#define BRAKE_OFF 0xD2            //disables braking
#define BRAKE_ON_FAILSAFE 0xD3    //brakes on failsafe
#define COAST_ON_FAILSAFE 0xD4    //coasts on failsafe
#define FAILSAFE 0xFF             //stops or brakes both motors depending on brake_on_failsafe

#define MOTOR_1_PIN_A 9
#define MOTOR_1_PIN_B 10
#define MOTOR_2_PIN_A 5
#define MOTOR_2_PIN_B 6

#define BRAKE_TIMEOUT 100  //set this to determine the idle timeout before braking is enabled

//user vars
bool brake = false;               //set this to false to disable braking after 100ms
bool brake_on_failsafe = true;  //set this to false to disable braking on failsafe

//incoming data var
byte data;
int mapped_data;

//settings vars
int curve_factor = 1;

//timer vars
unsigned long timer;
unsigned long last_timer;
unsigned long last_motor_1_active;
unsigned long last_motor_2_active;

//activity vars
bool motor_1_active = false;
bool motor_2_active = false;

//motor data vars
int motor_1_pin_a_data = 0;
int motor_1_pin_b_data = 0;
int motor_2_pin_a_data = 0;
int motor_2_pin_b_data = 0;

//settings enum
enum setting_entry {
  UNDEF,
  CURVE_SET,
};

setting_entry setting = UNDEF;

void setup() {
  Serial.begin(9600);  //start serial comm
}

void loop() {
  timer = millis();                            //enable timer
  last_timer = timer;                          //refresh timer
  mapped_data = map(data, 0, 200, -100, 100);  //calculate mapped data

  //handle motor 1 writing
  if (data < 201 && motor_1_active && !motor_2_active) {
    if (mapped_data > 5) {                                                                                  //fwd | the 5/-5 is a deadzone
      last_motor_1_active = timer;                                                                          //update activity timer
      motor_1_pin_a_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);  //we are mapping the throttle alone the equation |x(x/100)^n| where x is the mapped data and n is the curve factor
      motor_1_pin_b_data = 0;
    } else if (mapped_data < -5) {  //back
      last_motor_1_active = timer;
      motor_1_pin_a_data = 0;
      motor_1_pin_b_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);
    } else if (!brake || (timer - motor_1_active < BRAKE_TIMEOUT)) {  //we can't brake immediately, that would cause voltage spikes
      motor_1_pin_a_data = 0;
      motor_1_pin_b_data = 0;
    } else {
      motor_1_pin_a_data = 255;
      motor_1_pin_b_data = 255;
    }
  }
  //all that again but for the other motor
  if (data < 201 && motor_2_active && !motor_1_active) {
    if (mapped_data > 5) {
      last_motor_2_active = timer;
      motor_2_pin_a_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);
      motor_2_pin_b_data = 0;
    } else if (mapped_data < -5) {
      last_motor_2_active = timer;
      motor_2_pin_a_data = 0;
      motor_2_pin_b_data = map(abs(mapped_data * pow(mapped_data / 100.0, curve_factor)), 0, 100, 0, 255);
    } else if (!brake || (timer - motor_2_active < BRAKE_TIMEOUT)) {
      motor_2_pin_a_data = 0;
      motor_2_pin_b_data = 0;
    } else {
      motor_2_pin_a_data = 255;
      motor_2_pin_b_data = 255;
    }
  }
  //handle settings
  switch (setting) {

    case CURVE_SET:
      if (data < 201) {
        curve_factor = data;
      }
      break;
      //add more settings here
  }

  if (Serial.available()) {
    data = Serial.read();
    switch (data) {
      case WRITE_MOTOR_1:
        setting = UNDEF;  //disable settings mode to prevent conflicts
        motor_1_active = true;
        motor_2_active = false;
        break;  //only 1 motor or the settings mode can be enabled at any given time or there will be conflicts

      case WRITE_MOTOR_2:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = true;
        break;

      case BRAKE_MOTOR_1:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        //direct command - write to motors directly
        motor_1_pin_a_data = 255;
        motor_1_pin_b_data = 255;
        break;

      case BRAKE_MOTOR_2:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        motor_2_pin_a_data = 255;
        motor_2_pin_b_data = 255;
        break;

      case COAST_MOTOR_1:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        motor_1_pin_a_data = 0;
        motor_1_pin_b_data = 0;
        break;

      case COAST_MOTOR_2:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;

        motor_2_pin_a_data = 0;
        motor_2_pin_b_data = 0;
        break;

      case CURVE_SET_INTENSITY:
        setting = CURVE_SET;
        motor_1_active = false;
        motor_2_active = false;
        break;

      case CURVE_OFF:
        curve_factor = 0;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        break;

      case BRAKE_ON:
        brake = true;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        break;

      case BRAKE_OFF:
        brake = false;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        break;

      case BRAKE_ON_FAILSAFE:
        brake_on_failsafe = true;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        break;

      case COAST_ON_FAILSAFE:
        brake_on_failsafe = false;

        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;
        break;

      case FAILSAFE:
        setting = UNDEF;
        motor_1_active = false;
        motor_2_active = false;


        motor_1_pin_a_data = 255 * brake_on_failsafe; //if brake_on_failsafe is 0, this will evaluate to 0, if it's 1 it will evaluate to 255
        motor_1_pin_b_data = 255 * brake_on_failsafe;
        motor_2_pin_a_data = 255 * brake_on_failsafe;
        motor_2_pin_b_data = 255 * brake_on_failsafe;

        break;
    }
  }
  //write all our data we assigned
  analogWrite(MOTOR_1_PIN_A, motor_1_pin_a_data);
  analogWrite(MOTOR_1_PIN_B, motor_1_pin_b_data);
  analogWrite(MOTOR_2_PIN_A, motor_2_pin_a_data);
  analogWrite(MOTOR_2_PIN_B, motor_2_pin_b_data);
}
