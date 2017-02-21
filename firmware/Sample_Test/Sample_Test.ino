/*
 OpenLCD is an LCD with serial/i2c/spi interfaces.
 By: Nathan Seidle
 SparkFun Electronics
 Date: February 13th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 OpenLCD gives the user multiple interfaces (serial, I2C, and SPI) to control an LCD. SerLCD was the original
 serial LCD from SparkFun that ran on the PIC 16F88 with only a serial interface and limited feature set.
 This is an updated serial LCD.

 Select 'Arduino Pro or Pro Mini' as the board. Processor: 'ATmega328 (3.3V / 8MHz)'

 Backlight levels from original datasheet are wrong. Setting of 22 is 76%. See google doc

*/

#include <LiquidCrystalFast.h> //Faster LCD commands. From PJRC https://www.pjrc.com/teensy/td_libs_LiquidCrystal.html
//#include <LiquidCrystal.h> //
#include "settings.h" //Defines EEPROM locations for user settings

LiquidCrystalFast SerLCD(LCD_RS, LCD_RW, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup()
{
  delay(100);

  Serial.begin(57600);

  pinMode(7, OUTPUT);
  analogWrite(7, 0);
  delay(1);
  analogWrite(7, 255);

  SerLCD.begin(16, 2); //Init data and control pins to display

  delay(1);

  //Magic blob from mfg
  
  SerLCD.command(0x2A); //Command: Function Set, set extension register (RE)
  SerLCD.command(0x71); //Command: Function Selection A
  SerLCD.write(0x5C); //Enable internal Vdd regulator (5V I/O)
  SerLCD.command(0x28); //Command: Function Set, clear extension register (RE)

  SerLCD.command(0x08); //Command: Display ON/OFF Control. Display off, cursor off, blink off

  //Set Oscillator frequency
  SerLCD.command(0x2A); //Command: Function Set, set extension register (RE)
  SerLCD.command(0x79); //Command: OLED Characterization, OLED command set is enabled
  SerLCD.command(0xD5); //OLED Command: Set Display Clock Divide Ratio/Oscillator Frequency
  SerLCD.command(0x70); //(POR) Default oscillator freq, set divide ratio to 1
  SerLCD.command(0x78); //Command: OLED Characterization, OLED command set is disabled
  SerLCD.command(0x28); //Command: Function Set, clear extension register (RE)
  
  //Set ROM and CHRAM settings
  SerLCD.command(0x2A); //Command: Function Set, set extension register (RE)
  SerLCD.command(0x08); //Extended Function Set: NW = 0 for 2-lines
  SerLCD.command(0x06); //Entry Mode Set: BDC = 1, BDS = 0
  SerLCD.command(0x72); //Function Selection B - Selecting ROM and CGROM
  SerLCD.write(0x08); //ROM and CGROM Selection: ROM 'C' with 240 CGROM
  SerLCD.command(0x28); //Command: Function Set, clear extension register (RE)

  //Setup contrast, external VSL, etc
  SerLCD.command(0x2A); //Command: Function Set, set extension register (RE)
  SerLCD.command(0x79); //Command: OLED Characterization, OLED command set is enabled
  SerLCD.command(0xDA); //Set SEG Pins Hardware Configuration
  SerLCD.command(0x10); //Set SEG Pins Hardware Configuration: Disable SEG left/right, Alternative SEG pin config
  SerLCD.command(0xDC); //Set VSL/GPIO: function selection C
  SerLCD.command(0x00); //Function Select C: Disable external VSL, disable GPIOs
  SerLCD.command(0x81); //Set Contrast Control
  SerLCD.command(0xFF); //Set Contrast Control: 0 to 255
  SerLCD.command(0xD9); //Set Phase Length
  SerLCD.command(0xF1); //Set Phase Length: Phase 2 = 15, Phase 1 = 1
  SerLCD.command(0xDB); //Set VCOMH Deselect Level
  SerLCD.command(0x00); //Set VCOMH Deselect Level: 0.65 x VCC
  SerLCD.command(0x78); //Command: OLED Characterization, OLED command set is disabled
  SerLCD.command(0x28); //Command: Function Set, clear extension register (RE)

  SerLCD.command(0x01); //clear display
  SerLCD.command(0x0C); //Command: Display ON/OFF Control. Display on, cursor off, blink off  
  //SerLCD.command(0x0D); //Command: Display ON/OFF Control. Display on, blink on  
  
  SerLCD.setCursor(0, 0); //First spot
    
  SerLCD.print("hello");
  while (1)
  {
    for(int x = 0 ; x < 255 ; x += 5)
    {
      changeContrast(x);
      delay(500);
    }
  }

}

void changeContrast(byte contrast)
{

    SerLCD.command(0x2A); //Command: Function Set, set extension register (RE)
    SerLCD.command(0x79); //Command: OLED Characterization, OLED command set is enabled
    SerLCD.command(0x81); //Set Contrast Control
    SerLCD.command(contrast); //Set Contrast Control: 0 to 255
    SerLCD.command(0x78); //Command: OLED Characterization, OLED command set is disabled
    SerLCD.command(0x28); //Command: Function Set, clear extension register (RE)

  //Display the new contrast
  SerLCD.clear();
  SerLCD.setCursor(0, 0); //First position, 1st row
  SerLCD.print("Contrast Set");
  SerLCD.setCursor(0, 1); //First position, 2nd row
  SerLCD.print(contrast);

}


void loop()
{
  delay(10);
}

//Comes from http://playground.arduino.cc/Code/PwmFrequency
//Allows us to set the base PWM freq for various PWM pins
//Pins 3, 9, 10, and 11 has base freq of 31250 Hz
//Pins 5, 6 has base freq of 62500 Hz
//Note that this function will have side effects on anything else that uses timers:
//Changes on pins 3, 5, 6, or 11 may cause the delay() and millis() functions to stop working. Other timing-related functions may also be affected.
//Changes on pins 9 or 10 will cause the Servo library to function incorrectly.
void setPwmFrequency(int pin, int divisor)
{
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

