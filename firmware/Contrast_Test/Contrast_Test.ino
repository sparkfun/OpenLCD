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

 Todo:
 Check for size jumper
 Check how splash screen works on 16 vs 20 width displays
 Display message when resetting baud rate
 Display message when changing baud rate
 Add additional baud rates
 Document support for 1 line LCDs?
 Create and document support for re_init command: 124 then 8. Does SerLCD v2 have a clear or reset everything command? It should. Document it.
 Add support for custom I2C addresses. This might be a third tier command in order to maintain backwards compatibility
 Can we shut down/sleep while we wait for incoming things?
 Add watchdog so that we never freeze/fail

 Tests:
 Change LCD width to 20, then back to 16 (124/3, then 124/4) then send 18 characters and check for wrap
 Enable/Disable splash screen, send 124 then 9 to toggle, then power cycle
 Change baud rate: 124/12 to go to 4800bps, power cycle, send characters at 4800

 Emergency reset to 9600bps


*/

#include <Wire.h> //For I2C functions
#include <LiquidCrystalFast.h>
#include <EEPROM.h>  //Brightness, Baud rate, and I2C address are stored in EEPROM
#include "settings.h" //Defines EEPROM locations for user settings

LiquidCrystalFast SerLCD(LCD_RS, LCD_RW, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

byte cursorX = 0, cursorY = 0; //This keeps track of where we are on the screen
byte splashLine1[20]; //These arrays contain the characters that will be displayed
byte splashLine2[20]; //if splash is enabled

//byte endOfLine1, endOfLine2, endOfLine3, endOfLine4; //Hold the cursor value for the end of each line.
//These depend on LCD width and are calculated at each power on or setting change

//byte startingPosition1, startingPosition2, startingPosition3, startingPosition4; 
//Hold the starting cursor position for each line.
//These depend on LCD width and are calculated at each power on or setting change

boolean commandMode = false; //Used to indicate if a commandMode byte has been received
boolean settingMode = false; //Used to indicate if a commandMode byte has been received

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
//  setupSplash(); //Read and display the user's splash screen

  setupUART(); //Setup serial
//  setupSPI(); //Initialize SPI stuff (enable, mode, interrupts)
  setupTWI(); //Initialize I2C stuff (address, interrupt, enable)

  //  interrupts();  // Turn interrupts on, and les' go
  
  pinMode(9, OUTPUT);
  
  SerLCD.print("Hello world");
  analogWrite(9, 20);
  
  while(1);
}

void loop()
{
  //This is kind of freaky but stick with me:
  //The TWI interrupt will fire whenever it fires and adds incoming I2C characters to the buffer
  //As does the SPI interrupt
  //Serial is the only one that needs special attention
  serialEvent(); //Check the serial buffer for new data

  while(buffer.tail != buffer.head) updateDisplay(); //If there is new data in the buffer, display it!
  
}

// updateDisplay(): This beast of a function is called by the main loop if there is new data in the buffer. 
// If the data relates to a commandMode or settingMode will be set accordingly or a command/setting 
// will be executed from this function.
// If the incoming data is just a character it will be displayed
void updateDisplay()
{

  // First we read from the oldest data in the buffer
  unsigned char incoming = buffer.data[buffer.tail];
  buffer.tail = (buffer.tail + 1) % BUFFER_SIZE;  // and update the tail to the next oldest

  // if the last byte received wasn't a command byte (commandMode=0)
  // and if the data is is not the two special command characters
  if ((commandMode == false && settingMode == false) && (incoming != SPECIAL_COMMAND && incoming != SPECIAL_SETTING))
  {
    SerLCD.write(incoming); //Just write this character to the screen

    //Keep the first 2 lines logged for splash screen changes
    if (cursorY == 0 && cursorX < settingLCDwidth) splashLine1[cursorX] = incoming;
    if (cursorY == 1 && cursorX < settingLCDwidth) splashLine2[cursorX] = incoming;
    //TODO add check for more than 2 line LCDs

    //When the cursor gets to the end of one line, it must advance to the next visual line
    /*cursorX++; //Advance a spot
    if(cursorX == settingLCDwidth) //Wrap it!
    {
      SerLCD.write('w');
      cursorX = 0;
      cursorY++;
      if(cursorY == settingLCDlines) cursorY = 0;//Wrap it!
      
      SerLCD.setCursor(cursorX, cursorY); //X position, Y row
    }*/

//TODO: Does the LCD library wrap things naturally? Do we need the .setcursor command?
        
    //display.digits[display.cursor] = c;  // just store the read data into the cursor-active digit
    //display.cursor = ((display.cursor + 1) % 4);  // Increment cursor, set back to 0 if necessary
  }

/*
  else if (commandMode == true)  // Otherwise, if data is non-displayable and we're in a commandMode
  {
    //TODO Fill in and make sure we don't set LCD to 8bit mode
    commandMode = 0;
  }
  else if (settingMode == true)
  {
    //LCD width and line settings
    if (incoming >= 3 && incoming <= 6)
    {
      switch (incoming)
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

    //Enable / disable splash setting
    else if (incoming == 9)
    {
      if (settingSplashEnable == true)
        settingSplashEnable = false;
      else
        settingSplashEnable = true;

      //Record this new setting
      EEPROM.write(LOCATION_SPLASH_ONOFF, settingSplashEnable);
    }

    //Set baud rate
    else if (incoming <= 11 && incoming >= 19)
    {
      switch (incoming)
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
        case 17:
          settingUARTSpeed = BAUD_57600;
          break;
        case 18:
          settingUARTSpeed = BAUD_76800;
          break;
        case 19:
          settingUARTSpeed = BAUD_115200;
          break;

          //Add additional baud rates here but don't forget to increase the setting check parameters
      }

      //Record this new buad rate
      EEPROM.write(LOCATION_BAUD, settingUARTSpeed);

      //Display on LCD that we are at this new speed
      SerLCD.clear(); //Get rid of any garbage on the screen
      SerLCD.setCursor(0, 0); //First position, 1st row
      SerLCD.print("Baud now:");
      SerLCD.print(settingUARTSpeed);

      //Go to this new baud rate
      Serial.begin(settingUARTSpeed);
    }

    //Backlight Red or standard white
    if (incoming >= SPECIAL_RED_MIN && incoming <= (SPECIAL_RED_MIN+29))
    {
      byte settingBLred = map(incoming, SPECIAL_RED_MIN, SPECIAL_RED_MIN+29, 0, 255); //Covert 30 digit value to 255 digits
      EEPROM.write(LOCATION_RED_BRIGHTNESS, settingBLred); //Record new setting
      analogWrite(BL_RW, settingBLred); //Goto that setting
    }

    //Backlight Green
    if (incoming >= SPECIAL_GREEN_MIN && incoming <= (SPECIAL_GREEN_MIN+29))
    {
      byte settingBLgreen = map(incoming, SPECIAL_GREEN_MIN, SPECIAL_GREEN_MIN+29, 0, 255); //Covert 30 digit value to 255 digits
      EEPROM.write(LOCATION_GREEN_BRIGHTNESS, settingBLgreen); //Record new setting
      analogWrite(BL_G, settingBLgreen); //Goto that setting
    }

    //TODO Add backlight settings for green and blue here

    //Reset, re-init, clear everything out
    else if (incoming == SPECIAL_RESET)
    {
      SerLCD.clear();
      //SerLCD.setCursor(0, 0); //TODO Do we need this?
      cursorX = 0;
      cursorY = 0;
      
      //Clear the current buffers in the case user records splash screen
      for(byte x = 0 ; x < 20 ; x++)
      {
        splashLine1[x] = 0;
        splashLine2[x] = 0;
      }
    }
    
    //TODO Custom TWI address
    
    //TODO override the size jumper?
    
    //TODO Do we need a command to move the cursor?


    settingMode = 0;
  }

  else  //Finally, if we weren't in command mode, if the byte isn't displayable, we'll enter one of two modes
  {
    if(incoming == SPECIAL_SETTING) settingMode = true; //Set flag for which ever type character we recevied. This is 127
    else if(incoming == SPECIAL_COMMAND) commandMode = true; //This is 254
  }

*/

}


