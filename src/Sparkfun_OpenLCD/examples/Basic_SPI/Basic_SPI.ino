/*
 Sparkfun_OpenLCD Library - Basic SPI

 This is example code that shows how to send data over SPI to the display
 using the Sparkfun_OpenLCD library.
 
 OpenLCD is an LCD with Serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 To get this code to work, attached an OpenLCD to an Arduino Uno using the following pins:
 CS (OpenLCD) to 10 (Arduino)
 SDI to 11
 SDO to 12 (optional)
 SCK to 13
 VIN to 5V (3.3v to 9v)
 GND to GND
 
 Note: Use a 3.3v Arudino, like the Pro-Mini, or if you use a 5V Arduino instead, 
   you *MUST* use a level-shifter to convert the SPI voltage levels down to 3.3V for the display.

 AVR-Based Serial Enabled LCDs Hookup Guide
 https://learn.sparkfun.com/tutorials/avr-based-serial-enabled-lcds-hookup-guide  

 The Sparkfun_OpenLCD library is based on the LiquidCrystal code originally by David A. Mellis
 and the OpenLCD code by Nathan Seidle at Sparkfun.
 
 Sparkfun_OpenLCD library
 Created 29 Aug 2018
 by Gaston Williams

 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license)
*/
// include the Qwiic OpenLCD library code:
#include <Sparkfun_OpenLCD.h>
#include <SPI.h>

int csPin = 10; //You can use any output pin but for this example we use 10

int cycles = 0;

// initialize the library
Sparkfun_OpenLCD lcd;
void setup() 
{
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH); //By default, don't be selecting OpenLCD

  SPI.begin(); //Start SPI communication

  lcd.begin(SPI, csPin, SPISettings(100000, MSBFIRST, SPI_MODE0));
//For Arduino versions before 1.6, use the two lines below instead
//  SPI.setClockDivider(SPI_CLOCK_DIV128); //Slow down the master a bit
//  lcd.begin(SPI, csPin);  
}

void loop() 
{
  cycles++; //Counting cycles! Yay!

  lcd.clear();
  char tempString[50]; //Needs to be large enough to hold the entire string with up to 5 digits
  sprintf(tempString, "Cycles: %d", cycles);

  lcd.print(tempString);

  //250ms works well
  delay(250);
}


