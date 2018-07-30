/*
 OpenLCD is an LCD with Serial/I2C/SPI interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: April 19th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This is example code that shows how to send data over SPI to the display.
 
 To get this code to work, attached an OpenLCD to an Arduino Uno using the following pins:
 CS (OpenLCD) to 10 (Arduino)
 SDI to 11
 SDO to 12 (optional)
 SCK to 13
 VIN to 5V
 GND to GND

 Command cheat sheet:
 ASCII / DEC / HEX
 '|'    / 124 / 0x7C - Put into setting mode
 Ctrl+c / 3 / 0x03 - Change width to 20
 Ctrl+d / 4 / 0x04 - Change width to 16
 Ctrl+e / 5 / 0x05 - Change lines to 4
 Ctrl+f / 6 / 0x06 - Change lines to 2
 Ctrl+g / 7 / 0x07 - Change lines to 1
 Ctrl+h / 8 / 0x08 - Software reset of the system
 Ctrl+i / 9 / 0x09 - Enable/disable splash screen
 Ctrl+j / 10 / 0x0A - Save currently displayed text as splash
 Ctrl+k / 11 / 0x0B - Change baud to 2400bps
 Ctrl+l / 12 / 0x0C - Change baud to 4800bps
 Ctrl+m / 13 / 0x0D - Change baud to 9600bps
 Ctrl+n / 14 / 0x0E - Change baud to 14400bps
 Ctrl+o / 15 / 0x0F - Change baud to 19200bps
 Ctrl+p / 16 / 0x10 - Change baud to 38400bps
 Ctrl+q / 17 / 0x11 - Change baud to 57600bps
 Ctrl+r / 18 / 0x12 - Change baud to 115200bps
 Ctrl+s / 19 / 0x13 - Change baud to 230400bps
 Ctrl+t / 20 / 0x14 - Change baud to 460800bps
 Ctrl+u / 21 / 0x15 - Change baud to 921600bps
 Ctrl+v / 22 / 0x16 - Change baud to 1000000bps
 Ctrl+w / 23 / 0x17 - Change baud to 1200bps
 Ctrl+x / 24 / 0x18 - Change the contrast. Follow Ctrl+x with number 0 to 255. 120 is default.
 Ctrl+y / 25 / 0x19 - Change the TWI address. Follow Ctrl+x with number 0 to 255. 114 (0x72) is default.
 Ctrl+z / 26 / 0x1A - Enable/disable ignore RX pin on startup (ignore emergency reset)
 '-'    / 45 / 0x2D - Clear display. Move cursor to home position.
        / 128-157 / 0x80-0x9D - Set the primary backlight brightness. 128 = Off, 157 = 100%.
        / 158-187 / 0x9E-0xBB - Set the green backlight brightness. 158 = Off, 187 = 100%.
        / 188-217 / 0xBC-0xD9 - Set the blue backlight brightness. 188 = Off, 217 = 100%.
 
 For example, to change the baud rate to 115200 send 124 followed by 18.
*/

#include <SPI.h>

int csPin = 10; //You can use any output pin but for this example we use 10

int cycles = 0;

void setup() 
{
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH); //By default, don't be selecting OpenLCD
  
  SPI.begin(); //Start SPI communication
  //SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  SPI.setClockDivider(SPI_CLOCK_DIV128); //Slow down the master a bit
}

void loop() 
{
  cycles++; //Counting cycles! Yay!

  //Send the clear display command to the display - this forces the cursor to return to the beginning of the display
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenLCD
  SPI.transfer('|'); //Put LCD into setting mode
  SPI.transfer('-'); //Send clear display command
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenLCD

  char tempString[50]; //Needs to be large enough to hold the entire string with up to 5 digits
  sprintf(tempString, "Cycles: %d", cycles);
  spiSendString(tempString);
  
  //25ms works well
  //15ms slight flickering
  //5ms causes flickering
  delay(250);
}

//Sends a string over SPI
void spiSendString(char* data)
{
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenLCD
  for(byte x = 0 ; data[x] != '\0' ; x++) //Send chars until we hit the end of the string
    SPI.transfer(data[x]);
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenLCD
}
