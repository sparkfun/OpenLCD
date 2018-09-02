/*
 Sparkfun_OpenLCD Library - Basic Serial
 
 This is example code that shows how to send data over Serial interface to the display
 using the Sparkfun_OpenLCD library.
 
 OpenLCD is an LCD with Serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 To get this code to work, attached an OpenLCD to an Arduino Uno using the following pins:
 RX (OpenLCD) to 7 (Arduino)
 TX to 6 (optional)
 VIN to 5V (3.3v to 9v)
 GND to GND
 
 Note: Use a 3.3v Arudino, like the Pro-Mini, or if you use a 5V Arduino instead, 
   you *MUST* use a level-shifter to convert the voltage levels down to 3.3V for the display.

 AVR-Based Serial Enabled LCDs Hookup Guide
 https://learn.sparkfun.com/tutorials/avr-based-serial-enabled-lcds-hookup-guide  

 The Sparkfun_OpenLCD library is based on the LiquidCrystal code originally by David A. Mellis
 and the OpenLCD code by Nathan Seidle at Sparkfun.
 
 Sparkfun_OpenLCD library
 Created 29 Aug 2018
 by Gaston Williams

 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license)
*/
#include <Sparkfun_OpenLCD.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(6, 7); //RX, TX

byte counter = 0;

// initialize the library
Sparkfun_OpenLCD lcd;

void setup()
{
  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenLCD Basic Serial Example Code");
  
  //Set up Software Serial
  mySerial.begin(9600); 
  //Start communication with OpenLCD
  lcd.begin(mySerial);
}

void loop()
{
  //Send the clear command to the display - this returns the cursor to the beginning of the display
  lcd.clear();

  lcd.print("Hello World!");
  lcd.setCursor(0, 1);
  lcd.print("Counter: ");

  lcd.print(counter++);

  delay(250); //Hang out for a bit
}
