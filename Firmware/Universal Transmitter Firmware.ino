//By IceChes under the MIT license. 
//This firmware is for a two-joystick transmitter with one pot and one switch, linked to a robot using my Universal Robot Firmware.

//Include required libraries.
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Create variables for the inputs
int joyLeft;
int joyRight;
int motorPot;
int toggleSwitch;

int dataPacket[4]; //Initialize the packet

RF24 radio(10, 9); //Set up radio mode

const byte address[6] = "737373"; //Set the radio ID. CHANGE THIS IN YOUR OWN ROBOTS TO AVOID CONFLICTS!!!

void setup() {
  Serial.begin(9600); //Start talking.

  pinMode(2, INPUT_PULLUP); //Set up the switch pin.
  
  radio.begin(); //Start the radio.
  radio.openWritingPipe(address); //Assign address.
  radio.setPALevel(RF24_PA_MAX); //IT'S OVER 9000!
  radio.setDataRate(RF24_250KBPS); //Set the data rate to be slow. We don't need high data rate and slower tends to be more reliable.
  radio.stopListening(); //Don't bother looking for signals, this isn't a reciever.
}

void loop() {
  joyLeft = analogRead(A0); //Reads left joystick and assigns it to joyLeft
  joyRight = analogRead(A2); //Reads right joystick and assigns it to joyRight
  motorPot = analogRead(A4); //Reads potentiometer and assigns it to motorPot
  toggleSwitch = digitalRead(2); //Reads switch and assigns it to toggleSwitch

  //Assigns the variables to different parts of the packet.
  dataPacket[0] = motorPot;
  dataPacket[1] = joyRight;
  dataPacket[2] = joyLeft;
  dataPacket[3] = toggleSwitch;

  //Debugging.
  Serial.println(joyRight);
  Serial.println(joyLeft);
  Serial.println(motorPot);
  Serial.println(toggleSwitch);
  
  radio.write(&dataPacket, sizeof(dataPacket)); //Send data packet
  delay(5); //Slow the loop down so we don't overload the reciever as it tries to process everything.
}
