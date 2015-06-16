/* 
 OpenLCD System Functions

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

// setupTimer(): Set up timer 1, which controls interval reading from the buffer
//TODO - How often does this timer fire?
void setupTimer()
{
  // Timer 1 is se to CTC mode, 16-bit timer counts up to 0xFF
  TCCR1B = (1<<WGM12) | (1<<CS10);
  OCR1A = 0x00FF;
  TIMSK1 = (1<<OCIE1A);  // Enable interrupt on compare
}

//This sets up the UART with the stored baud rate in EEPROM
void setupUART()
{
  //Check to see if we are ignoring the RX reset or not
  byte settingIgnoreRX = EEPROM.read(LOCATION_IGNORE_RX);
  if(settingIgnoreRX > 1)
  {
    settingIgnoreRX = false; //Don't ignore
    EEPROM.write(LOCATION_IGNORE_RX, settingIgnoreRX);
  }
    
  if(settingIgnoreRX == false) //If we are NOT ignoring RX, then
    checkEmergencyReset(); //Look to see if the RX pin is being pulled low
  
  //Read what the current UART speed is from EEPROM memory
  //Default is 9600
  settingUARTSpeed = EEPROM.read(LOCATION_BAUD);
  if (settingUARTSpeed > BAUD_1000000) //Check to see if the baud rate has ever been set
  {
    settingUARTSpeed = DEFAULT_BAUD; //Reset UART to 9600 if there is no baud rate stored
    EEPROM.write(LOCATION_BAUD, settingUARTSpeed);
  }

  //Initialize the UART
  Serial.begin(lookUpBaudRate(settingUARTSpeed));
}

//This sets up the contrast
void setupContrast()
{
  //Read what the current contrast is, default is 200
  byte settingContrast = EEPROM.read(LOCATION_CONTRAST);
  if (settingContrast == 255) //Check to see if the baud rate has ever been set
  {
    settingContrast = DEFAULT_CONTRAST; //Default contrast to 200
    EEPROM.write(LOCATION_CONTRAST, settingContrast);
  }
  
  //Set contrast pin
  pinMode(LCD_CONTRAST, OUTPUT);
  analogWrite(LCD_CONTRAST, settingContrast);
}


// setupSPI(): Initialize SPI, sets up hardware pins and enables spi and receive interrupt
// SPI is set to MODE 0 (CPOL=0, CPHA=0), slave mode, LSB first
void setupSPI()
{
  pinMode(SPI_SCK, INPUT);
  pinMode(SPI_MOSI, INPUT);
  pinMode(SPI_CS, INPUT); //There is a 10k pull up on the SS pin

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
  
//TODO test this
  //Check the display jumper
  //If the jumper is set, use it
  pinMode(SIZE_JUMPER, INPUT_PULLUP);
  if(digitalRead(SIZE_JUMPER) == LOW)
  {
    settingLCDlines = 4;
    settingLCDwidth = 20;
  }

  SerLCD.begin(settingLCDwidth, settingLCDlines); //Setup the width and lines for this LCD

  //Clear any characters in the frame buffer
  clearFrameBuffer();
}

//Look up and start the 3 backlight pins in analog mode
void setupBacklight()
{
  pinMode(BL_RW, OUTPUT);
  pinMode(BL_G, OUTPUT);
  pinMode(BL_B, OUTPUT);
  
  //By default EEPROM is 255 or 100% brightness
  //Because it's PNP transistor we need to invert the logic (or subtract the user value from 255)
  analogWrite(BL_RW, 255 - EEPROM.read(LOCATION_RED_BRIGHTNESS));
  analogWrite(BL_G, 255 - EEPROM.read(LOCATION_GREEN_BRIGHTNESS));
  analogWrite(BL_B, 255 - EEPROM.read(LOCATION_BLUE_BRIGHTNESS));
}

void setupSplash()
{

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
    byte content = EEPROM.read(LOCATION_SPLASH_CONTENT);

    if(content == 0xFF)
    {
      //Display the default splash screen
      //This should work with both 16 and 20 character displays
      SerLCD.clear();
      SerLCD.setCursor(0, 0); //First position, 1st row
      SerLCD.print("SparkFun");
      SerLCD.setCursor(0, 1); //First position, 2nd row
      SerLCD.print("    OpenLCD");
    }
    else
    {
      //Pull splash content from EEPROM
      
      //Copy the EEPROM to the character buffer
      for(byte x = 0 ; x < settingLCDlines * settingLCDwidth ; x++)
        currentFrame[x] = EEPROM.read(LOCATION_SPLASH_CONTENT + x);
  
      //Now display the splash
      displayFrameBuffer();
    }

    //While we hold the splash screen monitor for incoming serial
    Serial.begin(9600); //During this period look for characters at 9600bps
    for(byte x = 0 ; x < (SYSTEM_MESSAGE_DELAY/10) ; x++)
    {
      //Reverse compatiblity with SerLCD 2.5: a ctrl+r during splash will reset unit to 9600bps.
      if(Serial.available())
      {
        if(Serial.read() == 18) //ctrl+r
        {
          //Reset baud rate
          SerLCD.clear();
          SerLCD.setCursor(0, 0); //First position, 1st row
          
          SerLCD.print("Baud Reset");
          
          EEPROM.write(LOCATION_BAUD, BAUD_9600);

          petSafeDelay(SYSTEM_MESSAGE_DELAY);
          
          break;
        }
      } //This assumes that Serial.begin() will happen later
      
      //serialEvent(); //Check the serial buffer for new data
      petSafeDelay(10); //Hang out looking for new characters
    }

    //Now erase it and the buffer
    clearFrameBuffer();

    SerLCD.clear(); //Trash the splash
    SerLCD.setCursor(0, 0); //Reset cursor
    
    //After this function we go back to system baud rate
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
    wdt_reset(); //Pet the dog
    delay(25);

    //Blink backlight
    if(digitalRead(BL_RW))
      digitalWrite(BL_RW, LOW);
    else
      digitalWrite(BL_RW, HIGH);

    if(digitalRead(rxPin) == HIGH) return; //Check to see if RX is not low anymore
  }		

  //If we make it here, then RX pin stayed low the whole time
  //Reset all EEPROM locations to factory defaults.
  for(int x = 0 ; x < 200 ; x++)
    EEPROM.write(x, 0xFF);
  
  SerLCD.clear();
  SerLCD.print("System reset");
  SerLCD.setCursor(0, 1); //First position, 2nd row
  SerLCD.print("Power cycle me");

  //Now sit in forever loop indicating system is now at 9600bps
  digitalWrite(BL_RW, HIGH);
  while(1)
  {
    petSafeDelay(500);

    //Blink backlight
    if(digitalRead(BL_RW))
      digitalWrite(BL_RW, LOW);
    else
      digitalWrite(BL_RW, HIGH);
  }
}

//We store the baud rate as a single digit in EEPROM
//This function converts the byte to the actual baud rate
long lookUpBaudRate(byte setting)
{
  switch(setting)
  {
    case BAUD_2400: return(2400);
    case BAUD_4800: return(4800);
    case BAUD_9600: return(9600);
    case BAUD_14400: return(14400);
    case BAUD_19200: return(19200);
    case BAUD_38400: return(38400);
    case BAUD_57600: return(57600);
    case BAUD_115200: return(115200);
    case BAUD_230400: return(230400);
    case BAUD_460800: return(460800);
    case BAUD_921600: return(921600);
    case BAUD_1000000: return(1000000);
  }
 
}

//Delays for a specified period that is pet safe
void petSafeDelay(int delayAmount)
{
  long startTime = millis();
  
  while(millis() - startTime < delayAmount)
  {
    wdt_reset(); //Pet the dog
    
    //Max 100ms delay
    for(byte x = 0 ; x < 100 ; x++)
    {
      delay(1);
      if( (millis() - startTime) >= delayAmount) break;
    }
  }

  wdt_reset(); //Pet the dog
}

