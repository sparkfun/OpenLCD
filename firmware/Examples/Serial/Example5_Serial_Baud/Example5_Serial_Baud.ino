/*
 OpenLCD is an LCD with serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original
 serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
 This is an updated serial LCD.
 
 This example shows how to change the baud rate. We assume the module is currently at default 9600bps.

 We use software serial because if OpenLCD is attached to an Arduino's hardware serial port during bootloading 
 it can cause problems for both devices.

 Note: If OpenLCD gets into an unknown state or you otherwise can't communicate with it send 18 (0x12 or ctrl+r) 
 at 9600 baud while the splash screen is active and the unit will reset to 9600 baud.

 Emergency reset: If you get OpenLCD stuck into an unknown baud rate, unknown I2C address, etc, there is a 
 safety mechanism built-in. Tie the RX pin to ground and power up OpenLCD. You should see the splash screen 
 then "System reset Power cycle me" and the backlight will begin to blink. Now power down OpenLCD and remove 
 the RX/GND jumper. OpenLCD is now reset to 9600bps with a I2C address of 0x72. Note: This feature can be 
 disabled if necessary. See *Ignore Emergency Reset* for more information.

 To get this code to work, attached an OpenLCD to an Arduino Uno using the following pins:
 RX (OpenLCD) to Pin 7 (Arduino)
 VIN to 5V
 GND to GND
 
Command cheat sheet:
 ASCII / DEC / HEX
 '|'    / 124 / 0x7C - Put into setting mode
 Ctrl+c / 3 / 0x03 - Change width to 20
 Ctrl+d / 4 / 0x04 - Change width to 16
 Ctrl+e / 5 / 0x05 - Change lines to 4
 Ctrl+f / 6 / 0x06 - Change lines to 2
 Ctrl+g / 7 / 0x07 - Change lines to 1
 Ctrl+h / 8 / 0x08 - Software reset of the system
 Ctrl+i / 9 / 0x09 - Enable/disable splash screen
 Ctrl+j / 10 / 0x0A - Save currently displayed text as splash
 Ctrl+k / 11 / 0x0B - Change baud to 2400bps
 Ctrl+l / 12 / 0x0C - Change baud to 4800bps
 Ctrl+m / 13 / 0x0D - Change baud to 9600bps
 Ctrl+n / 14 / 0x0E - Change baud to 14400bps
 Ctrl+o / 15 / 0x0F - Change baud to 19200bps
 Ctrl+p / 16 / 0x10 - Change baud to 38400bps
 Ctrl+q / 17 / 0x11 - Change baud to 57600bps
 Ctrl+r / 18 / 0x12 - Change baud to 115200bps
 Ctrl+s / 19 / 0x13 - Change baud to 230400bps
 Ctrl+t / 20 / 0x14 - Change baud to 460800bps
 Ctrl+u / 21 / 0x15 - Change baud to 921600bps
 Ctrl+v / 22 / 0x16 - Change baud to 1000000bps
 Ctrl+w / 23 / 0x17 - Change baud to 1200bps
 Ctrl+x / 24 / 0x18 - Change the contrast. Follow Ctrl+x with number 0 to 255. 120 is default.
 Ctrl+y / 25 / 0x19 - Change the TWI address. Follow Ctrl+x with number 0 to 255. 114 (0x72) is default.
 Ctrl+z / 26 / 0x1A - Enable/disable ignore RX pin on startup (ignore emergency reset)
 '-'    / 45 / 0x2D - Clear display. Move cursor to home position.
        / 128-157 / 0x80-0x9D - Set the primary backlight brightness. 128 = Off, 157 = 100%.
        / 158-187 / 0x9E-0xBB - Set the green backlight brightness. 158 = Off, 187 = 100%.
        / 188-217 / 0xBC-0xD9 - Set the blue backlight brightness. 188 = Off, 217 = 100%.
         For example, to change the baud rate to 115200 send 124 followed by 18.
 '+'    / 43 / 0x2B - Set Backlight to RGB value, follow + by 3 numbers 0 to 255, for the r, g and b values.
         For example, to change the backlight to yellow send + followed by 255, 255 and 0.

*/

#include <SoftwareSerial.h>

SoftwareSerial OpenLCD(6, 7); //RX (not used), TX

byte counter = 0;

void setup()
{
  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenLCD Example Code");
  
  OpenLCD.begin(9600);

  //Change baud rate
  OpenLCD.write('|'); //Setting character
  //OpenLCD.write(17); //Change baud to 57600
  OpenLCD.write(18); //Change baud to 115200
  
  delay(2000); //Wait for 'Baud now: 115200' to clear

  //OpenLCD.begin(57600); //Start serial communication at this new speed
  OpenLCD.begin(115200); //Start serial communication at this new speed
}

void loop()
{
  //Send the clear command to the display - this returns the cursor to the beginning of the display
  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display

  OpenLCD.print("Hello World!    Counter: "); //For 16x2 LCDs
  //OpenLCD.print("Hello World!        Counter: "); //For 20x4 LCDs
  OpenLCD.print(counter++);
  
  //25ms works well
  //15ms is ok
  //5ms causes flicker
  delay(25); //Hang out for a bit
}

