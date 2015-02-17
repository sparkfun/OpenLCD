/*
 OpenLCD is an LCD with serial/i2c/spi interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: February 13th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original 
 serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set. 
 This is an updated serial LCD.

 This firmware relies on LiquidCrystalFast that controls the R/W pin (built-in libraries do not).
 https://www.pjrc.com/teensy/td_libs_LiquidCrystal.html 
 Thanks goes to Paul Stoffregen for this library.

*/

#include <LiquidCrystalFast.h>

LiquidCrystalFast lcd(A0, A1, A2, A3, 2, 3, 4);
         // LCD pins: RS  RW  EN  D4 D5 D6 D7

void setup()
{
  
  lcd.begin(16, 2); // set up the LCD's number of rows and columns: 
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis());
  
  delay(1);
}

