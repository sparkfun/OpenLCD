/*
 OpenLCD is an LCD with Serial/I2C/SPI interfaces.
 By: fourstix
 Date: August 28th, 2018
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This sketch changes the backlight color and displays text using
 the OpenLCD functions.

 OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original
 serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
 This is an updated serial LCD.
 
 Please Note: 0x72 is the 7-bit I2C address. If you are using a different language than Arduino you will probably
 need to add the Read/Write bit to the end of the address. This means the default read address for the OpenLCD
 is 0b.1110.0101 or 0xE5 and the write address is 0b.1110.0100 or 0xE4.
 For more information see https://learn.sparkfun.com/tutorials/i2c

 Note: This code expects the display to be listening at the default I2C address. If your display is not at 0x72, you can
 do a hardware reset. Tie the RX pin to ground and power up OpenLCD. You should see the splash screen 
 then "System reset Power cycle me" and the backlight will begin to blink. Now power down OpenLCD and remove 
 the RX/GND jumper. OpenLCD is now reset.
 
 The circuit:
 SparkFun RGB OpenLCD Serial display connected through 
 a Sparkfun Qwiic adpater to an Ardruino with a 
 Qwiic shield or a SparkFun Blackboard with Qwiic built in.
 
 The Qwiic adapter should be attached to the display as follows:
  
 Display Qwiic Qwiic Cable Color
 GND     GND   Black
 RAW     3.3V  Red
 SDA     SDA   Blue
 SCL     SCL   Yellow
 Note: If you connect directly to a 5V Arduino instead, you *MUST* use
 level-shifter to convert the i2c voltage levels down to 3.3V for the display.
 
 The OpenLCD has 4.7k pull up resistors on SDA and SCL. If you have other devices on the
 I2C bus then you may want to disable the pull up resistors by clearing the PU (pull up) jumper.
 
 OpenLCD will work at 400kHz Fast I2C. Use the .setClock() call shown below to set the data rate 
 faster if needed.

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

#include <Wire.h>

#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD

void setup() 
{
  Wire.begin(); //Join the bus as master

  //By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
  Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz

  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenLCD Example Code");

  //Send the reset command to the display - this forces the cursor to return to the beginning of the display
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command
  Wire.endTransmission();
  Wire.print("Testing Set RGB");
  delay(2000);
}

void loop() 
{
  //Turn off backlight (black)
  Serial.println("Setting RGB backlight to black");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command  
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0x00); //Send the red value
  Wire.write(0x00); //Send the green value
  Wire.write(0x00); //Send the blue value
  Wire.print("Black (Off)!");
  Wire.endTransmission(); //Stop I2C transmission
  
  delay(2000);


  //Set red backlight
  Serial.println("Setting RGB backlight to red");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command  
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0xFF); //Send the red value
  Wire.write(0x00); //Send the green value
  Wire.write(0x00); //Send the blue value
  Wire.print("Red!");   
  Wire.endTransmission(); //Stop I2C transmission
  
  delay(2000);

  //Set Orange backlight
  Serial.println("Setting RGB backlight to orange");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command  
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0xFF); //Send the red value
  Wire.write(0x8C); //Send the green value
  Wire.write(0x00); //Send the blue value
  Wire.print("Orange!");   
  Wire.endTransmission(); //Stop I2C transmission
  
  delay(2000);

  //Set yellow backlight
  Serial.println("Setting RGB backlight to yellow");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command  
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0xFF); //Send the red value
  Wire.write(0xFF); //Send the green value
  Wire.write(0x00); //Send the blue value
  Wire.print("Yellow!");   
  Wire.endTransmission(); //Stop I2C transmission
  
  delay(2000);
  
  //Set green backlight
  Serial.println("Setting RGB backlight to green");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0x00); //Send the red value
  Wire.write(0xFF); //Send the green value
  Wire.write(0x00); //Send the blue value
  Wire.print("Green!");     
  Wire.endTransmission(); //Stop I2C transmission
  delay(2000);
  
  //Set blue backlight
  Serial.println("Setting RGB backlight to blue");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command  
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0x00); //Send the red value
  Wire.write(0x00); //Send the green value
  Wire.write(0xFF); //Send the blue value
  Wire.print("Blue!");   
  Wire.endTransmission(); //Stop I2C transmission
  delay(2000);

  //Set violet backlight
  Serial.println("Setting RGB backlight to violet");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command  
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0xA0); //Send the red value
  Wire.write(0x20); //Send the green value
  Wire.write(0xF0); //Send the blue value
  Wire.print("Violet!");   
  Wire.endTransmission(); //Stop I2C transmission
  delay(2000);

  //Turn on all (white)
  Serial.println("Setting RGB backlight to white");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0xFF); //Send the red value
  Wire.write(0xFF); //Send the green value
  Wire.write(0xFF); //Send the blue value
  Wire.print("White!");   
  Wire.endTransmission(); //Stop I2C transmission
  delay(2000);

  //Set to Gray
  Serial.println("Setting RGB backlight to gray");
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0x80); //Send the red value
  Wire.write(0x80); //Send the green value
  Wire.write(0x80); //Send the blue value
  Wire.print("Gray!");   
  Wire.endTransmission(); //Stop I2C transmission
  delay(2000);
}

