/*
  OpenLCD control for changing settings

  See main file for license and information.
*/

//Toggle the ignore rx setting
void changeIgnore()
{
  settingIgnoreRX = EEPROM.read(LOCATION_IGNORE_RX);

  //Toggle the setting
  if (settingIgnoreRX == true)
    settingIgnoreRX = false;
  else
    settingIgnoreRX = true;

  //Record this new setting
  EEPROM.update(LOCATION_IGNORE_RX, settingIgnoreRX);

  if (settingDisplaySystemMessages == true)
  {
    //Display new settings to the user
    SerLCD.clear();
    SerLCD.setCursor(0, 0);

    SerLCD.print(F("Ignore RX O"));

    if (settingIgnoreRX == false)
      SerLCD.print(F("FF"));
    else
      SerLCD.print(F("N"));

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Return the contents of the display
  }
}

//Turn on messages like 'Contrast: 5' when user changes setting
void enableDisplaySystemMessages()
{
  settingDisplaySystemMessages = true;

  //Record this new setting
  EEPROM.update(LOCATION_DISPLAY_SYSTEM_MESSAGES, settingDisplaySystemMessages);

  //Display new setting to the user
  SerLCD.clear();
  SerLCD.setCursor(0, 0);
  SerLCD.print(F("Messages ON"));
  petSafeDelay(SYSTEM_MESSAGE_DELAY);
  displayFrameBuffer(); //Return the contents of the display
}

//Turn off system messsages
void disableDisplaySystemMessages()
{
  settingDisplaySystemMessages = false;

  //Record this new setting
  EEPROM.update(LOCATION_DISPLAY_SYSTEM_MESSAGES, settingDisplaySystemMessages);
}

//Display the current firmware version for a set amount of time
void displayFirmwareVersion()
{
  SerLCD.clear();
  SerLCD.setCursor(0, 0);

  SerLCD.print(F("Firmware v"));
  SerLCD.print(firmwareVersionMajor);
  SerLCD.print(F("."));
  SerLCD.print(firmwareVersionMinor);

  petSafeDelay(SYSTEM_MESSAGE_DELAY);

  displayFrameBuffer(); //Return the contents of the display
}

//Change the digital contrast
//Press a or z to adjust, x to exit
void changeContrast(byte contrast)
{
  EEPROM.update(LOCATION_CONTRAST, contrast); //Store this new contrast

  //Go to this new contrast
  analogWrite(LCD_CONTRAST, contrast);

  if (settingDisplaySystemMessages == true)
  {
    //Display the new contrast
    SerLCD.clear();
    SerLCD.setCursor(0, 0); //First position, 1st row
    SerLCD.print("Contrast Set");
    SerLCD.setCursor(0, 1); //First position, 2nd row
    SerLCD.print(contrast);

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Display what was there before
  }
}

//Change the I2C or TWI address
void changeTWIAddress(byte newAddress)
{
  //Record the new address
  EEPROM.update(LOCATION_TWI_ADDRESS, newAddress);

  setupTWI(); //Leverage the regular startup function

  if (settingDisplaySystemMessages == true)
  {
    //Display the new TWI address
    SerLCD.clear();
    SerLCD.setCursor(0, 0); //First position, 1st row

    SerLCD.print("New TWI: 0x");
    SerLCD.print(newAddress, HEX);

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Display what was there before
  }
}

//Change the remote client TWIST address
void changeTWISTAddress(byte newAddress)
{
  //Record the new address
  EEPROM.update(LOCATION_TWI_CLIENT_ADDR, newAddress);

  setupTWI(); //Leverage the regular startup function

  if (settingDisplaySystemMessages == true)
  {
    //Display the new TWI address
    SerLCD.clear();
    SerLCD.setCursor(0, 0); //First position, 1st row

    SerLCD.print("New TWIST: 0x");
    SerLCD.print(newAddress, HEX);

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Display what was there before
  }
}

//Save the current frame buffer to EEPROM as the splash screen
void changeSplashContent()
{
  //Record the current frame to EEPROM
  for (byte x = 0 ; x < settingLCDlines * settingLCDwidth ; x++)
    EEPROM.update(LOCATION_SPLASH_CONTENT + x, currentFrame[x]);

  if (settingDisplaySystemMessages == true)
  {
    //Display the backlight setting
    SerLCD.clear();
    SerLCD.setCursor(0, 0); //First position, 1st row

    SerLCD.print("Splash Recorded");

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Display what was there before
  }
}

//Changes the brightness of a given pin and updates the EEPROM location with that value
//Incoming brightness should be 0 to 255
void changeBLBrightness(byte color, byte brightness)
{
  byte red = EEPROM.read(LOCATION_RED_BRIGHTNESS);
  byte green = EEPROM.read(LOCATION_GREEN_BRIGHTNESS);
  byte blue = EEPROM.read(LOCATION_BLUE_BRIGHTNESS);
  if (color == RED)
  {
    EEPROM.update(LOCATION_RED_BRIGHTNESS, brightness); //Record new setting
    analogWrite(BL_RW, 255 - brightness); //Controlled by PNP so reverse the brightness value
    if ( EEPROM.read(LOCATION_TWIST_FOLLOW_LCD) > 0) {
      changeTwistlightRGB(brightness, green, blue);
    }
  }
  else if (color == GREEN)
  {
    EEPROM.update(LOCATION_GREEN_BRIGHTNESS, brightness); //Record new setting
    analogWrite(BL_G, 255 - brightness); //Controlled by PNP so reverse the brightness value
    if ( EEPROM.read(LOCATION_TWIST_FOLLOW_LCD) > 0) {
      changeTwistlightRGB(red, brightness, blue);
    }
  }
  else if (color == BLUE)
  {
    EEPROM.update(LOCATION_BLUE_BRIGHTNESS, brightness); //Record new setting
    //analogWrite(BL_B, 255 - brightness); //Controlled by PNP so reverse the brightness value
    SoftPWMSet(BL_B, brightness); //Controlled by software PWM. Reversed by SoftPWM
    if ( EEPROM.read(LOCATION_TWIST_FOLLOW_LCD) > 0) {
      changeTwistlightRGB(red, green, brightness);
    }
  }

  if (settingDisplaySystemMessages == true)
  {
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
}

//Changes the brightness of all three backlight pins and updates the EEPROM locations
//with their rgb values to eliminate flicker. Incoming brightness values should be 0 to 255
void changeBacklightRGB(byte red, byte green, byte blue) {
  //update red
  EEPROM.update(LOCATION_RED_BRIGHTNESS, red); //Record new setting
  analogWrite(BL_RW, 255 - red); //Controlled by PNP so reverse the brightness value

  //update green
  EEPROM.update(LOCATION_GREEN_BRIGHTNESS, green); //Record new setting
  analogWrite(BL_G, 255 - green); //Controlled by PNP so reverse the brightness value

  //update blue (SoftPWM)
  EEPROM.update(LOCATION_BLUE_BRIGHTNESS, blue); //Record new setting
  //analogWrite(BL_B, 255 - brightness); //Controlled by PNP so reverse the brightness value
  SoftPWMSet(BL_B, blue); //Controlled by software PWM. Reversed by SoftPWM
  if ( EEPROM.read(LOCATION_TWIST_FOLLOW_LCD) > 0) {
    changeTwistlightRGB(red, green, blue);
  }
}

void changeTwistlightRGB(byte red, byte green, byte blue) {
  //update EEPROM
  EEPROM.update(LOCATION_TWIST_RED_BRIGHTNESS, red); //Record new setting
  EEPROM.update(LOCATION_TWIST_GREEN_BRIGHTNESS, green); //Record new setting
  EEPROM.update(LOCATION_TWIST_BLUE_BRIGHTNESS, blue); //Record new setting

  //update the TWIST
  if (twistPresent == true) {
    twist.setColor(red, green, blue);
  }
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
  EEPROM.update(LOCATION_BAUD, settingUARTSpeed);

  //Go to this new baud rate
  Serial.begin(lookUpBaudRate(settingUARTSpeed));

  if (settingDisplaySystemMessages == true)
  {
    //Display that we are at this new speed
    SerLCD.clear();
    SerLCD.setCursor(0, 0); //First position, 1st row
    SerLCD.print(F("Baud now:"));
    SerLCD.print(lookUpBaudRate(settingUARTSpeed));
    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Display what was there before
  }
}

void changeSplashEnable()
{
  settingSplashEnable = EEPROM.read(LOCATION_SPLASH_ONOFF);

  //Toggle setting
  if (settingSplashEnable == true)
    settingSplashEnable = false;
  else
    settingSplashEnable = true;

  //Record this new setting
  EEPROM.update(LOCATION_SPLASH_ONOFF, settingSplashEnable);

  if (settingDisplaySystemMessages == true)
  {
    //Display new settings to the user
    SerLCD.clear();
    SerLCD.setCursor(0, 0);

    SerLCD.print(F("Splash O"));

    if (settingSplashEnable == true)
      SerLCD.print(F("FF"));
    else
      SerLCD.print(F("N"));

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Return the contents of the display
  }
}

//Turn on splash at power on
void enableSplash()
{
  settingSplashEnable = true;

  //Record this new setting
  EEPROM.update(LOCATION_SPLASH_ONOFF, settingSplashEnable);

  if (settingDisplaySystemMessages == true)
  {
    //Display new settings to the user
    SerLCD.clear();
    SerLCD.setCursor(0, 0);

    SerLCD.print(F("Splash ON"));

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Return the contents of the display
  }
}

//Disable the power on splash
void disableSplash()
{
  settingSplashEnable = false;

  //Record this new setting
  EEPROM.update(LOCATION_SPLASH_ONOFF, settingSplashEnable);

  if (settingDisplaySystemMessages == true)
  {
    //Display new settings to the user
    SerLCD.clear();
    SerLCD.setCursor(0, 0);

    SerLCD.print(F("Splash OFF"));

    petSafeDelay(SYSTEM_MESSAGE_DELAY);

    displayFrameBuffer(); //Return the contents of the display
  }
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
  EEPROM.update(LOCATION_WIDTH, settingLCDwidth);
  EEPROM.update(LOCATION_LINES, settingLCDlines);

  if (settingDisplaySystemMessages == true)
  {
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
}

// poll the Twist Rotoray Encoder and Button
void pollTwist(void)
{
  if (twistPresent == true)
  {
    bool pressed;
    unsigned long currentMillis = millis();

    if(currentMillis - twistPreviousMillis > twistSampleDelay) {
      // delay needed to match speed with Twist
      twistPreviousMillis = currentMillis;   
    
      if (twist.getDiff())
      {
        pollTwistEncoder();
      }

      pressed = twist.isPressed();
      if (pressed != twistButtonPressed)
      {
        printTwistButton(pressed);
        twistButtonPressed = pressed;
      }
    }
  }
}

//Poll the Twist rotorary encoder count
void pollTwistEncoder(void)
{
  Serial.print("Count: ");
  Serial.println(twist.getCount());
}

//Poll the Twist rotorary button
void pollTwistButton(void)
{
  printTwistButton(twist.isPressed());
}

void printTwistButton(bool pressed)
{
  if (pressed)
    Serial.println("Button depressed");
  else
    Serial.println("Button released");
}

void disableTwistStreaming()
{
  enableTwistStream = false;

  //Record this new setting
  EEPROM.update(LOCATION_TWIST_STREAM, enableTwistStream);
}

void enableTwistStreaming()
{
  enableTwistStream = true;

  //Record this new setting
  EEPROM.update(LOCATION_TWIST_STREAM, enableTwistStream);
}

