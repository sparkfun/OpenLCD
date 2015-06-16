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
 SDO to 12
 SCK to 13
 VIN to 5V
 GND to GND

*/

#include <SPI.h>

int csPin = 10; //You can use any output pin but for this example we use 10

int cycles = 0;

void setup() 
{
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH); //By default, don't be selecting OpenLCD
  
  SPI.begin(); //Start SPI communication
  SPI.setClockDivider(SPI_CLOCK_DIV64); //Slow down the master a bit
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
  delay(25);
}

//Sends a string over SPI
void spiSendString(char* data)
{
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenLCD
  for(byte x = 0 ; data[x] != '\0' ; x++) //Send chars until we hit the end of the string
    SPI.transfer(data[x]);
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenLCD
}
