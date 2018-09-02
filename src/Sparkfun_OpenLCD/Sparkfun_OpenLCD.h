#ifndef SPARKFUN_OPEN_LCD_H
#define SPARKFUN_OPEN_LCD_H

#include <Arduino.h>
#include <Wire.h>
#include <Stream.h>
#include <SPI.h>

//Uncomment next line to use the RGB backlight command in newer firmware, comment next line if using originl firmware without command
#define FIRMWARE_HAS_RGB_COMMAND

#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD
#define MAX_ROWS      	  4
#define MAX_COLUMNS  	 20

//OpenLCD command characters
#define SPECIAL_COMMAND  254  //Magic number for sending a special command
#define SETTING_COMMAND  0x7C //124, |, the pipe character: The command to change settings: baud, lines, width, backlight, splash, etc

//OpenLCD commands
#define CLEAR_COMMAND    0x2D //45, -, the dash character: command to clear and home the display
#define CONTRAST_COMMAND 0x18 //Command to change the contrast setting
#define ADDRESS_COMMAND  0x19 //Command to change the i2c address
#define SET_RGB_COMMAND  0x2B //43, +, the plus character: command to set backlight RGB value


// special commands
#define LCD_RETURNHOME 		0x02
#define LCD_ENTRYMODESET 	0x04
#define LCD_DISPLAYCONTROL	0x08
#define LCD_CURSORSHIFT 	0x10
#define LCD_SETDDRAMADDR 	0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT			0x00
#define LCD_ENTRYLEFT 			0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON	0x04
#define LCD_DISPLAYOFF	0x00
#define LCD_CURSORON 	0x02
#define LCD_CURSOROFF	0x00
#define LCD_BLINKON 	0x01
#define LCD_BLINKOFF 	0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

class Sparkfun_OpenLCD : public Print {

public:
	Sparkfun_OpenLCD();
	~Sparkfun_OpenLCD();
	void begin(TwoWire &wirePort);
	void begin(TwoWire &wirePort, byte i2c_addr);
	void begin(Stream &serial);
	void begin(SPIClass &spiPort, byte csPin);
//Only available for Arduino 1.6 and greater
#ifdef SPI_HAS_TRANSACTION
    //pass SPISettings by value to allow settings object creation in fucntion call like examples
    void begin(SPIClass &spiPort, byte csPin, SPISettings spiSettings);
#endif
	void clear();
	void home();
	void setCursor(byte col, byte row);
	void createChar(byte location, byte charmap[]);
	void writeChar(byte location);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buffer, size_t size);
    virtual size_t write(const char *str);
	void noDisplay();
	void display();
	void noCursor();
	void cursor();
	void noBlink();
	void blink();
	void scrollDisplayLeft();
	void scrollDisplayRight();
	void scrollDisplayLeft(byte count);
	void scrollDisplayRight(byte count);
	void moveCursorLeft();
	void moveCursorRight();
	void moveCursorLeft(byte count);
	void moveCursorRight(byte count);
	void setBacklight(unsigned long rgb);
	void setBacklight(byte r, byte g, byte b);
	void leftToRight();
	void rightToLeft();
	void autoscroll();
	void noAutoscroll();
	void setContrast(byte new_val);
	void setAddress(byte new_addr);
	void command(byte command);
	void specialCommand(byte command);
    void specialCommand(byte command, byte count);
private:
    TwoWire  *_i2cPort = NULL; //The generic connection to user's chosen I2C hardware
    Stream   *_serialPort = NULL; //The generic connection to user's chosen serial hardware
    SPIClass *_spiPort = NULL;  //The generic connection to user's chosen spi hardware

//SPI transactions only available for Arduino 1.6 and later
#ifdef SPI_HAS_TRANSACTION
    SPISettings _spiSettings = SPISettings(100000, MSBFIRST, SPI_MODE0);
    bool        _spiTransaction = false;  //since we pass by value, we need a flag
#endif
    byte  _csPin = 10;
	byte _i2cAddr = DISPLAY_ADDRESS1;
	byte _displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    byte _displayMode    = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    void init();
    void beginTransmission();
    void transmit(byte data);
    void endTransmission();
};

#endif