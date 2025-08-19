#include <Bluepad32.h>
#include <ESP32Servo.h>
#include <SoftwareSerial.h>

#define WEAPON_PIN 0
#define SOFT_SERIAL_TX_PIN 0
#define SOFT_SERIAL_RX_PIN 0

#define MOTOR_1_MINIMUM_VALUE 0
#define MOTOR_1_MAXIMUM_VALUE 200
#define MOTOR_2_MINIMUM_VALUE 0
#define MOTOR_2_MAXIMUM_VALUE 200

#define CURVE 0  //set to 0 to disable
#define AUTO_BRAKE true
#define FAILSAFE_BRAKE false
#define INVERT_MOTOR_1 false
#define INVERT_MOTOR_2 true

#define STOP_TIMEOUT 1000

Servo esc;
EspSoftwareSerial::UART snapSerial;

int esc_value = 123;
int motor_1_value;
int motor_2_value;

byte curve = 0;



ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void triggerFailsafe() {
  //Slowly throttle down the weapon motor. Stopping something spinning with that much inertia instantly is a really bad idea.
  for (int esc_stop = esc_value; esc_stop >= 0; esc_stop--) {
    esc.write(esc_stop);
    Serial.println("ERROR: STOPPING ESC! Current value: " + String(map(esc_stop, 0, 180, 0, 100)) + "%");
    delay(5);
  }
  esc.detach();  //Detach the weapon ESC.
  snapSerial.write(0xFF);
}


//Bluepad32 connection callback. I don't know how this works, but I am going to add some failsafe code in there.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.println("BLUEPAD: Controller is connected!");
      
      myControllers[i] = ctl;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println("BLUEPAD: Controller connected, but could not found empty slot");
  }
}

//Bluepad32 disconnect callback
void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.println("BLUEPAD: Lost controller connection");
      triggerFailsafe();
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void dumpGamepad(ControllerPtr ctl) {
  //huge mess because idk how printf works
  Serial.println("BLUEPAD: DPAD: " + String(ctl->dpad()) + ", BUTTONS: " + String(ctl->buttons()) + ", LS: " + String(ctl->axisY()) + ", RS: " + String(ctl->axisRY()) + ", LT: " + String(ctl->brake()) + ", RT: " + String(ctl->throttle()));
}

void processGamepad(ControllerPtr ctl) {




  //This was in the example code. Saving it for later.
  //ctl->playDualRumble(0 /* delayedStartMs */, 2000 /* durationMs */, 255 /* weakMagnitude */, 255 /* strongMagnitude */);
  dumpGamepad(ctl);
}

//I have no idea what this does. This is a great library, if only they would document it.
void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      processGamepad(myController);
    }
  }
}

void setup() {
  snapSerial.begin(9600);
  Serial.begin(115200);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);
}

// Arduino loop function. Runs in CPU 1.
void loop() {
  // This call fetches all the controllers' data.
  // Call this function in your main loop.
  bool dataUpdated = BP32.update();
  if (dataUpdated) {
    processControllers();
  }


  // The main loop must have some kind of "yield to lower priority task" event.
  // Otherwise, the watchdog will get triggered.
  // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
  // Detailed info here:
  // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

  //     vTaskDelay(1);
}
