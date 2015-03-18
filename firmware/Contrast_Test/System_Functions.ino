/* 
 Serial LCD System Functions

 See main file for license and information.

 These are the ISRs for system functions that allow SerLCD to run
 
 This is heavily based on the Serial 7 Segment firmware
 
*/

// SPI byte received interrupt routine
ISR(SPI_STC_vect)
{
  noInterrupts();  // don't be rude! I'll be quick...

  unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
  unsigned char c = SPDR;  // Read data byte into c, from SPI data register

  if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
  {
    buffer.data[buffer.head] = c;  // Store the data into the buffer's head
    buffer.head = i;  // update buffer head, since we stored new data
  }

  interrupts();  // Fine, you were saying?
}

// This is effectively the UART0 byte received interrupt routine
// But not quite: serialEvent is only called after each loop() interation
void serialEvent()
{
  while (Serial.available()) 
  {
    unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
    unsigned char c = Serial.read();  // Read data byte into c, from UART0 data register

    if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
    {
      buffer.data[buffer.head] = c;  // Store the data into the buffer's head
      buffer.head = i;  // update buffer head, since we stored new data
    }
  }
}

// I2C byte receive interrupt routine
// Note: this isn't an ISR. I'm using wire library (because it just works), so
// Wire.onReceive(twiReceive); should be called
void twiReceive(int rxCount)
{
  while(Wire.available())  // Do this while data is available in Wire buffer
  {
    unsigned int i = (buffer.head + 1) % BUFFER_SIZE;  // read buffer head position and increment
    unsigned char c = Wire.read();  // Read data byte into c, from Wire data buffer

    if (i != buffer.tail)  // As long as the buffer isn't full, we can store the data in buffer
    {
      buffer.data[buffer.head] = c;  // Store the data into the buffer's head
      buffer.head = i;  // update buffer head, since we stored new data
    }    
  }
}

//This sets up the UART with the stored baud rate in EEPROM
void setupUART()
{
  //Check to see if we are ignoring the RX reset or not
  byte settingIgnoreRX = EEPROM.read(LOCATION_IGNORE_RX);
  if(settingIgnoreRX > 1)
  {
    settingIgnoreRX = 0; //Don't ignore
    EEPROM.write(LOCATION_IGNORE_RX, settingIgnoreRX);
  }
    
  if(settingIgnoreRX == 0) //If we are NOT ignoring RX, then
    checkEmergencyReset(); //Look to see if the RX pin is being pulled low
  
  //Read what the current UART speed is from EEPROM memory
  //Default is 9600
  settingUARTSpeed = EEPROM.read(LOCATION_BAUD);
  if (settingUARTSpeed > BAUD_115200) //Check to see if the baud rate has ever been set
  {
    settingUARTSpeed = DEFAULT_BAUD; //Reset UART to 9600 if there is no baud rate stored
    EEPROM.write(LOCATION_BAUD, settingUARTSpeed);
  }

  //Initialize the UART
  switch (settingUARTSpeed)
  {
    case (BAUD_2400):
      Serial.begin(2400);
      break;
    case (BAUD_4800):
      Serial.begin(4800);
      break;
    case (BAUD_9600):
      Serial.begin(9600);
      break;
    case (BAUD_14400):
      Serial.begin(14400);
      break;
    case (BAUD_19200):
      Serial.begin(19200);
      break;
    case (BAUD_38400):
      Serial.begin(38400);
      break;
    case (BAUD_57600):
      Serial.begin(57600);
      break;
    case (BAUD_76800):
      Serial.begin(76800);
      break;
    case (BAUD_115200):
      Serial.begin(115200);
      break;
    /*    case(BAUD_250000):
        Serial.begin(250000);
        break;
        case(BAUD_500000):
        Serial.begin(500000);
        break;
        case(BAUD_1000000):
        Serial.begin(1000000);
        break;
        */
    default:
      //We should never reach this state, but if we do
      Serial.begin(9600);
      break;
  }

}

// setupSPI(): Initialize SPI, sets up hardware pins and enables spi and receive interrupt
// SPI is set to MODE 0 (CPOL=0, CPHA=0), slave mode, LSB first
void setupSPI()
{
  pinMode(SPI_SCK, INPUT);
  pinMode(SPI_MOSI, INPUT);
  pinMode(SPI_CS, INPUT_PULLUP);

  SPCR = (1<<SPIE) | (1<<SPE);  // Enable SPI interrupt, enable SPI
  // DORD = 0, LSB First
  // MSTR = 0, SLAVE
  // CPOL = 0, sck low when idle                  } MODE 0
  // CPHA = 0, data sampled on leading clock edge } MODE 0
  // SPR1:0 = 0, no effect (slave mode)
}

// setupTWI(): initializes I2C
// I'm using the rock-solid Wire library for this. We'll initialize TWI, setup the address,
// and tell it what interrupt function to jump to when data is received.
void setupTWI()
{
  unsigned char twiAddress;

  twiAddress = EEPROM.read(LOCATION_TWI_ADDRESS);  // read the TWI address from 

  if ((twiAddress == 0) || (twiAddress > 0x7F))  
  { // If the TWI address is invalid, use a default address
    twiAddress = DEFAULT_TWI_ADDRESS;
    EEPROM.write(LOCATION_TWI_ADDRESS, DEFAULT_TWI_ADDRESS);
  }

  Wire.begin(twiAddress);  //Initialize Wire library as slave at twiAddress address
  Wire.onReceive(twiReceive);  //Setup interrupt routine for when data is received
}

//Look up and initialize the LCD with the lines and width
void setupDisplay()
{
  //Look up LCD lines and width
  settingLCDlines = EEPROM.read(LOCATION_LINES);
  if (settingLCDlines > 4)
  {
    settingLCDlines = DEFAULT_LINES;
    EEPROM.write(LOCATION_LINES, settingLCDlines);
  }

  settingLCDwidth = EEPROM.read(LOCATION_WIDTH);
  if (settingLCDwidth > 20)
  {
    settingLCDwidth = DEFAULT_WIDTH;
    EEPROM.write(LOCATION_WIDTH, settingLCDwidth);
  }
  
//  SerLCD.begin(settingLCDwidth, settingLCDlines); //Setup the width and lines for this LCD
  SerLCD.begin(16, 2); //Setup the width and lines for this LCD
}

//Look up and start the 3 backlight pins in analog mode
void setupBacklight()
{
  pinMode(BL_RW, OUTPUT);
  pinMode(BL_G, OUTPUT);
  pinMode(BL_B, OUTPUT);
  
  analogWrite(BL_RW, EEPROM.read(LOCATION_RED_BRIGHTNESS));
  analogWrite(BL_G, EEPROM.read(LOCATION_GREEN_BRIGHTNESS));
  analogWrite(BL_B, EEPROM.read(LOCATION_BLUE_BRIGHTNESS));

//TODO Testing
  analogWrite(BL_RW, 20);
}

void setupSplash()
{
  SerLCD.clear(); //Get rid of any garbage on the screen

  //Find out if we should display the splash or not
  settingSplashEnable = EEPROM.read(LOCATION_SPLASH_ONOFF);
  if(settingSplashEnable > 1)
  {
    settingSplashEnable = DEFAULT_SPLASH;
    EEPROM.write(LOCATION_SPLASH_ONOFF, settingSplashEnable);
  }

  if(settingSplashEnable)
  {
    //Look up user content from memory
    byte content = EEPROM.read(LOCATION_SPLASH_CONTENT_LINE1);
    if(content != 0xFF)
    {
//TODO - Make this work with 20 character displays as well
      for(byte x = 0 ; x < 16 ; x++)
        SerLCD.write(EEPROM.read(LOCATION_SPLASH_CONTENT_LINE1 + x));

      SerLCD.setCursor(0, 1); //First position, 2nd row

      for (byte x = 0 ; x < 16 ; x++)
        SerLCD.write(EEPROM.read(LOCATION_SPLASH_CONTENT_LINE2 + x));

    }
    else
    {
      //Display the default
      //This should work with both 16 and 20 character displays
      SerLCD.setCursor(0, 0); //First position, 1st row
      SerLCD.print("SparkFun.com");
      SerLCD.setCursor(0, 1); //First position, 2nd row
      SerLCD.print("SerLCD v3");
    }

    delay(1000); //Hold spash screen for an amount of time
    SerLCD.clear(); //Trash the splash
    SerLCD.setCursor(0, 0); //Reset cursor
  }

}

//Check to see if we need an emergency UART reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then reset to 9600bps
void checkEmergencyReset(void)
{
  byte rxPin = 0; //The RX pin is zero
  
  pinMode(rxPin, INPUT_PULLUP); //Turn the RX pin into an input with pullups

  if(digitalRead(rxPin) == HIGH) return; //Quick pin check

  //Wait 2 seconds, blinking backlight while we wait
  pinMode(BL_RW, OUTPUT);
  digitalWrite(BL_RW, HIGH); //Set the STAT2 LED
  for(byte i = 0 ; i < 80 ; i++)
  {
    delay(25);

    //Blink backlight
    if(digitalRead(BL_RW))
      digitalWrite(BL_RW, LOW);
    else
      digitalWrite(BL_RW, HIGH);

    if(digitalRead(rxPin) == HIGH) return; //Check to see if RX is not low anymore
  }		

  //If we make it here, then RX pin stayed low the whole time
  EEPROM.write(LOCATION_BAUD, DEFAULT_BAUD); //Reset baud rate
  
  SerLCD.clear();
  SerLCD.print("9600bps reset");
  SerLCD.setCursor(0, 1); //First position, 2nd row
  SerLCD.print("Power cycle me");

  //Now sit in forever loop indicating system is now at 9600bps
  digitalWrite(BL_RW, HIGH);
  while(1)
  {
    delay(500);

    //Blink backlight
    if(digitalRead(BL_RW))
      digitalWrite(BL_RW, LOW);
    else
      digitalWrite(BL_RW, HIGH);
  }
}

