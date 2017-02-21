/*
 OpenLCD is an LCD with serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example shows how to display a counter on the display over serial. We use software serial because if 
 OpenLCD is attached to an Arduino's hardware serial port during bootloading  it can cause problems for both devices.
 
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
  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenLCD Example Code");
  
  OpenLCD.begin(9600); //Start communication with OpenLCD
}

void loop()
{
  //Send the clear command to the display - this returns the cursor to the beginning of the display
  OpenLCD.write('|'); //Setting character
  OpenLCD.write('-'); //Clear display

  OpenLCD.print("Hello World!    Counter: "); //For 16x2 LCD
  //OpenLCD.print("Hello World!        Counter: "); //For 20x4 LCD
  OpenLCD.print(counter++);
  
  delay(250); //Hang out for a bit
}

