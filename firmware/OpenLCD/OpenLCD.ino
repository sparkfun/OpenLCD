/*
  OpenLCD is an LCD with serial/i2c/spi interfaces.
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 13th, 2015
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original
  serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
  This is an updated serial LCD.

  Select 'SerLCD v2' as the board. We use an ATmega328P running at 11.0592MHz in order to have error free serial comm.

  Backlight levels from original datasheet are wrong. Setting of 22 is 76%. See google doc

  Todo:
  Check how splash screen works on 16 vs 20 width displays
  Establish and cut down on boot time

  Tests:
  -Change LCD width to 20, then back to 16 (124/3, then 124/4) then send 18 characters and check for wrap
  -Enable/Disable splash screen, send 124 then 9 to toggle, then power cycle
  -Change baud rate: 124/12 to go to 4800bps, power cycle, send characters at 4800
*/

#include <Wire.h> //For I2C functions
#include <SPI.h> //For SPI functions
#include <LiquidCrystalFast.h> //Faster LCD commands. From PJRC https://www.pjrc.com/teensy/td_libs_LiquidCrystal.html
#include <EEPROM.h>  //Brightness, Baud rate, and I2C address are stored in EEPROM
#include "settings.h" //Defines EEPROM locations for user settings
#include <avr/wdt.h> //Watchdog to prevent system freeze
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

#include <SoftPWM.h> //Software PWM for Blue backlight: From https://github.com/bhagman/SoftPWM
//SoftPWM uses Timer 2

LiquidCrystalFast SerLCD(LCD_RS, LCD_RW, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

#define OLED 0
#define LCD 1
//#define DISPLAY_TYPE OLED
#define DISPLAY_TYPE LCD

byte characterCount = 0;
char currentFrame[DISPLAY_BUFFER_SIZE]; //Max of 4 x 20 LCD

byte customCharData[8]; //Records incoming custom character data
byte customCharSpot = 0 ; //Keeps track of where we are in custCharData array
byte customCharNumber = 0; //LCDs can store 8 custom chars, this keeps track

//New variables for Set RGB command
byte rgbData[3]; //Records incoming backlight rgb triplet
byte rgbSpot = 0 ; //Keeps track of where we are in rgbData array

enum displayMode
{
  MODE_NORMAL, //No mode, just print
  MODE_COMMAND, //Used to indicate if a command byte has been received
  MODE_SETTING, //Used to indicate if a setting byte has been received
  MODE_CONTRAST, //First setting mode, then contrast change mode, then the value to change to
  MODE_TWI, //First setting mode, then custom char mode, then record 8 bytes
  MODE_RECORD_CUSTOM_CHAR, //First setting mode, then custom char mode, then record 8 bytes
  MODE_SET_RGB //First setting mode, then RGB mode, then get 3 bytes
} currentMode = MODE_NORMAL;

// Struct for circular data buffer
// Data received over UART, SPI and I2C are all sent into a single buffer
struct dataBuffer
{
  byte data[BUFFER_SIZE];  // THE data buffer
  volatile byte head;  // store new data at this index
  volatile byte tail;  // read oldest data from this index
} buffer;  // our data buffer is creatively named - buffer

void setup()
{
  wdt_reset(); //Pet the dog
  wdt_disable(); //We don't want the watchdog during init

  //During testing reset everything
  //for(int x = 0 ; x < 200 ; x++)
  //  EEPROM.write(x, 0xFF);

  if (DISPLAY_TYPE == LCD) setupLCD(); //Initialize the LCD
  else if (DISPLAY_TYPE == OLED) setupOLED(); //Initialize the OLED

  setupContrast(); //Set contrast

  setupBacklight(); //Turn on any backlights

  setupSplash(); //Read and display the user's splash screen

  setupCustomChars(); //Pre-load user's custom chars from EEPROM

  setupUART(); //Setup serial, check for emergency reset after the splash is done

  setupSPI(); //Initialize SPI stuff (enable, mode, interrupts)

  setupTWI(); //Initialize I2C stuff (address, interrupt, enable)

  setupPower(); //Power down peripherals that we won't be using

  interrupts();  // Turn interrupts on, and let's go
  wdt_enable(WDTO_250MS); //Unleash the beast
}

void loop()
{
  wdt_reset(); //Pet the dog

  //The TWI interrupt will fire whenever it fires and adds incoming I2C characters to the buffer
  //As does the SPI interrupt
  //Serial is the only one that needs special attention
  serialEvent(); //Check the serial buffer for new data

  while (buffer.tail != buffer.head) updateDisplay(); //If there is new data in the buffer, display it!

  //Once we've cleared the buffer, go to sleep
  sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received
}

// updateDisplay(): This beast of a function is called by the main loop
// If the data relates to a commandMode or settingMode will be set accordingly or a command/setting
// will be executed from this function.
// If the incoming data is just a character it will be displayed
void updateDisplay()
{
  wdt_reset(); //Pet the dog

  // First we read from the oldest data in the buffer
  byte incoming = buffer.data[buffer.tail];
  buffer.tail = (buffer.tail + 1) % BUFFER_SIZE;  // and update the tail to the next oldest

  //If the last byte received wasn't special
  if (currentMode == MODE_NORMAL)
  {
    //Check to see if the incoming byte is special
    if (incoming == SPECIAL_SETTING) //SPECIAL_SETTING is 127
    {
      currentMode = MODE_SETTING;
    }
    else if (incoming == SPECIAL_COMMAND) //SPECIAL_COMMAND is 254
    {
      currentMode = MODE_COMMAND;
    }
    else if (incoming == 8) //Backspace
    {
      if (characterCount == 0) characterCount = settingLCDwidth * settingLCDlines; //Special edge case

      characterCount--; //Back up

      currentFrame[characterCount] = ' '; //Erase this spot from the buffer
      displayFrameBuffer(); //Display what we've got
    }
    else //Simply display this character to the screen
    {
      SerLCD.write(incoming);

      currentFrame[characterCount++] = incoming; //Record this character to the display buffer
      if (characterCount == settingLCDwidth * settingLCDlines) characterCount = 0; //Wrap condition
    }
  }
  else if (currentMode == MODE_SETTING)
  {
    currentMode = MODE_NORMAL; //We assume we will be returning to normal

    //LCD width and line settings
    if (incoming >= 3 && incoming <= 7) //Ctrl+c to Ctrl+g
    {
      //Convert incoming value down to 0 to 4
      changeLinesWidths(incoming - 3);
    }
    //Software reset
    else if (incoming == 8) //Ctrl+h
    {
      while (1); //Hang out and let the watchdog punish us
    }
    //Enable / disable splash setting
    else if (incoming == 9) //Ctrl+i
    {
      changeSplashEnable();
    }
    //Save current buffer as splash
    else if (incoming == 10) //Ctrl+j
    {
      changeSplashContent();
    }
    //Set baud rate
    else if (incoming >= 11 && incoming <= 23) //Ctrl+k to ctrl+w
    {
      //Convert incoming value down to 0
      changeUARTSpeed(incoming - 11);
    }
    //Set contrast
    else if (incoming == 24) //Ctrl+x
    {
      currentMode = MODE_CONTRAST; //Go to new mode
      //We now grab the next character on the next loop and use it to change the contrast
    }
    //Set TWI address
    else if (incoming == 25) //Ctrl+y
    {
      currentMode = MODE_TWI; //Go to new mode
      //We now grab the next character on the next loop and use it to change the TWI address
    }
    //Control ignore RX on boot
    else if (incoming == 26) //Ctrl+z
    {
      changeIgnore();
    }
    //Clear screen and buffer
    else if (incoming == 45) //'-'
    {
      SerLCD.clear();
      SerLCD.setCursor(0, 0);

      clearFrameBuffer(); //Get rid of all characters in our buffer
    }
    //Backlight Red or standard white
    else if (incoming >= SPECIAL_RED_MIN && incoming <= (SPECIAL_RED_MIN + 29))
    {
      byte brightness = map(incoming, SPECIAL_RED_MIN, SPECIAL_RED_MIN + 29, 0, 255); //Covert 30 digit value to 255 digits
      changeBLBrightness(RED, brightness);
    }
    //Backlight Green
    else if (incoming >= SPECIAL_GREEN_MIN && incoming <= (SPECIAL_GREEN_MIN + 29))
    {
      byte brightness = map(incoming, SPECIAL_GREEN_MIN, SPECIAL_GREEN_MIN + 29, 0, 255); //Covert 30 digit value to 255 digits
      changeBLBrightness(GREEN, brightness);
    }
    //Backlight Blue
    else if (incoming >= SPECIAL_BLUE_MIN && incoming <= (SPECIAL_BLUE_MIN + 29))
    {
      byte brightness = map(incoming, SPECIAL_BLUE_MIN, SPECIAL_BLUE_MIN + 29, 0, 255); //Covert 30 digit value to 255 digits
      changeBLBrightness(BLUE, brightness);
    }
    //Record custom characters
    else if (incoming >= 27 && incoming <= 34)
    {
      //User can record up to 8 custom chars
      customCharNumber = incoming - 27; //Get the custom char spot to record to

      currentMode = MODE_RECORD_CUSTOM_CHAR; //Change to this special mode
    }

    //Display custom characters, 8 characters allowed, 35 to 42 inclusive
    else if (incoming >= 35 && incoming <= 42)
    {
      SerLCD.write(byte(incoming - 35)); //You write location zero to display customer char 0
    }
    //If we get a second special setting character, then write it to the display
    //This allows us to print a pipe by escaping it as a double
    else if (incoming == SPECIAL_SETTING) {
      SerLCD.write(incoming);

      currentFrame[characterCount++] = incoming; //Record this character to the display buffer
      if (characterCount == settingLCDwidth * settingLCDlines) characterCount = 0; //Wrap condition
    }
    //Set Backlight RGB in one command to eliminate flicker
    else if (incoming == 43) {
      currentMode = MODE_SET_RGB; //Go to new mode
    }
  }
  else if (currentMode == MODE_TWI)
  {
    //Custom TWI address
    changeTWIAddress(incoming);

    currentMode = MODE_NORMAL; //Return to normal operation
  }
  else if (currentMode == MODE_COMMAND) //Deal with lower level commands
  {
    currentMode = MODE_NORMAL; //In general, return to normal mode

    if (incoming >> 7 == 1) //This is a cursor position command
    {
      incoming &= 0x7F; //Get rid of the leading 1

      byte line = 0;
      byte spot = 0;
      if (incoming >= 0 && incoming <= 19)
      {
        spot = incoming;
        line = 0;
      }
      else if (incoming >= 64 && incoming <= 83)
      {
        spot = incoming - 64;
        line = 1;
      }
      else if (incoming >= 20 && incoming <= 39)
      {
        spot = incoming - 20;
        line = 2;
      }
      else if (incoming >= 84 && incoming <= 103)
      {
        spot = incoming - 84;
        line = 3;
      }

      SerLCD.setCursor(spot, line); //(x, y) - Set to X spot on the given line
    }
    else if (incoming >> 6 == 1) //This is Set CGRAM address command
    {
      //User is trying to create custom character

      incoming &= 0b10111111; //Clear the ACG bit

      //User can record up to 8 custom chars
      customCharNumber = incoming - 27; //Get the custom char spot to record to

      currentMode = MODE_RECORD_CUSTOM_CHAR; //modeRecordCustomChar = true; //Change to this special mode
    }
    else if (incoming >> 4 == 1) //This is a scroll/shift command
    {
      /*See page 24/25 of the datasheet: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
        Bit 3: (S/C) 1 = Display shift, 0 = cursor move
        Bit 2: (R/L) 1 = Shift to right, 0 = shift left
      */

      //Check for display shift or cursor shift
      if (incoming & 1 << 3) //Display shift
      {
        if (incoming & 1 << 2) SerLCD.scrollDisplayRight(); //Go right
        else SerLCD.scrollDisplayLeft(); //Go left
      }
      else //Cursor move
      {
        //Check for right/left cursor move
        if (incoming & 1 << 2) //Right shift
        {
          characterCount++; //Move cursor right
          if (characterCount == settingLCDwidth * settingLCDlines) characterCount = 0; //Wrap condition
        }
        else
        {
          if (characterCount == 0) characterCount = settingLCDwidth * settingLCDlines; //Special edge case
          characterCount--; //Move cursor left
        }
        SerLCD.setCursor(characterCount % settingLCDwidth, characterCount / settingLCDwidth); //Move the cursor
      }
    }
    else if (incoming >> 3 == 1) //This is a cursor or display on/off control command
    {
      /*See page 24 of the datasheet: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

        Bit 3: Always 1 (1<<3)
        Bit 2: 1 = Display on, 0 = display off
        Bit 1: 1 = Cursor displayed (an underline), 0 = cursor not displayed
        Bit 0: 1 = Blinking box displayed, 0 = blinking box not displayed

        You can combine bits 1 and 2 to turn on the underline and then blink a box. */

      //Check for blinking box cursor on/off
      if (incoming & 1 << 0) SerLCD.blink();
      else SerLCD.noBlink();

      //Check for underline cursor on/off
      if (incoming & 1 << 1) SerLCD.cursor();
      else SerLCD.noCursor();

      //Check for display on/off
      if (incoming & 1 << 2) SerLCD.display();
      else SerLCD.noDisplay();
    }
    else if (incoming >> 4 != 0b00000011) //If not the data length (DL) command then send it to LCD
    {
      //We ignore the command that could set LCD to 8bit mode
      //But otherwise give the user the ability to pass commands directly
      //into the LCD.
      SerLCD.command(incoming);
    }
  }
  else if (currentMode == MODE_RECORD_CUSTOM_CHAR)
  {
    //We get into this mode if the user has sent the correct setting or system command

    customCharData[customCharSpot] = incoming; //Record this byte to the array

    customCharSpot++;
    if (customCharSpot > 7)
    {
      //Once we have 8 bytes, stop listening
      customCharSpot = 0; //Wrap variable at max of 7

      SerLCD.createChar(customCharNumber, customCharData); //Record the array to CGRAM

      //Record this custom char to EEPROM
      for (byte charSpot = 0 ; charSpot < 8 ; charSpot++)
        EEPROM.write(LOCATION_CUSTOM_CHARACTERS + (customCharNumber * 8) + charSpot, customCharData[charSpot]); //addr, val

      //For some reason you need to re-init the LCD after a custom char is created
      SerLCD.begin(settingLCDwidth, settingLCDlines);

      currentMode = MODE_NORMAL; //Exit this mode
    }
  }
  else if (currentMode == MODE_CONTRAST)
  {
    //We get into this mode if the user has sent the ctrl+x (24) command to change contast
    changeContrast(incoming);
    currentMode = MODE_NORMAL; //Exit this mode
  }
  else if (currentMode == MODE_SET_RGB)
  {
    //We get into this mode if the user has sent the + (43) command to set the backlight rgb values
    rgbData[rgbSpot] = incoming; //Record this byte to the array

    rgbSpot++;
    if (rgbSpot > 2)
    {
      //Once we have 3 bytes, stop listening and change the backlight color
      rgbSpot = 0;
      changeBacklightRGB(rgbData[0], rgbData[1], rgbData[2]);
      currentMode = MODE_NORMAL; //Exit this mode
    } //if (rgbSpot > 2)
  } // else if modeSetRGB

}

//Flushes all characters from the frame buffer
void clearFrameBuffer()
{
  //Clear the frame buffer
  characterCount = 0;
  for (byte x = 0 ; x < (settingLCDwidth * settingLCDlines) ; x++)
    currentFrame[x] = ' ';
}

//Display the LCD buffer and return the cursor to where it was before the system message
void displayFrameBuffer(void)
{
  //Return display to previous buffer
  SerLCD.clear();
  SerLCD.setCursor(0, 0);

  for (byte x = 0 ; x < (settingLCDlines * settingLCDwidth) ; x++)
    SerLCD.write(currentFrame[x]);

  //Return the cursor to its original position
  SerLCD.setCursor(characterCount % settingLCDwidth, characterCount / settingLCDwidth);
}
