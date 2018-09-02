/*
 Sparkfun_OpenLCD Library - Basic I2C

 This is example code that shows how to send data over I2C to the display.
 
 OpenLCD is an LCD with Serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Note: This code expects the display to be listening at the default I2C address. If your display is not at 0x72, you can
 do a hardware reset. Tie the RX pin to ground and power up OpenLCD. You should see the splash screen
 then "System reset Power cycle me" and the backlight will begin to blink. Now power down OpenLCD and remove
 the RX/GND jumper. OpenLCD is now reset.
 
 To get this code to work, attached an OpenLCD to an Arduino Uno using the following pins:
 SCL (OpenLCD) to A5 (Arduino)
 SDA to A4
 VIN to 5V (3.3v to 9v)
 GND to GND

 You can also use a Sparkfun RGB OpenLCD Serial display connected through 
 a Sparkfun Qwiic adpater to an Ardruino with a  Qwiic shield or a Sparkfun Blackboard
 with Qwiic built in.

 The Qwiic adapter should be attached to the display as follows:
 Display Qwiic Qwiic Cable Color
 GND     GND   Black
 RAW     3.3V  Red
 SDA     SDA   Blue
 SCL     SCL   Yellow

 Note: If you connect directly to a 5V Arduino instead, you *MUST* use
    a level-shifter to convert the i2c voltage levels down to 3.3V for the display.
    
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
#include <Wire.h>

#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD

int cycles = 0;

// initialize the library
Sparkfun_OpenLCD lcd;

void setup()
{
  Wire.begin(); //Join the bus as master

  //By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
  Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz

  Serial.begin(9600); //Start serial communication at 9600 for debug statements
  Serial.println("OpenLCD Basic I2C Example Code");

   // set up the LCD for I2C with specified address
  lcd.begin(Wire,DISPLAY_ADDRESS1);
  //lcd.begin(Wire); //This way uses the default address 0x72
}

void loop()
{
  cycles++; //Counting cycles! Yay!
  //clear display- this forces the cursor to return to the beginning of the display
  lcd.clear();
  lcd.print("Cycle: "); //These serial.print statements take multiple miliseconds
  lcd.print(cycles);

  
  delay(250); //The maximum update rate of OpenLCD is about 100Hz (10ms). A smaller delay will cause flicker
}


