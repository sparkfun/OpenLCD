/*
 OpenLCD is an LCD with serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original
 serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
 This is an updated serial LCD.
 
 This example shows how to change the backlight brightness. We assume the module is currently at default 9600bps.

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

*/

#include <SoftwareSerial.h>

SoftwareSerial OpenLCD(6, 7); //RX (not used), TX

byte counter = 0;

void setup()
{
  Serial.begin(9600); //Begin local communication for debug statements
  
  OpenLCD.begin(9600); //Begin communication with OpenLCD

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(158 + 0); //Set green backlight amount to 0%

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(188 + 0); //Set blue backlight amount to 0%
}

void loop()
{
  //Control red backlight
  Serial.println("Mono/Red backlight set to 0%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(128); //Set white/red backlight amount to 0%
  
  delay(2000);

  //Control red backlight
  Serial.println("Mono/Red backlight set to 51%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(128 + 15); //Set white/red backlight amount to 51%
  
  delay(2000);

  //Control red backlight
  Serial.println("Mono/Red backlight set to 100%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(128 + 29); //Set white/red backlight amount to 100%
  
  delay(2000);
  
  //The following green and blue backlight control only apply if you have an RGB backlight enabled LCD

  all_off(); // turn off all backlights - see function below

  //Control green backlight
  Serial.println("Green backlight set to 51%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(158 + 15); //Set green backlight amount to 51%
  
  delay(2000);

  //Control green backlight
  Serial.println("Green backlight set to 100%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(158 + 29); //Set green backlight amount to 100%
  
  delay(2000);

  all_off(); // turn off all backlights - see function below

  //Control blue backlight
  Serial.println("Blue backlight set to 51%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(188 + 15); //Set blue backlight amount to 51%
  
  delay(2000);

  //Control blue backlight
  Serial.println("Blue backlight set to 100%");
  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(188 + 29); //Set blue backlight amount to 100%
  
  delay(2000);

  all_off(); // turn off all backlights - see function below  

}

void all_off(void)
{
  // Set all colors to 0

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(128); //Set white/red backlight amount to 0%    

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(158 + 0); //Set green backlight amount to 0%

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(188 + 0); //Set blue backlight amount to 0%
  
  delay(2000);  
}

