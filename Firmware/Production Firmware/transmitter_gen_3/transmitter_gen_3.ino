#include <SPI.h>               //SPI
#include <nRF24L01.h>          //Radio lib 1
#include <RF24.h>              //Radio lib 2
#include <Wire.h>              //I2C
#include <Adafruit_GFX.h>      //Adafruit GFX library
#include <Adafruit_SSD1306.h>  //OLED library

#define DISPLAY_WIDTH 128  //Width in pixels of the OLED
#define DISPLAY_HEIGHT 64  //Height in pixels of the OLED

#define CE 7   //CE pin for the RF24
#define CSN 8  //CSN pin for the RF24

//Joystick pins
#define LEFT_Y_PIN A2
#define LEFT_X_PIN A3
#define RIGHT_Y_PIN A0
#define RIGHT_X_PIN A1

//Hold length
#define HOLD_LENGTH 500

//I2C pins
#define SDA A4
#define SCL A5

#define WEAPON_CONTROL_PIN A6  //The pin for the weapon motor potentiometer

//Pins for the joystick buttons
#define LEFT_BUTTON_PIN 5
#define RIGHT_BUTTON_PIN 6

//Create variables for the inputs

//Joysticks
int left_x;
int left_y;
int right_x;
int right_y;

float weapon_control;  //Weapon knob

//Joystick switches
bool left_switch;
bool right_switch;

//Last joystick switch states for scrolling behavior
bool last_left_switch;
bool last_right_switch;

bool inverted = false;  //If the robot is inverted

const byte address[6] = "737373";  //Radio ID

//enum that controls which state the transmitter is in.
enum states {
  WAIT,
  COMBAT
};

states transmitter_state = WAIT;  //Set the transmitter to the one-time-use prep mode.

int data_packet[5];  //Initialize the packet

unsigned long timer;       //The global timer that checks button inputs and things
unsigned long hold_timer;  //A timer specifically for button holds

RF24 radio(7, 8);                                                    //Create radio
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);  //Create display

void setup() {
  Serial.begin(9600);  //Start talking.

  //Setup pins

  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);

  radio.begin();                    //Start the radio.
  radio.openWritingPipe(address);   //Assign address.
  radio.setPALevel(RF24_PA_MAX);    //Highest power available.
  radio.setDataRate(RF24_250KBPS);  //Set the data rate to be slow. We don't need high data rate and slower tends to be more reliable.
  radio.stopListening();            //Don't bother looking for signals, this isn't a reciever.

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  //This is a handler from Adafruit's SSD1306 example that makes sure the screen is initialized
    Serial.println("The display broke somehow.");
    for (;;)
      ;  //Stop
  }
  display.display();              //Start the display
  printStatusMessage("Welcome");  //Greet the user because why not
}

void loop() {
  timer = millis();  //Start the global timer

  //Read EVERYTHING.
  left_x = analogRead(LEFT_X_PIN);
  left_y = analogRead(LEFT_Y_PIN);
  right_x = analogRead(RIGHT_X_PIN);
  right_y = analogRead(RIGHT_Y_PIN);
  weapon_control = analogRead(WEAPON_CONTROL_PIN);
  left_switch = digitalRead(LEFT_BUTTON_PIN);
  right_switch = digitalRead(RIGHT_BUTTON_PIN);

  //Start the main program loop!
  switch (transmitter_state) {
    case WAIT:

      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.drawFastHLine(0, 50, 128, SSD1306_WHITE);
      display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
      display.drawFastHLine(0, 16, 128, SSD1306_WHITE);
      display.setCursor(16, 0);
      display.print("Waiting!");
      display.setTextSize(1);
      display.setCursor(0, 54);
      display.print("RS: TRANSMIT");
      display.display();

      //Checks to see if you are trying to move into setup
      if (right_switch == 0) {

        //If you are, and the weapon knob is above 1% throttle, stop everything and flash lights repeatedly
        if (weapon_control > 10) {
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.drawFastHLine(0, 50, 128, SSD1306_WHITE);
          display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
          display.drawFastHLine(0, 16, 128, SSD1306_WHITE);
          display.setCursor(16, 0);
          display.print("ERROR!");
          display.setTextSize(1);
          display.setCursor(0, 54);
          display.print("Correct weapon knob.");
          display.display();
          break;
        }
        //Check if you've held the button down for 300ms
        if (timer - hold_timer >= 300) {
          //If so, move into combat
          transmitter_state = COMBAT;
        }
      } else {
        hold_timer = timer;  //Reset the timer if the user is not holding the button down
      }

      break;

    case COMBAT:


      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.drawFastHLine(0, 50, 128, SSD1306_WHITE);
      display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
      display.drawFastHLine(0, 16, 128, SSD1306_WHITE);
      display.drawFastVLine(64, 16, 34, SSD1306_WHITE);
      display.setCursor(20, 0);
      display.print("ACTIVE!");
      display.setTextSize(1);
      display.setCursor(0, 54);
      display.print("LS: WAIT MODE");
      display.setCursor(0, 21);
      display.print("LFT: " + String(map(left_y, 0, 1023, -100, 100)) + "%");
      display.setCursor(0, 30);
      display.print("RGT: " + String(map(right_y, 0, 1023, -100, 100)) + "%");
      display.setCursor(0, 39);
      display.print("WEP: " + String(map(weapon_control, 0, 1023, 0, 100)) + "%");

      display.setCursor(68, 21);
      if (inverted) {
        display.print("INV: TRUE");
      } else {
        display.print("INV: FALSE");
      }
      display.setCursor(68, 30);
      display.print("BAT: N/A");
      display.setCursor(68, 39);
      display.print("STS: N/A");

      display.display();

      //Checks to see if you've held down the left switch for 300ms
      if (left_switch == 0) {
        if (timer - hold_timer >= HOLD_LENGTH) {
          transmitter_state = WAIT;  //Move into setup
        }
      } else {
        hold_timer = timer;  //Reset the hold timer
      }

      if (right_switch == 1 && last_right_switch == 0) {
        inverted = !inverted;
      }

      //Safety check to make sure the weapon variable is okay.
      if (data_packet[0] > 1024) {
        //print danger dialogue if the weapon failed
        delay(100);
        transmitter_state = WAIT;
      }

      radio.write(&data_packet, sizeof(data_packet));  //Send data packet
      //Data is only sent in combat mode.

      break;
  }

  //The main program loop has ended. Now write all the data pieces to the array.

  //Check if the invert switch is switched and if it is, invert all the data from the joysticks
  if (inverted) {
    data_packet[1] = map(analogRead(LEFT_Y_PIN), 0, 1023, 1023, 0);
    data_packet[2] = map(analogRead(RIGHT_Y_PIN), 0, 1023, 1023, 0);
  } else {
    data_packet[1] = analogRead(RIGHT_Y_PIN);
    data_packet[2] = analogRead(LEFT_Y_PIN);
  }

  data_packet[0] = weapon_control;
  data_packet[3] = inverted;
  data_packet[4] = timer;


  //Update the last switch states
  last_left_switch = left_switch;
  last_right_switch = right_switch;
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
