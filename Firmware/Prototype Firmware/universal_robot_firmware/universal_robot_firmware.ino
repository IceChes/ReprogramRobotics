//By IceChes under the MIT license. 
//This firmware is for a two wheel drive robot with one weapon, linked to a transmitter using my Universal Transmitter Firmware.

//Include required libraries.
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(7, 8); //Attach radio on CE 7 and CSN 8.

//Create servo objects for the weapon and drive motors.
Servo esc;
Servo drive1;
Servo drive2;

int packet[5]; //Initialize the packet
int last_check = 0;

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

  //Attach ESCs. I used pin 5 for the weapon and 2 and 3 for the drive. 
  esc.attach(4, 1000, 2000);
  drive2.attach(2, 1000, 2000);
  drive1.attach(3, 1000, 2000);
    
  radio.begin(); //Start the radio.
  radio.openReadingPipe(0, address); //Not sure what this does.
  radio.setDataRate(RF24_250KBPS); //Set the data rate to be slow. We don't need high data rate and slower tends to be more reliable.
  radio.setPALevel(RF24_PA_MAX); //I'm not sure this is needed on the reciever side.
  radio.startListening(); //Start listening for data.

  Serial.println("It's robot fighting time!"); //Let the user know the setup was completed.
}

void loop() {
  timer = millis(); //Set the timer.
  if (radio.available()) { //Check if the radio is available.
    radio.read(&packet, sizeof(packet)); //Read the packet.

    //Print all the packet data.
    Serial.println(packet[0]);
    Serial.println(packet[1]);
    Serial.println(packet[2]);
    Serial.println(packet[3]);

    //Map the raw data values from the transmitter to usable values for the ESCs.
    escWrite = map(packet[0], 0, 1000, 1000, 2000);
    drive1Write = map(packet[1], 1000, 2, 2000, 1000);
    drive2Write = map(packet[2], 2, 1000, 2000, 1000);

    //Send the data.
    esc.writeMicroseconds(escWrite);
    drive1.writeMicroseconds(drive1Write);
    drive2.writeMicroseconds(drive2Write);

    if(packet[4] == last_check){
      //Detach both drive ESCs instantly.
      drive1.detach(); 
      drive2.detach();

      //Slowly throttle down the weapon motor. Stopping something spinning with that much inertia instantly is a really bad idea.
      for (int escStop = escWrite; escStop >= 1000; escStop--) {
        esc.write(escStop);
        delay(2);
      }
      esc.detach(); //Detach the weapon ESC.
      Serial.println("TX RADIO NOT AVAILABLE"); //Complain.
    }

    //Update the success timer. Note that the success timer only updates if a radio signal has been recieved.
    lastSuccess = timer;
    last_check = packet[4];
  }
  else {
    if (timer - lastSuccess >= 300) { //If it's been a while since the last success...
      
      //Detach both drive ESCs instantly.
      drive1.detach(); 
      drive2.detach();

      //Slowly throttle down the weapon motor. Stopping something spinning with that much inertia instantly is a really bad idea.
      for (int escStop = escWrite; escStop >= 1000; escStop--) {
        esc.write(escStop);
        delay(2);
      }
      esc.detach(); //Detach the weapon ESC.
      Serial.println("TX RADIO NOT AVAILABLE"); //Complain.
    }
  }
}
