
//By IceChes under the MIT license.
//This firmware is for a two wheel drive robot with one weapon, linked to a transmitter using my Universal Transmitter Firmware.

//Include required libraries.
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define RF_CE 7
#define RF_CSN 8
#define weaponPin 5
#define output1MinVal 5
#define output1MaxVal 99
#define output2MinVal 105
#define output2MaxVal 199
#define emergencyVal 50
#define timeout 200
#define debugLED 13

RF24 radio(RF_CE, RF_CSN); //Attach radio on CE 7 and CSN 8.

Servo esc;

int packet[5]; //Initialize the packet
int last_check = 0;

int motor1;
int motor2;

const byte address[6] = "737373"; //Set the radio ID. CHANGE THIS IN YOUR OWN ROBOTS TO AVOID CONFLICTS!!!

//Create variables for what to write to the motors.
int escWrite;
int drive1Write;
int drive2Write;

//Create variables for the failsafe timers.
unsigned long timer;
unsigned long lastSuccess;

void setup() {
  Serial.begin(9600); //Start serial monitor.
  esc.attach(weaponPin);

  pinMode(debugLED, OUTPUT);

  radio.begin(); //Start the radio.
  radio.openReadingPipe(0, address); //Not sure what this does.
  radio.setDataRate(RF24_250KBPS); //Set the data rate to be slow. We don't need high data rate and slower tends to be more reliable.
  radio.setPALevel(RF24_PA_MAX); //I'm not sure this is needed on the reciever side.
  radio.startListening(); //Start listening for data.
  delay(50);
}

void loop() {
  timer = millis();
  if (radio.available()) {
    digitalWrite(debugLED, 0);
    if(!esc.attached()){
      esc.attach(weaponPin);
    }
    Serial.begin(9600);

    //Read
    radio.read(&packet, sizeof(packet));

    //Print all received data.

    motor1 = map(packet[1], 0, 1023, output1MinVal, output1MaxVal);
    motor2 = map(packet[2], 0, 1023, output2MinVal, output2MaxVal);

    Serial.write(motor1);
    Serial.write(motor2);
   


    escWrite = map(packet[0], 10, 1023, 0, 180);

    esc.write(escWrite);

    if(packet[4] == last_check){
      //Detach both drive ESCs instantly.


      //Slowly throttle down the weapon motor. Stopping something spinning with that much inertia instantly is a really bad idea.
      for (int escStop = escWrite; escStop >= 1000; escStop--) {
        esc.write(escStop);
        delay(2);
      }
      esc.detach(); //Detach the weapon ESC.
      Serial.end();
    }

    last_check = packet[4];
    digitalWrite(debugLED, 1);
    lastSuccess = timer;
  }
  else {
    if (timer - lastSuccess > timeout) {
      Serial.end();

      //Slowly throttle down the weapon motor. Stopping something spinning with that much inertia instantly is a really bad idea.
      for (int escStop = escWrite; escStop >= 0; escStop--) {
        esc.write(escStop);
        delay(5);
      }
      digitalWrite(debugLED, 1);
      esc.detach(); //Detach the weapon ESC.
    }
    else {
      esc.write(escWrite);
    }
  }
}
