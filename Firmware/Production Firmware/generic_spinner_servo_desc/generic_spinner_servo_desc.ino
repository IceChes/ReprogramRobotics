//Include required libraries.
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define RF_CE 7
#define RF_CSN 8
#define WEAPON_PIN 4
#define MOTOR_A_PIN 2
#define MOTOR_B_PIN 3
#define LED_PIN 13

#define MOTOR_1_MINIMUM_VALUE 30
#define MOTOR_1_MAXIMUM_VALUE 150
#define MOTOR_2_MINIMUM_VALUE 30
#define MOTOR_2_MAXIMUM_VALUE 150
#define INVERT_MOTOR_1 true
#define INVERT_MOTOR_2 false

#define STOP_TIMEOUT 1000

RF24 radio(RF_CE, RF_CSN);  //Attach radio on CE 7 and CSN 8

Servo esc;  //Create esc
Servo motor_a;
Servo motor_b;

int packet[5];  //Initialize the packet

const byte address[6] = "737373";  //Set the radio ID

//Create variables for what to write to the motors
int esc_value;
int motor_1_value;
int motor_2_value;

//Create variables for the failsafe timers
unsigned long timer;
unsigned long last_success;

//For the lag checker
int last_check_value = 0;

void setup() {
  delay(50);           //The receiver has to start AFTER the motor driver does
  Serial.begin(9600);  //Start serial

  pinMode(LED_PIN, OUTPUT);

  radio.begin();  //Start the radio
  radio.openReadingPipe(0, address);
  radio.setDataRate(RF24_250KBPS);  //We don't need high data rate and slower tends to be more reliable
  radio.startListening();           //Start listening for data
}

void loop() {
  timer = millis();
  if (radio.available()) {
    digitalWrite(LED_PIN, 0);
    if (!esc.attached()) {
      esc.attach(WEAPON_PIN);
    }
    if (!motor_a.attached()) {
      motor_a.attach(MOTOR_A_PIN, 1000, 2000);
    }
    if (!motor_b.attached()) {
      motor_b.attach(MOTOR_B_PIN, 1000, 2000);
    }

    //Read
    radio.read(&packet, sizeof(packet));

    //Print all received data.

    if (INVERT_MOTOR_1) {
      motor_1_value = map(packet[1], 1023, 0, MOTOR_1_MINIMUM_VALUE, MOTOR_1_MAXIMUM_VALUE);
    } else {
      motor_1_value = map(packet[1], 0, 1023, MOTOR_1_MINIMUM_VALUE, MOTOR_1_MAXIMUM_VALUE);
    }

    if (INVERT_MOTOR_2) {
      motor_2_value = map(packet[2], 1023, 0, MOTOR_2_MINIMUM_VALUE, MOTOR_2_MAXIMUM_VALUE);
    } else {
      motor_2_value = map(packet[2], 0, 1023, MOTOR_2_MINIMUM_VALUE, MOTOR_2_MAXIMUM_VALUE);
    }

    esc_value = map(packet[0], 10, 1023, 0, 180);


    esc.write(esc_value);
    motor_a.write(motor_1_value);
    motor_b.write(motor_2_value);

    Serial.println(esc_value);
    Serial.println(motor_1_value);
    Serial.println(motor_2_value);
    Serial.println(packet[4]);

    if (packet[4] == last_check_value) {
      //Detach both drive ESCs instantly.
      //Slowly throttle down the weapon motor
      for (int esc_stop = esc_value; esc_stop >= 0; esc_stop--) {
        esc.write(esc_stop);
        delay(2);
      }
      motor_a.write(90);
      motor_b.write(90);
      esc.detach();      //Detach the weapon ESC.
      motor_a.detach();  //Detach the weapon ESC.
      motor_b.detach();  //Detach the weapon ESC.
    }

    last_check_value = packet[4];
    digitalWrite(LED_PIN, 1);
    last_success = timer;

    delay(10);

  } else {
    if (timer - last_success > STOP_TIMEOUT) {
      //Slowly throttle down the weapon motor. Stopping something spinning with that much inertia instantly is a really bad idea.
      for (int esc_stop = esc_value; esc_stop >= 0; esc_stop--) {
        esc.write(esc_stop);
        delay(5);
      }
      digitalWrite(LED_PIN, 1);
      motor_a.write(90);
      motor_b.write(90);
      esc.detach();      //Detach the weapon ESC.
      motor_a.detach();  //Detach the weapon ESC.
      motor_b.detach();  //Detach the weapon ESC.
    } else {
      esc.write(esc_value);
    }
  }
}
