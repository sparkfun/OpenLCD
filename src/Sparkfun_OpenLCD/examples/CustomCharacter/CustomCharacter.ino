/*
  Sparkfun_OpenLCD Library - Custom Characters

 Demonstrates the use of a Sparkfun AVR-Based Serial Enabled LCD
 display with a Qwiic adapter.
 
 This sketch prints "I <heart> Qwiic!" and a little dancing man
 to the LCD. Since the Serial OpenLCD display uses a serial command
 to display a customer character, the writeChar() function was added 
 for this function.

 The circuit:
 * Sparkfun RGB OpenLCD Serial display connected through 
 * a Sparkfun Qwiic adpater to an Ardruino with a 
 * Qwiic shield or a Sparkfun Blackboard with Qwiic built in.
 * 
 * 10K poterntiometer on pin A0 of arduino or blackboard
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
 example created 21 Mar 2011
 by Tom Igoe
 modified 11 Nov 2013
 by Scott Fitzgerald
 modified 7 Nov 2016
 by Arturo Guadalupi
 modified 29 Aug 2018
 by Gaston Williams


 Based on Adafruit's example at
 https://github.com/adafruit/SPI_VFD/blob/master/examples/createChar/createChar.pde

 This example code is in the public domain.
 http://www.arduino.cc/en/Tutorial/LiquidCrystalCustomCharacter

 Also useful:
 http://icontexto.com/charactercreator/

 More info on Qwiic here:
 https://www.sparkfun.com/qwiic
 
 AVR-Based Serial Enabled LCDs Hookup Guide
 https://learn.sparkfun.com/tutorials/avr-based-serial-enabled-lcds-hookup-guide

*/

// include the Sparkfun OpenLCD library code:
#include <Sparkfun_OpenLCD.h>

// initialize the library with default i2c address 0x72
Sparkfun_OpenLCD lcd;

// make some custom characters:
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};

byte frownie[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b10001
};

byte armsDown[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b01010
};

byte armsUp[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b01010
};

void setup() {
  //Setup Wire
  Wire.begin();
   //By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
  Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz
  
  // set up the LCD for I2C
  lcd.begin(Wire);

  // create a new character
  lcd.createChar(0, heart);
  // create a new character
  lcd.createChar(1, smiley);
  // create a new character
  lcd.createChar(2, frownie);
  // create a new character
  lcd.createChar(3, armsDown);
  // create a new character
  lcd.createChar(4, armsUp);

  // set the cursor to the top left
  lcd.setCursor(0, 0);

  // Print a message to the lcd.
  lcd.print("I ");
  lcd.writeChar(0); // have to use writeChar since it's a serial display
  lcd.print(" Qwiic! ");
  lcd.writeChar(1); // have to use writeChar since it's a serial display

}

void loop() {
  // read the potentiometer on A0:
  int sensorReading = analogRead(A0);
  // map the result to 200 - 1000:
  int delayTime = map(sensorReading, 0, 1023, 200, 1000);
  // set the cursor to the bottom row, 5th position:
  lcd.setCursor(4, 1);
  // draw the little man, arms down:
  lcd.writeChar(3); // have to use writeChar since it's a serial display
  delay(delayTime);
  lcd.setCursor(4, 1);
  // draw him arms up:
  lcd.writeChar(4); // have to use writeChar since it's a serial display
  delay(delayTime);
}
