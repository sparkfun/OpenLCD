/*
  Sparkfun_OpenLCD Library - Move Cursor

 Demonstrates the use of a Sparkfun AVR-Based Serial Enabled LCD
 display with a Qwiic adapter.
 
 This sketch displays text and then moves the cursor back and forth.  These
 functions are not usually part of the LiquidCrystal library, but the functions
 are available in the Serial OpenLCD display, so I created functions for it.

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
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example created 22 Aug 2018
 by Gaston Williams
 modified 29 Aug 2018
 by Gaston Williams

 This example code is in the public domain.

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
  
  // clear the screen
  lcd.clear();
  lcd.cursor();
  lcd.print("Watch the cursor!");  
}

void loop() {
  //move cursor left in 3 steps
  lcd.moveCursorLeft();
  delay(500);
  lcd.moveCursorLeft();
  delay(500);
  lcd.moveCursorLeft();
  delay(500);
  
  //move back in one step
  lcd.moveCursorRight(3);
  delay(500);
  
  //move cursor ahead 3 places in 1 step
  lcd.moveCursorRight(3);
  delay(500);
  
  //move back in 3 steps
  lcd.moveCursorLeft();
  delay(500);
  lcd.moveCursorLeft();
  delay(500);
  lcd.moveCursorLeft();
  delay(500);
} //loop
