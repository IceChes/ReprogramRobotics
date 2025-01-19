//By IceChes under the MIT license.

//Include required libraries.
#include <SPI.h> //SPI
#include <nRF24L01.h> //Radio lib 1
#include <RF24.h> //Radio lib 2
#include <Wire.h> //I2C
#include <Adafruit_GFX.h> //Adafruit GFX library
#include <Adafruit_SSD1306.h> //OLED library

#define displayWidth 128 //Width in pixels of the OLED
#define displayHeight 32 //Height in pixels of the OLED

#define CE 7 //CE pin for the RF24
#define CSN 8 //CSN pin for the RF24

//Joystick pins
#define leftYPin A0 
#define leftXPin A1
#define rightYPin A2
#define rightXPin A3

//I2C pins
#define SDA A4
#define SCL A5

#define motorPotPin A7 //The pin for the weapon motor potentiometer
#define invertSwitchPin 6 //The pin used for the inversion switch

//Pins for the joystick buttons
#define leftButtonPin 2 
#define rightButtonPin 3

//Pins for the left and right LEDs
#define leftLEDPin 4
#define rightLEDPin 5

//Create variables for the inputs

//Joysticks
int leftX;
int leftY;
int rightX;
int rightY;

float motorPot; //Weapon knob
bool invertSwitch; //Inversion switch

//Joystick switches
bool leftSwitch; 
bool rightSwitch;

//Last joystick switch states for scrolling behavior
bool lastLeftSwitch;
bool lastRightSwitch;

bool inverted = false; //If the robot is inverted
float weaponCurveFactor; //Exponent to curve the weapon to
int weaponLimit; //Value to limit the weapon throttle to
byte settingIndex = 2; //Value that controls which setting is being displayed in the setup menu

const byte address[6] = "737373"; //Radio ID

//enum that controls which state the transmitter is in.
enum states {
  PREP,
  COMBAT,
  SETUP
};

states transmitterState = PREP; //Set the transmitter to the one-time-use prep mode.

int dataPacket[5]; //Initialize the packet

unsigned long timer; //The global timer that checks button inputs and things
unsigned long holdTimer; //A timer specifically for button holds

RF24 radio(7, 8); //Create radio
Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, -1); //Create display

void setup() {
  Serial.begin(9600); //Start talking.

  //Setup pins
  pinMode(leftLEDPin, OUTPUT);
  pinMode(rightLEDPin, OUTPUT);
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(invertSwitchPin, INPUT_PULLUP);

  radio.begin(); //Start the radio.
  radio.openWritingPipe(address); //Assign address.
  radio.setPALevel(RF24_PA_MAX); //Highest power available.
  radio.setDataRate(RF24_250KBPS); //Set the data rate to be slow. We don't need high data rate and slower tends to be more reliable.
  radio.stopListening(); //Don't bother looking for signals, this isn't a reciever.

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //This is a handler from Adafruit's SSD1306 example that makes sure the screen is initialized
    Serial.println("The display broke somehow lol."); 
    for (;;); //Stop

  }
  
  display.display(); //Start the display
  printStatusMessage("Welcome"); //Greet the user because why not
  delay(50); //Wait a sec
}

void loop() {
  timer = millis(); //Start the global timer

  //Read EVERYTHING.
  leftX = analogRead(leftXPin);
  leftY = analogRead(leftYPin);
  rightX = analogRead(rightXPin);
  rightY = analogRead(rightYPin);
  motorPot = analogRead(motorPotPin);
  invertSwitch = digitalRead(invertSwitchPin);
  leftSwitch = digitalRead(leftButtonPin);
  rightSwitch = digitalRead(rightButtonPin);

  //Start the main program loop!
  switch (transmitterState) {
    case PREP:
      //Flash because yes
      blinkLeftLED();
      blinkRightLED();
      
      transmitterState = SETUP; //Move into the setup sequence.
      break; 
    case COMBAT:

      digitalWrite(leftLEDPin, 1); //Turn on the left LED for status indication
      printStatusMessageWithFooter("DANGER", "Hold LS for setup."); //Print DANGER in big letters
      
      //Checks to see if you've held down the left switch for 300ms
      if (leftSwitch == 0) {
        if (timer - holdTimer >= 300) {
          transmitterState = SETUP; //Move into setup
          blinkRightLED();
        }
      }
      else {
        holdTimer = timer; //Reset the hold timer
      }

      //Safety check to make sure the weapon variable is okay.
      if(dataPacket[0] > 1024){
        printStatusMessageWithFooter("DANGER", "WEAPON FAILED CRITICALLY");
        delay(100);
        transmitterState = SETUP;
      }

      radio.write(&dataPacket, sizeof(dataPacket)); //Send data packet
      //Data is only sent in combat mode.

      break;
    case SETUP:
  
      digitalWrite(leftLEDPin, 0); //Turn off the left LED.
      
      //Checks to see if you are trying to move into setup
      if (rightSwitch == 0) {
        
        //If you are, and the weapon knob is above 1% throttle, stop everything and flash lights repeatedly
        if (motorPot > 10) {
          printStatusMessageWithFooter("STOPPED", "Correct weapon knob!");
          blinkLeftLED();
          blinkRightLED();
          break;
        }
        //Check if you've held the button down for 300ms
        if (timer - holdTimer >= 300) {
          //If so, move into combat
          transmitterState = COMBAT;
          blinkRightLED();
        }
      }
      else {
        holdTimer = timer; //Reset the timer if the user is not holding the button down
      }

      //Handler for scrolling through settings
      if (lastLeftSwitch != leftSwitch && lastLeftSwitch == 1 && settingIndex < 2) {
        settingIndex++;
      }
      else if (lastLeftSwitch != leftSwitch && lastLeftSwitch == 1 && settingIndex == 2) {
        settingIndex = 0;
      }

      switch (settingIndex) {
        case 0:
          //Setting to change the weapon curve factor
          weaponCurveFactor = map(motorPot, 0, 1000, 0, 10);
          print2LineMessage("Weapon Curve", String(weaponCurveFactor));
          break;
        case 1:
          //Setting to change the weapon limit
          weaponLimit = motorPot;
          print2LineMessage("Weapon Limit", String(map(motorPot, 0, 1023, 0, 100))); //This map setting is here to make 0-1023 look like 0-100 and that is it
          break;
        case 2:
          //A standby setting. Does nothing. This is the default power-on state.
          print3LineMessage("STANDBY", "Hold RS to start.", "Press LS for setup.");
      }
      break;
  }

  //The main program loop has ended. Now write all the data pieces to the array.

  //Check if the invert switch is switched and if it is, invert all the data from the joysticks
  if (invertSwitch) { 
    dataPacket[1] = map(analogRead(leftYPin), 0, 1023, 1023, 0);
    dataPacket[2] = map(analogRead(rightYPin), 0, 1023, 1023, 0);
    digitalWrite(rightLEDPin, 1);
  }
  else {
    dataPacket[1] = analogRead(rightYPin);
    dataPacket[2] = analogRead(leftYPin);
    digitalWrite(rightLEDPin, 0);
  }

  //Write the weapon knob data, dictated by the configurable curve. Also write the invert switch because some bots need that.
  dataPacket[0] = map(motorPot * pow(motorPot / 1023, weaponCurveFactor), 0, 1023, 0, weaponLimit);
  dataPacket[3] = invertSwitch;

  dataPacket[4] = timer;

  //Print everything
  Serial.println(dataPacket[0]);
  Serial.println(dataPacket[1]);
  Serial.println(dataPacket[2]);
  Serial.println(dataPacket[3]);

  //Update the last switch states
  lastLeftSwitch = leftSwitch;
  lastRightSwitch = rightSwitch;

}

//Function defs. These are all super self explanatory
void printStatusMessage(String message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(message);
  display.display();
}

void printStatusMessageWithFooter(String message, String footer) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.println(message);
  display.setTextSize(1);
  display.setCursor(5, 20);
  display.println(footer);
  display.display();
}

void printSmallMessage(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.println(message);
  display.display();
}

void print2LineMessage(String message1, String message2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.println(message1);
  display.setCursor(5, 20);
  display.println(message2);
  display.display();
}

void print3LineMessage(String message1, String message2, String message3) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.println(message1);
  display.setCursor(5, 12);
  display.println(message2);
  display.setCursor(5, 24);
  display.println(message3);
  display.display();
}

void blinkRightLED() {
  digitalWrite(rightLEDPin, 1);
  delay(10);
  digitalWrite(rightLEDPin, 0);
}

void blinkLeftLED() {
  digitalWrite(leftLEDPin, 1);
  delay(10);
  digitalWrite(leftLEDPin, 0);
}