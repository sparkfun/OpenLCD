//Hardware pin definitions
#define LCD_RS A0
#define LCD_RW A1
#define LCD_EN A2
#define LCD_D4 A3
#define LCD_D5 2
#define LCD_D6 3
#define LCD_D7 4

#define  BL_RW 5
#define  BL_G  6
#define  BL_B  9

#define SPI_CS 10
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK 13

//Define the different baud rate levels
#define BAUD_2400	0
#define BAUD_4800	1
#define BAUD_9600	2
#define BAUD_14400	3
#define BAUD_19200	4
#define BAUD_38400	5
#define BAUD_57600	6
#define BAUD_76800	7
#define BAUD_115200	8

const byte DEFAULT_TWI_ADDRESS = 0x72; //0x71 for Serial7Segment. 0x72 for SerLCD.
const byte DEFAULT_BAUD = BAUD_9600;  //9600 for 8MHz, 2x speed
const byte DEFAULT_BRIGHTNESS = 100;  //100%, full brightness
const byte DEFAULT_LINES = 2;
const byte DEFAULT_WIDTH = 16;
const byte DEFAULT_SPLASH = 1; //Default on

//Internal EEPROM locations for the user settings
#define LOCATION_BAUD 0
#define LOCATION_TWI 1
#define LOCATION_SPLASH_ONOFF 2
#define LOCATION_LINES 3
#define LOCATION_WIDTH 4
#define LOCATION_RED_BRIGHTNESS 5
#define LOCATION_GREEN_BRIGHTNESS 6
#define LOCATION_BLUE_BRIGHTNESS 7
#define LOCATION_IGNORE_RX 8
#define LOCATION_TWI_ADDRESS 9
#define LOCATION_SPLASH_CONTENT_LINE1 100
#define LOCATION_SPLASH_CONTENT_LINE2 120

const int BUFFER_SIZE = 64;

//Global setting variables
byte settingLCDwidth;
byte settingLCDlines;
byte settingBLred;
byte settingSplashEnable;
byte settingUARTSpeed;

