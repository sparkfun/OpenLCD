/*
 OpenLCD is an LCD with serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example pushes the display as fast as it can go over serial
 
 To get this code to work, attached an OpenLCD to an Arduino Uno using the following pins:
 RX (OpenLCD) to Pin 7 (Arduino)
 VIN to 5V
 GND to GND
 
*/

#include <SoftwareSerial.h>

SoftwareSerial OpenLCD(6, 7); //RX, TX

byte counter = 0;

void setup()
{
  Serial.begin(115200); //Start serial communication at 9600 for debug statements
  Serial.println("OpenLCD Example Code");

  OpenLCD.begin(9600);
  
  //Change baud rate
  OpenLCD.write('|'); //Setting character
  //OpenLCD.write(17); //Change baud to 57600
  OpenLCD.write(18); //Change baud to 115200
  
  delay(2000); //Wait for 'Baud now: 115200' to clear

  //OpenLCD.begin(57600); //Start serial communication at this new speed
  OpenLCD.begin(115200); //Start serial communication at this new speed

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(24); //Send contrast command
  OpenLCD.write(1); //Set contrast

  OpenLCD.write('|'); //Put LCD into setting mode
  //OpenLCD.write(128 + 29); //Set white/red backlight amount to 100%
  OpenLCD.write(128 + 1); //Set white/red backlight amount to 100%

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(158 + 29); //Set green backlight amount to 100%

  OpenLCD.write('|'); //Put LCD into setting mode
  OpenLCD.write(188 + 29); //Set blue backlight amount to 100%
}

void loop()
{
  //Send the clear command to the display - this returns the cursor to the beginning of the display
  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display

  OpenLCD.print("Hello World!    Counter: "); //For 16x2 LCD
  //OpenLCD.print("Hello World!        Counter: "); //For 20x4 LCD
  OpenLCD.print(counter++);
  
  //25ms works well
  //15ms is ok
  //5ms causes flicker
  delay(25); //Hang out for a bit
}

