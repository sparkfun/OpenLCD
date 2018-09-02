/*
  Sparkfun_OpenLCD Library - Backlight

 Demonstrates the use of a Sparkfun AVR-Based Serial Enabled LCD
 display with a Qwiic adapter.
 
 This sketch changes the backlight color and displays text using
 the OpenLCD functions.

 The circuit:
 * Sparkfun RGB OpenLCD Serial display connected through 
 * a Sparkfun Qwiic adpater to an Ardruino with a 
 * Qwiic shield or a Sparkfun Blackboard with Qwiic built in.
 * 
 * The Qwiic adapter should be attached to the display as follows:
 *  
 *  Display Qwiic Qwiic Cable Color
 *  GND GND Black
 *  RAW 3.3V  Red
 *  SDA SDA Blue
 *  SCL SCL Yellow
 *
 * Note: If you connect directly to a 5V Arduino instead, you *MUST* use
 * a level-shifter to convert the i2c voltage levels down to 3.3V for the display.


 Based on the LiquidCrystal code originally by David A. Mellis
 and the OpenLCD code by Nathan Seidle at Sparkfun.
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example created 23 Aug 2018
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
}

void loop() {
  //change the backlight color to various colors
  lcd.setBacklight(0, 0, 0); //black is off
  lcd.clear();
  lcd.print("Black (off)");
  delay(3000);
  lcd.setBacklight(255, 0, 0); //bright red
  lcd.clear();
  lcd.print("Red");
  delay(3000);
  lcd.setBacklight(0xFF8C00); //orange
  lcd.clear();
  lcd.print("Orange");
  delay(3000);
  lcd.setBacklight(255, 255, 0); //bright yellow
  lcd.clear();
  lcd.print("Yellow");
  delay(3000);
  lcd.setBacklight(0, 255, 0); //bright green
  lcd.clear();
  lcd.print("Green");
  delay(3000);
  lcd.setBacklight(0, 0, 255); //bright blue
  lcd.clear();
  lcd.print("Blue");
  delay(3000);
  lcd.setBacklight(0x4B0082); //indigo, a kind of dark purplish blue
  lcd.clear();
  lcd.print("Indigo");
  delay(3000);
  lcd.setBacklight(0xA020F0); //violet
  lcd.clear();
  lcd.print("Violet");
  delay(3000);
  lcd.setBacklight(0x808080); //grey
  lcd.clear();
  lcd.print("Grey");
  delay(3000);
  lcd.setBacklight(255, 255, 255); //bright white
  lcd.clear();
  lcd.print("White");
  delay(3000);  
}
