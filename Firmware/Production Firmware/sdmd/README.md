# SDMD Firmware
## Introduction
The SDMD firmware uses a custom unidirectional communication protocol built with UART, designed for absolute simplicity and ease of use.

## Commands
At its core, the SDMD firmware is about interpreting and executing commands. Some commands control the motors, while others are configuration commands. There are 3 types of commands.
-	Raw commands: Raw numbers between 0 and 200. These are used to inform interpreted commands.
-	Basic commands: Commands which execute an action within the device. An example would be to stop motor 1. In code, there are basic output commands and basic configuration commands. The configuration commands only change internal variables that tell the SDMD what to do, while the output commands write directly to the motors. Writing a basic command might look like: 

```Serial.write(0xCD); //coast motor 1```
-	Informed commands: Commands which put the device into a special state. In this state, they interpret the next raw command sent in order to execute an informed action. An example would be to write to motor 2. In code, there are interpreted commands for output and configuration. Configuration informed commands only change variables, while output-based informed commands directly manipulate motors. Writing an informed command might look like: 
```
Serial.write(0xC9); //write to motor 1
Serial.write(150);  //raw value to write to motor 1
```

## Features
-	Auto-braking: Automatically brakes individual outputs after they are inactive for 100ms. The 100ms timeout is not current editable by sending commands, and the firmware must be modified.
-	Curving: Curve the throttle to increase precision at low throttle values. 

## Basic Commands

| Hex code to write over UART | Action                                    | Type          |
| --------------------------- | ----------------------------------------- | ------------- |
| 0xCB                        | Stop motor 1                              | Output        |
| 0xCC                        | Stop motor 2                              | Output        |
| 0xCD                        | Coast motor 1                             | Output        |
| 0xCE                        | Coast motor 2                             | Output        |
| 0xD0                        | Disable curving                           | Configuration |
| 0xD1                        | Enable auto-braking (on by default)       | Configuration |
| 0xD2                        | Disable auto-braking                      | Configuration |
| 0xD3                        | Set the driver to brake on failsafe       | Configuration |
| 0xD4                        | Set the driver to coast on failsafe       | Configuration |
| 0xFF                        | Stop outputs based on failsafe condition. | Output        |

## Informed Commands

| Hex code | Action             | Type          | Typical values                                                     |
| -------- | ------------------ | ------------- | ------------------------------------------------------------------ |
| 0xC9     | Write to motor 1   | Output        | 0 – max speed backwards. 100 – inactive. 200 – max speed forwards. |
| 0xCA     | Write to motor 2   | Output        | 0 – max speed backwards. 100 – inactive. 200 – max speed forwards. |
| 0xCF     | Set throttle curve | Configuration | 0-3 (0 is off/linear)                                              |

## Examples From a Controller
Write values to both motors 
```
Serial.write(0xC9);              //write to motor 1
Serial.write(motor_1_value);     //write motor_1_value to motor 1
Serial.write(0xCA);              //write to motor 2
Serial.write(motor_2_value);     //write motor_2_value to motor 2
```

Write forwards values to motor 1
```
if(motor_1_input) > 100){         //only activate forwards
	Serial.write(0xC9);
	Serial.write(motor_1_input);
}
else{
	Serial.write(0xCB);         //brake if not forwards
}
```

Setup example
```
void setup(){
	Serial.write(0xD1); //Enable auto-braking
	Serial.write(0xCF); //Set curve
	Serial.write(2);    //Set curve to 2
	Serial.write(0xD3); //Brake on failsafe
}
//rest of program
```

## Other Behavior Notes
The driver will preserve the last command written to each motor even as commands are written to the other motor. If you tell motor 1 to spin forwards once during void setup(), it will do that forever even as commands are written to motor 2 unless a new command is written to motor 1. Motors will never update their behavior unless another command is issued.

The SDMD is always stopped until it gets a serial signal, when it will activate. Ending the serial connection to the SDMD while it is powered on will NOT affect the motors, they will continue to operate based on the last received command. To stop operation of the motors, either maintain a serial connection and send a stop command, or power off the device. 

SDMD’s short-circuit protection shutdown is not configurable or recoverable in firmware, the device must be power cycled. 

Sending an informed command with no raw command after it will not result in any problems due to internal checking inside the firmware.
