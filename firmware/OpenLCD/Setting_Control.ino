/*
  OpenLCD control for changing settings

  See main file for license and information.
*/

//Toggle the ignore rx setting
void changeIgnore()
{
  byte settingIgnoreRX = EEPROM.read(LOCATION_IGNORE_RX);

  //Display new settings to the user
  SerLCD.clear();
  SerLCD.setCursor(0, 0);

  SerLCD.print(F("Ignore RX O"));

  if (settingIgnoreRX == true)
  {
    settingIgnoreRX = false;
    SerLCD.print(F("FF"));
  }
  else
  {
    settingIgnoreRX = true;
    SerLCD.print(F("N"));
  }
  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  //Record this new setting
  EEPROM.write(LOCATION_IGNORE_RX, settingIgnoreRX);

  displayFrameBuffer(); //Return the contents of the display
}

//Change the digital contrast
//Press a or z to adjust, x to exit
void changeContrast(byte contrast)
{
  EEPROM.write(LOCATION_CONTRAST, contrast); //Store this new contrast

  //Go to this new contrast
  if (DISPLAY_TYPE == LCD) analogWrite(LCD_CONTRAST, contrast);
  else if (DISPLAY_TYPE == OLED)
  {
    SerLCD.command(0x2A); //Command: Function Set, set extension register (RE)
    SerLCD.command(0x79); //Command: OLED Characterization, OLED command set is enabled
    SerLCD.command(0x81); //Set Contrast Control
    SerLCD.command(contrast); //Set Contrast Control: 0 to 255
    SerLCD.command(0x78); //Command: OLED Characterization, OLED command set is disabled
    SerLCD.command(0x28); //Command: Function Set, clear extension register (RE)
  }

  //Display the new contrast
  SerLCD.clear();
  SerLCD.setCursor(0, 0); //First position, 1st row
  SerLCD.print("Contrast Set");
  SerLCD.setCursor(0, 1); //First position, 2nd row
  SerLCD.print(contrast);

  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Display what was there before
}

//Change the I2C or TWI address
void changeTWIAddress(byte newAddress)
{
  //Record the new address
  EEPROM.write(LOCATION_TWI_ADDRESS, newAddress);

  setupTWI(); //Leverage the regular startup function

  //Display the new TWI address
  SerLCD.clear();
  SerLCD.setCursor(0, 0); //First position, 1st row

  SerLCD.print("New TWI: 0x");
  SerLCD.print(newAddress, HEX);

  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Display what was there before
}

//Save the current frame buffer to EEPROM as the splash screen
void changeSplashContent()
{
  //Record the current frame to EEPROM
  for (byte x = 0 ; x < settingLCDlines * settingLCDwidth ; x++)
    EEPROM.write(LOCATION_SPLASH_CONTENT + x, currentFrame[x]);

  //Display the backlight setting
  SerLCD.clear();
  SerLCD.setCursor(0, 0); //First position, 1st row

  SerLCD.print("Splash Recorded");

  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Display what was there before
}

//Changes the brightness of a given pin and updates the EEPROM location with that value
//Incoming brightness should be 0 to 255
void changeBLBrightness(byte color, byte brightness)
{
  if (color == RED)
  {
    EEPROM.write(LOCATION_RED_BRIGHTNESS, brightness); //Record new setting
    analogWrite(BL_RW, 255 - brightness); //Controlled by PNP so reverse the brightness value
  }
  else if (color == GREEN)
  {
    EEPROM.write(LOCATION_GREEN_BRIGHTNESS, brightness); //Record new setting
    analogWrite(BL_G, 255 - brightness); //Controlled by PNP so reverse the brightness value
  }
  else if (color == BLUE)
  {
    EEPROM.write(LOCATION_BLUE_BRIGHTNESS, brightness); //Record new setting
    //analogWrite(BL_B, 255 - brightness); //Controlled by PNP so reverse the brightness value
    SoftPWMSet(BL_B, 255 - brightness); //Controlled by software PWM
  }

  //Display the backlight setting
  SerLCD.clear();
  SerLCD.setCursor(0, 0); //First position, 1st row

  if (color == RED)
    SerLCD.print(F("Backlight"));
  else if (color == GREEN)
    SerLCD.print(F("Green"));
  else if (color == BLUE)
    SerLCD.print(F("Blue"));

  SerLCD.print(F(": "));

  brightness = map(brightness, 0, 255, 0, 100); //Covert to percentage
  SerLCD.print(brightness);
  SerLCD.print(F("%"));
  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Display what was there before
}

//Changes the brightness of all three backlight pins and updates the EEPROM locations
//with their rgb values to eliminate flicker. Incoming brightness values should be 0 to 255
void changeBacklightRGB(byte red, byte green, byte blue) {
  //update red
  EEPROM.write(LOCATION_RED_BRIGHTNESS, red); //Record new setting
  analogWrite(BL_RW, 255 - red); //Controlled by PNP so reverse the brightness value
  //update green
  EEPROM.write(LOCATION_GREEN_BRIGHTNESS, green); //Record new setting
  analogWrite(BL_G, 255 - green); //Controlled by PNP so reverse the brightness value
  //update blue (SoftPWM)
  EEPROM.write(LOCATION_BLUE_BRIGHTNESS, blue); //Record new setting
  //analogWrite(BL_B, 255 - brightness); //Controlled by PNP so reverse the brightness value
  SoftPWMSet(BL_B, 255 - blue); //Controlled by software PWM

  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Display what was there before
}


//Changes the baud rate setting
//Assumes caller is passing a number 0 to 12
void changeUARTSpeed(byte setting)
{
  //This is ugly but we need to maintain compatibility with SerLCD v2.5 units
  switch (setting)
  {
    case 0: //Ctrl+k
      settingUARTSpeed = BAUD_2400;
      break;
    case 1: //Ctrl+l
      settingUARTSpeed = BAUD_4800;
      break;
    case 2: //Ctrl+m
      settingUARTSpeed = BAUD_9600;
      break;
    case 3: //Ctrl+n
      settingUARTSpeed = BAUD_14400;
      break;
    case 4: //Ctrl+o
      settingUARTSpeed = BAUD_19200;
      break;
    case 5: //Ctrl+p
      settingUARTSpeed = BAUD_38400;
      break;
    case 6: //Ctrl+q
      settingUARTSpeed = BAUD_57600;
      break;
    case 7: //Ctrl+r
      settingUARTSpeed = BAUD_115200;
      break;
    case 8: //Ctrl+s
      settingUARTSpeed = BAUD_230400;
      break;
    case 9: //Ctrl+t
      settingUARTSpeed = BAUD_460800;
      break;
    case 10: //Ctrl+u
      settingUARTSpeed = BAUD_921600;
      break;
    case 11: //Ctrl+v
      settingUARTSpeed = BAUD_1000000;
      break;
    case 12: //Ctrl+w
      settingUARTSpeed = BAUD_1200;
      break;
  }

  //Record this new buad rate
  EEPROM.write(LOCATION_BAUD, settingUARTSpeed);

  //Display that we are at this new speed
  SerLCD.clear();
  SerLCD.setCursor(0, 0); //First position, 1st row
  SerLCD.print(F("Baud now:"));
  SerLCD.print(lookUpBaudRate(settingUARTSpeed));
  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  //Go to this new baud rate
  Serial.begin(lookUpBaudRate(settingUARTSpeed));

  displayFrameBuffer(); //Display what was there before
}

void changeSplashEnable()
{
  byte settingSplashEnable = EEPROM.read(LOCATION_SPLASH_ONOFF);

  //Display new settings to the user
  SerLCD.clear();
  SerLCD.setCursor(0, 0);

  SerLCD.print(F("Splash O"));

  if (settingSplashEnable == true)
  {
    settingSplashEnable = false;
    SerLCD.print(F("FF"));
  }
  else
  {
    settingSplashEnable = true;
    SerLCD.print(F("N"));
  }
  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  //Record this new setting
  EEPROM.write(LOCATION_SPLASH_ONOFF, settingSplashEnable);

  displayFrameBuffer(); //Return the contents of the display
}

void changeLinesWidths(byte setting)
{
  switch (setting)
  {
    case 0:
      settingLCDwidth = 20;
      break;
    case 1:
      settingLCDwidth = 16;
      break;
    case 2:
      settingLCDlines = 4;
      break;
    case 3:
      settingLCDlines = 2;
      break;
    case 4:
      settingLCDlines = 1;
      break;
  }

  SerLCD.begin(settingLCDwidth, settingLCDlines); //Go to new setting

  //Very funky characters can show up here because the buffer has been resized
  //Not sure if we should clear the buffer or not. User would loose the characters on the current screen
  clearFrameBuffer();

  //Record this new setting
  EEPROM.write(LOCATION_WIDTH, settingLCDwidth);
  EEPROM.write(LOCATION_LINES, settingLCDlines);

  //Display new settings to the user
  SerLCD.clear();
  SerLCD.setCursor(0, 0);

  SerLCD.print(F("Lines:"));
  SerLCD.print(settingLCDlines);

  //If we have a single line LCD then clear the message after a second and print more
  if (settingLCDlines == 1)
  {
    petSafeDelay(SYSTEM_MESSAGE_DELAY);
    SerLCD.clear();
    SerLCD.setCursor(0, 0);
  }
  else
  {
    SerLCD.setCursor(0, 1); //We are assuming at least a two line LCD
  }

  SerLCD.print(F("Width:"));
  SerLCD.print(settingLCDwidth);
  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Return the contents of the display
}
