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

#include <Wire.h> //For I2C functions
#include <LiquidCrystalFast.h>
#include <EEPROM.h>  //Brightness, Baud rate, and I2C address are stored in EEPROM
#include "settings.h" //Defines EEPROM locations for user settings

LiquidCrystalFast SerLCD(LCD_RS, LCD_RW, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Struct for circular data buffer data received over UART, SPI and I2C are all sent into a single buffer
struct dataBuffer
{
  unsigned char data[BUFFER_SIZE];  // THE data buffer
  unsigned int head;  // store new data at this index
  unsigned int tail;  // read oldest data from this index
}
buffer;  // our data buffer is creatively named - buffer

void setup()
{
  setupBacklight(); //Turn on any backlights
  setupDisplay(); //Initialize the LCD
  setupSplash(); //Read and display the user's splash screen

  setupUART(); //Setup serial
  setupSPI(); //Initialize SPI stuff (enable, mode, interrupts)
  setupTWI(); //Initialize I2C stuff (address, interrupt, enable)
  setupTimer(); //Setup timer to control interval reading from buffer
  
//  interrupts();  // Turn interrupts on, and les' go
}

void loop()
{
  byte incoming = buffer.data[buffer.tail++];
  //Process characters as they come in
  if(incoming == 124)
  {
    //We've got a special command
    byte setting = buffer.data[buffer.tail++];
    
    //LCD width and lines
    if(setting >= 3 && setting <= 6)
    {
      switch(setting)
      {
        case 3:
          settingLCDwidth = 20;
          break;
        case 4:
          settingLCDwidth = 16;
          break;
        case 5:
          settingLCDlines = 4;
          break;
        case 6:
          settingLCDlines = 2;
          break;
      }

      //Record this new setting
      EEPROM.write(LOCATION_WIDTH, settingLCDwidth);
      EEPROM.write(LOCATION_LINES, settingLCDlines);
    }
    
    //Enable / disable splash
    else if(setting == 9)
    {
      if(settingSplashEnable == 1)
        settingSplashEnable = 0;
      else
        settingSplashEnable = 1;
        
      //Record this new setting
      EEPROM.write(LOCATION_SPLASH_ONOFF, settingSplashEnable);
    }

    //Set baud rate
    else if(setting <= 11 && setting >= 16)
    {
      switch(setting)
      {
        case 11:
          settingUARTSpeed = BAUD_2400;
          break;
        case 12:
          settingUARTSpeed = BAUD_4800;
          break;
        case 13:
          settingUARTSpeed = BAUD_9600;
          break;
        case 14:
          settingUARTSpeed = BAUD_14400;
          break;
        case 15:
          settingUARTSpeed = BAUD_19200;
          break;
        case 16:
          settingUARTSpeed = BAUD_38400;
          break;
          
          //Add baud rates here
      }
      
      //Record this new buad rate
      EEPROM.write(LOCATION_BAUD, settingUARTSpeed);
      
//TODO Display on LCD that we are at this new speed
      
      //Go to this new baud rate
      //Serial.begin(new speed);
    }


    //Backlight
    else if(setting >= 128 && setting <= 157)
    {
      //Covert 30 digit value to 255 digits
      settingBLred = map(setting, 128, 157, 0, 255);

      //Record new setting
      EEPROM.write(LOCATION_RED_BRIGHTNESS, settingBLred);

      //Goto that setting
      analogWrite(BL_RW, settingBLred);

    }
    
  }
  
  //LCD commands
  else if(incoming == 254)
  {
    //We've got a special command
    byte command = buffer.data[buffer.tail++];
    
  }
  
  //Just put the characters on the display
  else
  {
    
  }

  //Just hang out and update the display as new data comes in
//  SerLCD.write(
 // myDisplay.DisplayString(display.digits, display.decimals); //(numberToDisplay, decimal point location)

  //serialEvent(); //Check the serial buffer for new data


  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  SerLCD.setCursor(0, 1);
  // print the number of seconds since reset:
  SerLCD.print(millis());

  delay(1);
}


