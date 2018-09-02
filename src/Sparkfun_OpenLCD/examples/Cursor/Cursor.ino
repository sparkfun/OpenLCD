/*
   Sparkfun_OpenLCD Library - Cursor

 Demonstrates the use of a Sparkfun AVR-Based Serial Enabled LCD
 display with a Qwiic adapter.

 This sketch prints "Hello World!" to the LCD and
 uses the cursor()  and noCursor() methods to turn
 on and off the cursor.

 The circuit:
 * Sparkfun RGB OpenLCD Serial display connected through 
 * a Sparkfun Qwiic adpater to an Ardruino with a 
 * Qwiic shield or a Sparkfun Blackboard with Qwiic built in.
 * 
 * The Qwiic adapter should be attached to the display as follows:
 *	
 * 	Display	Qwiic	Qwiic Cable Color
 *	GND	GND	Black
 *	RAW	3.3V	Red
 *	SDA	SDA	Blue
 *	SCL	SCL	Yellow
 *
 * Note: If you connect directly to a 5V Arduino instead, you *MUST* use
 * a level-shifter to convert the i2c voltage levels down to 3.3V for the display.

 Based on the LiquidCrystal code originally by David A. Mellis
 and the OpenLCD code by Nathan Seidle at Sparkfun.
 
 Library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi
 modified 29 Aug 2018
 by Gaston Williams

 This example code is in the public domain.
 http://www.arduino.cc/en/Tutorial/LiquidCrystalCursor
 
 More info on Qwiic here:
 https://www.sparkfun.com/qwiic
 
 AVR-Based Serial Enabled LCDs Hookup Guide
 https://learn.sparkfun.com/tutorials/avr-based-serial-enabled-lcds-hookup-guide 

*/

// include the Sparkfun OpenLCD library code:
#include <Sparkfun_OpenLCD.h>

// initialize the library with default i2c address 0x72
Sparkfun_OpenLCD lcd;

void setup() {
  //Setup Wire
  Wire.begin();
   //By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
  Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz
  
  // set up the LCD for I2C
  lcd.begin(Wire);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // Turn off the cursor:
  lcd.noCursor();
  delay(500);
  // Turn on the cursor:
  lcd.cursor();
  delay(500);
}

