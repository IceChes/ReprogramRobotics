# Gen 2 Transmitter
While you can technically build this, I advise against it. It has a lot of problems and design issues that you will need to solve. Nevertheless, I am placing the files and BOM here. If you choose to build this, refer to the Gen 2 transmitter sketch in the Firmware folder for Arduino wiring.

![tx2](https://github.com/user-attachments/assets/d987dac7-0ecd-46a7-af55-bb17de3eb8c5)

# BOM
- 2x PS2 joystick module
- 1x 0.96" I2C 128x32 OLED
- 1x Seeed LiPo Rider Plus
- 1x 18650 cell
- 1x Keystone 18650 battery holder
- 1x RF-nano
- 1x Green LED
- 1x Yellow LED
- 2x 220 ohm resistor
- 2x SPST panel mount toggle switch
- 1x 10k potentiometer
- Optional - 1x M5 eyelet for lanyard

# Assembly
This is not a step by step guide, these are merely guidelines. Proceed at your own risk. 

- Wire 18650 holder directly to LiPo rider pins.
- Glue LiPo rider in place with the USB-C port aligned with the hole in the back of the transmitter.
- LEDs and display must be glued in place.
- Wires from display will be threaded through the hole in the display mount.
- All 5V wires and GND wires should be condensed into individual power rails.
- One of the switches will go between the 18650 GND and LiPo Rider to act as a power switch.
