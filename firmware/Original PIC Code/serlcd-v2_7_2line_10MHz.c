/*
    1-16-04
    Copyright Spark Fun Electronics© 2004
    
    Uses 16F688 to drive Parallel LCD with Serial Commands.
    
    For educational and hobbyist use only. NOT for commerical use.


    Thanks to Olimex for providing the original C test code for the PIC-MT-C w/ 16F873
    
    1-17-04 Started migrating 16F873A code to 16F630


    1-17-04pm Works well from user keyboard input. No busy flag checking yet.
              Ok, busy flag works.


    1-19-04 Got commands to pass through after decimal 254 is seen. The special 8-bit data
            mode command is also correctly ignored.
            
    1-30-04 v1.1 Added 16x4 line support
    
    2-10-04 v1.2 Added BL Control. Moved to direct RC0-RC3 = D4-D7 mapping. This significantly
            improved timing between characters.
    
    2-12-04 Added EEPROM recording of Backlight State. Also added Super Special
            Command input. Pass the '|' character to the LCD, then pass '1' to turn
            on/off backlight.
            
            Added universal support for any LCD upto 4 lines, 40 characters wide.
            
    2-16-04 New PCBs v1.1 - Replaced some if statements with EL1, EL2, etc, in the 4 line wrapping 
            routine to try to speed things up.
            
    2-17-04 v1.3 Timing has now been nearly reduced to within one stop bit. In the past, 
            wrapping characters were some times lost because of the time required to relocate 
            the cursor. Now almost all characters are seen, every time, every location. 
            There are stuff certain combinations that cause a character miss.
    
    3-3-04 New PCB - 1.1a. BL Control has been routed to PGC - bad idea. Causes problems with programming.
           Unit must be powered +5V while programming to get it to work. Will move this line on next revision.
           
    4-5-04 Customer reported problems with interface. Baud rate was too low. The test bed was running 
           at much too low of a speed. The test bed has been corrected, the code must now be tweaked.
           
    4-14-04 Added a special command 8 that will re-initialize the LCD. There are certain circumstances (low
            power) where the PIC will keep running, but the LCD will shut down. 


    6-1-04 Re-routed some pins for a smoother PCB layout. Firmware had to be updated.


    7-7-04 Added support for 40x2 displays
    
    12-15-04 New version - SerLCD v2 using 16F688
             Incoming characters buffered
             PWM of back light
             Turning off splash screen
             Higher data speeds
             
             We now use the 16F688 with onboard UART and internal 8MHz oscillator. The internal oscillator is much
             tighter tolerance and the UART interrupt is used. The incoming characters are automatically buffered,
             up to 80 characters can be stored at a time in case the LCD is busy doing other things.
             
             The backlight is now pulse width modulated. What this means is you can set how bright the backlight is
             from 0 (off) to 30 (completely on). This will greatly effect how much current the module uses while 
             retaining part of the ability to see in low light conditions. The backlight defaults to maximum 
             output (brightest) unless otherwise configured by the user. To control the backlight, send the 'special
             command' character (124) followed by 0b.100?.???? where ?? is the value from 0-30.
             
             You can now turn off the splash screen. Send the 'special command' character 124 followed by decimal 9.
             The splash screen option will toggle and will take effect the next time the LCD is initialized (power-up).
    
    3-30-05  Added busy checking before the LCD is init'd. SPBRG is set to 51 per datasheet for 16F688.
             Extended the delays a bit during LCD init to try to avoid hangups with 3% of units that report hanging.
    
    4/29/09  Changed code for integrated LCD with backpack, outsourced hardware. Control lines to LCD from PIC have changed.
    
*/


#define Clock_10MHz
#define Baud_9600
//#define DEBUG


#include "C:\Global\PIC\C\16F88.h"  // device dependent interrupt definitions
#include "C:\Global\PIC\C\int16CXX.H"



#pragma config|= 0x3F02
//#pragma config|= 0x2F8A   //10MHz xtal
//#pragma config|= 0x29D3     //int RC


//LCD Type
#define Display_16x2    0
#define Display_16x4    0


//Hardware port definitions
#define     RS              PORTB.4
#define     R_W             PORTB.1  
#define     BL_Control      PORTA.4
#define     E               PORTB.0

//#define     STAT            PORTA.4


#define     D4              PORTA.0
#define     D5              PORTA.1
#define     D6              PORTA.2
#define     D7              PORTA.3


//Constant definitions
#define     FALSE       0
#define     TRUE        1


#define     LCD_BACKLIGHT_SETTING   0
#define     LCD_WIDTH_SETTING       1
#define     LCD_LINE_SETTING        2
#define     LCD_BRIGHTNESS_SETTING  3
#define     LCD_SPLASH_ENABLE       4
#define     LINE1_START             5
#define     LINE2_START             25
#define     BAUD_RATE               45




#define     CLR_DISP        0b.0000.0001 //Clear display


#define     CUR_HOME        0b.0000.0010    //Move cursor home and clear screen memory
#define     CUR_RIGHT       0b.0001.0100    //Move cursor one to right
#define     CUR_LEFT        0b.0001.0000    //Move cursor one to left


#define     SCROLL_RIGHT    0b.0001.1100    //Scroll entire screen right one space
#define     SCROLL_LEFT     0b.0001.1000    //Scroll entire screen left one space


#define     DISP_ON         0b.0000.1100    //Turn visible LCD on
#define     DISP_OFF        0b.0000.1000    //Turn visible LCD off


#define     UNDERLINE_ON    0b.0000.1110    //Turn on underlined cursor
#define     UNDERLINE_OFF   0b.0000.1100    //Turn off underlined cursor


#define     BLINKCUR_ON     0b.0000.1101    //Turn on blinking box cursor
#define     BLINKCUR_OFF    0b.0000.1100    //Turn off blinking box cursor


#define     DUALCUR_ON      0b.0000.1111    //Turn on blinking box and underline cursor
#define     DUALCUR_OFF     0b.0000.1100    //Turn off blinking box and underine cursor


#define     SET_CURSOR      0b.1000.0000    //SET_CURSOR + X : Sets cursor position to X


#define     ENTRY_INC       0b.0000.0110 //
#define     DD_RAM_ADDR     0b.1000.0000 //
#define     DD_RAM_ADDR2    0b.1100.0000 //
#define     CG_RAM_ADDR     0b.0100.0000 //


//Global variable declarations
uns8 letter;
static uns8 cursor_position;
uns8 splash_enable;


uns8 backlight_counter;
uns8 brightness_setting;
uns8 Backlight_Setting;


uns8 LCD_Type;
uns8 LCD_Width;
uns8 LCD_Lines;
uns8 EL1, EL2, EL3, EL4;
uns8 CP1, CP2, CP3, CP4;
uns8 baud;


uns8 RX_In;
uns8 Displayed_Letters;


#pragma rambank 1
#define BUFFER_SIZE 80
uns8 RX_Array[BUFFER_SIZE];

#pragma rambank 0
uns8 line1[20];
uns8 line2[20];


//End Global variable declarations



//Interrupt Vectors
#pragma origin 4
interrupt serverX( void)
{
    int_save_registers
    char sv_FSR = FSR;  // save FSR if required


    if(RCIF) //If we have received something from the computer store it in the RX_Array array
    {
        
        RX_In++;
        if(RX_In == BUFFER_SIZE) RX_In = 0;
        
        RX_Array[RX_In] = RCREG;
    }


    if(T0IF) //TMR0 Overflow Interrupt - Occurs every 1024us ~1ms
    {
        backlight_counter++;


        if(backlight_counter == 30)
        {
            backlight_counter = 0; //30ms pulse width period
            
            if (brightness_setting > 0) BL_Control = 0; //Kick on the back light only if brightness setting is greater than 0
        }    


        //Turn off back light after its assigned duration
        if(backlight_counter == brightness_setting) BL_Control = 1; 
        
        T0IF = 0; //Clear Interrupt Flag
    }
    
    FSR = sv_FSR;               // restore FSR if saved
    int_restore_registers 
}


//Function declarations
void boot_up(void);
void special_commands(void);
void delay_ms(uns16);
void delay_ms(uns16);
uns8 onboard_eeread(uns8 e_address);
void onboard_eewrite(uns8 e_data, uns8 e_address);
void set_baud_rate(void);


void send_char(uns8);
void send_cmd(uns8);
void send_string(const char* incoming_string);
void LCD_wait(void);
void init_lcd(void);


//#include "C:\Global\PIC\C\stdio.c"

//End Function declarations


void main(void)
{
    //Initialize LCD and PIC
    boot_up();
    
    while(1)
    {
        //sleep(); //Wait for incoming command to wake us up...
        //nop(); //Executes after wake-up and before INTF runs
        
        if(RX_In != Displayed_Letters)
        {
            Displayed_Letters++;
            if(Displayed_Letters == BUFFER_SIZE) Displayed_Letters = 0;


            letter = RX_Array[Displayed_Letters];



            //Check for special LCD command
            //===============================================================
            if (letter == 254)
            {
                //Wait for the next letter
                while(RX_In == Displayed_Letters);
                
                //Look at the next letter
                Displayed_Letters++;
                if(Displayed_Letters == BUFFER_SIZE) Displayed_Letters = 0;


                letter = RX_Array[Displayed_Letters];
                
                //Ignore the one command that will send the LCD into 8-bit mode
                if ( (letter >> 4) != 3 ) //If not 0b.0000.0011, then send it to LCD
                    send_cmd(letter);
                    
                //Correct cursor position variable if certain commands are received
                if (letter == CLR_DISP)
                    cursor_position = 0;
                else if (letter.7 == 1) //Move cursor command
                    cursor_position = letter & 0b.0111.1111; //Ignore first bit - obtain address
            }    
            //===============================================================


            //Super Special LCD Commands
            //===============================================================
            else if (letter == 124) //This is the '|' character above '\'
            {
                //Wait for the next letter
                while(RX_In == Displayed_Letters);


                //Look at the next letter
                Displayed_Letters++;
                if(Displayed_Letters == BUFFER_SIZE) Displayed_Letters = 0;


                letter = RX_Array[Displayed_Letters];


                special_commands();
            }
            //===============================================================
            
            
            
            //All else, just print it to the LCD and adjust the cursor position
            //===============================================================
            else
            {   
                #ifdef DEBUG  
                    if (letter == '+') brightness_setting++;
                    if (brightness_setting > 30) brightness_setting = 0;
                    
                    if( letter == 'A')
                    {
                        letter = 0b.1000.0000 | 15;
                        special_commands();
                    }
                #endif
                
                
                
                
//Backspace============================================================================                
                if (letter == 8)
                {
                    
                    if(cursor_position == (LCD_Width + 64))        //from beginning of line 4
                    {
                        //send_cmd(CP3);
                        send_cmd(CP3 + LCD_Width - 1);
                        send_char(32);
                        send_cmd(CP3 + LCD_Width - 1);
                        cursor_position = EL3 - 1;
                        
                        //printf("%d\r\n",cursor_position);
                    }
                    
                    else if(cursor_position == LCD_Width)   //from beginning of line 3
                    {
                        //send_cmd(CP2);
                        send_cmd(CP2 + LCD_Width - 1);
                        send_char(32);
                        send_cmd(CP2 + LCD_Width - 1);
                        cursor_position = EL2 - 1;
                        
                        line2[cursor_position - 64] = 32;
                        
                        //printf("%d\r\n",cursor_position);
                    }
                    
                    
                    else if(cursor_position == 64)      //from beginning of line 2
                    {
                        
                        //send_cmd(CP1);
                        send_cmd(CP1 + LCD_Width - 1);
                        send_char(32);
                        send_cmd(CP1 + LCD_Width - 1);
                        cursor_position = EL1 - 1;
                        
                        line1[cursor_position] = 32;
                        
                        //printf("%d\r\n",cursor_position);
                    
                    }
                        
                    else if (cursor_position == 0)      //from beginning of line 1
                    {
                        if(LCD_Lines == 1)
                        {
                            send_cmd(CP1 + LCD_Width - 1);
                            send_char(32);
                            send_cmd(CP1 + LCD_Width - 1);
                            cursor_position = EL1 - 1;
                            
                            //printf("%d\r\n",cursor_position);
                        }
                        
                        else if (LCD_Lines == 2)                      
                        {
                            //send_cmd(CP2);
                            send_cmd(CP2 + LCD_Width - 1);
                            send_char(32);
                            send_cmd(CP2 + LCD_Width - 1);
                            cursor_position = EL2 - 1;
                            
                            //printf("%d\r\n",cursor_position);
                        }
                        
                        else    //4 line display
                        {
                            //send_cmd(CP4);
                            send_cmd(CP4 + LCD_Width - 1);
                            send_char(32);
                            send_cmd(CP4 + LCD_Width - 1);
                            cursor_position = EL4 - 1;
                            
                            //printf("%d\r\n",cursor_position);
                        }
                    }
                    
                    else
                    {
                        send_cmd(CUR_LEFT);
                        send_char(32);
                        send_cmd(CUR_LEFT);
                        cursor_position--;
                        
                        //make sure that the line arrays are correct
                        if (cursor_position < 16) line1[cursor_position] = 32;
                        if ((cursor_position > 63) && (cursor_position < 80)) line2[cursor_position - 64] = 32;
                        
                        //printf("%d\r\n",cursor_position);
                    }
                }
                
                
//Regular character send==========================================================================
                else
                {
                    send_char(letter);
                    
                    //keep the first 2 lines logged for splash screen changes
                    if (cursor_position < 16) line1[cursor_position] = letter;
                    if ((cursor_position > 63) && (cursor_position < 80)) line2[cursor_position - 64] = letter;
    
                    cursor_position++;
                    
                    
                    //When the cursor gets to the end of one line, it must
                    //advance to the next visual line
                    //===============================================================
                    if (cursor_position == EL1) //End of line one
                    {
                        if(LCD_Lines == 1)
                        {
                            cursor_position = 0; //Return to beginning of line 1
                            send_cmd(CP1);
                        }
                        else
                        {
                            cursor_position = 64; //Beginning of line 2
                            send_cmd(CP2);
                        }
                    }
                    
                    else if (cursor_position == EL2) //End of line 2
                    {
                        if(LCD_Lines == 2)
                        {
                            cursor_position = 0; //Return to line 1
                            send_cmd(CP1);
                        }
                        else
                        {
                            cursor_position = LCD_Width; //Beginning of line 3
                            send_cmd(CP3);
                        }
                    }
                    
                    else if (cursor_position == EL3) //End of line 3
                    {
                        cursor_position = EL2; //Beginning of line 4 is the end of line 2
                        send_cmd(CP4);
                    }
                    
                    else if (cursor_position == EL4) //End of line 4
                    {
                        cursor_position = 0; //Beginning of line 1
                        send_cmd(CP1);
                    }
                    
                    
                
                }
                //===============================================================
            }
            //===============================================================
                


        }//End regular character printing
            
   }//End infinite loop


}//End Main


//This is called when the control character is received.
//The control character is 124 or '|' (above the '\' on standard US layout keyboards)
void special_commands(void)
{
    uns8 x;

    //Backlight control
    if (letter == 1)
    {
        brightness_setting = 30;
        onboard_eewrite(brightness_setting, LCD_BRIGHTNESS_SETTING); //Record to EEPROM
        //Backlight_Setting = 0; //Turn on backlight setting
        //BL_Control = Backlight_Setting; //Turn on backlight
        //onboard_eewrite(Backlight_Setting, LCD_BACKLIGHT_SETTING); //Record to EEPROM
    }
    else if(letter == 2)
    {
        brightness_setting = 0;
        onboard_eewrite(brightness_setting, LCD_BRIGHTNESS_SETTING); //Record to EEPROM
        //Backlight_Setting = 1; //Turn off backlight setting
        //BL_Control = Backlight_Setting; //Turn off backlight
        //onboard_eewrite(Backlight_Setting, LCD_BACKLIGHT_SETTING); //Record to EEPROM
    }
    
    //LCD type commands
    else if(letter == 3)
    {
        LCD_Width = 20;
        onboard_eewrite(LCD_Width, 1);
    }
    else if(letter == 4)
    {
        LCD_Width = 16;
        onboard_eewrite(LCD_Width, 1);
    }
    else if(letter == 5)
    {
        LCD_Lines = 4;
        onboard_eewrite(LCD_Lines, 2);
    }
    else if(letter == 6)
    {
        LCD_Lines = 2;
        onboard_eewrite(LCD_Lines, 2);
    }
    else if(letter == 7)
    {
        LCD_Lines = 1;
        onboard_eewrite(LCD_Lines, 2);
    }
    else if(letter == 8) //Re-init command
    {
        init_lcd();
    }


    //Turn on/off splash screen
    else if(letter == 9)
    {
        if (splash_enable == 1) splash_enable = 0;
        else splash_enable = 1;
        
        onboard_eewrite(splash_enable, LCD_SPLASH_ENABLE); //Record to EEPROM
    }
    
    //Save alternate splash screen from line arrays (first two lines)
    else if(letter == 10)   //^j
    {
        for (x = 0; x < 16; x++)
        {
            onboard_eewrite(line1[x], LINE1_START + x); //Record to EEPROM
            onboard_eewrite(line2[x], LINE2_START + x);
        }
        
            
    }
    
    //set baud rate to 2400
    else if(letter == 11)   //^k
    {
        baud = 0;
        set_baud_rate();
        onboard_eewrite(baud, BAUD_RATE);
            
    }
    
    
    //set baud rate to 4800
    else if(letter == 12)   //^l
    {
        baud = 1;
        set_baud_rate();
        onboard_eewrite(baud, BAUD_RATE);
            
    }
    
    
    //set baud rate to 9600
    else if(letter == 13)   //^m
    {
        baud = 2;
        set_baud_rate();
        onboard_eewrite(baud, BAUD_RATE);
            
    }
    
    
    //set baud rate to 14400
    else if(letter == 14)   //^n
    {
        baud = 3;
        set_baud_rate();
        onboard_eewrite(baud, BAUD_RATE);
            
    }
    
    
    //set baud rate to 19200
    else if(letter == 15)   //^o
    {
        baud = 4;
        set_baud_rate();
        onboard_eewrite(baud, BAUD_RATE);
            
    }
    
    
    //set baud rate to 38400
    else if(letter == 16)   //^p
    {
        baud = 5;
        set_baud_rate();
        onboard_eewrite(baud, BAUD_RATE);
            
    }


    //Back light brightness settings
    else if(letter >= 128)
    {
        brightness_setting = letter & 0b.0001.1111; //Brightness is a value from 0-29 only (5-bit)
        onboard_eewrite(brightness_setting, LCD_BRIGHTNESS_SETTING); //Record to EEPROM
    }
    
    
    //LCD_Width is the number of seen characters
    //EL1 is End Line 1 - Normally LCD_Width
    //EL2 is End Line 2 - Normally 64 + LCD_Width
    //EL3 is End Line 3 - LCD_Width + LCD_Width
    //EL4 is End Line 4 - 64 + LCD_Width + LCD_Width


    EL1 = LCD_Width;
    EL2 = 64 + LCD_Width;
    EL3 = LCD_Width + LCD_Width;
    EL4 = 64 + LCD_Width + LCD_Width;
    
    //CP1 = Cursor Position 1 - 0 Always
    //CP1 = CP1 & 0b.1000.000 - this is to set the DRAM command
    //CP2 = 64 Always = 0b.1000.0000 | 64
    //CP3 = LCD_width = 0b.1000.0000 | LCD_Width
    //CP4 = 64 + LCD_Width = 0b.1000.0000 | (64 + LCD_Width)
    
    CP1 = 0b.1000.0000 | 0;
    CP2 = 0b.1000.0000 | 64;
    CP3 = 0b.1000.0000 | LCD_Width;
    CP4 = 0b.1000.0000 | (LCD_Width + 64);
    
}


//Initializes the various ports and interrupts
//Also inits the LCD
void boot_up(void)
{
    uns8 x;
    //OSCCON = 0b.0000.1000; //Setup internal oscillator for 8MHz
    //while(OSCCON.2 == 0); //Wait for frequency to stabilize
    
    //Setup Ports
    ANSEL = 0b.0000.0000; //Disable ADC on all pins
    //CMCON0 = 0b.0000.0111; //Turn off comparator on RA port 


    PORTA = 0b.0001.0000;
    TRISA = 0b.1110.0000; //0 = Output, 1 = Input


    PORTB = 0b.0000.0000;
    TRISB = 0b.1110.1100; //0 = Output, 1 = Input (RX on RB2)


    //OPTION.7 = 0; //Enable weak pull-ups
    
    Displayed_Letters = 0;
    RX_In = 0;
    
    //Setup the hardware UART module and interrupts
    //=============================================================
    //baud = onboard_eeread(BAUD_RATE);
    //set_baud_rate();
    SPBRG = 64;

    TXSTA = 0b.0010.0100; //
    RCSTA = 0b.1001.0000; //Receive enable, 8-bit asych continous receive mode


    //BRG16 = 1;
    BRGH = 1;


    RCIF = 0;
    RCIE = 1;
    PEIE = 1;
    //=============================================================


    //Setup TMR0 and interrupts
    OPTION_REG = 0b.0101.0001; //Give Timer0 prescalar of 4
    T0IE = 1;


    GIE = 1; //Enable global interrupts
    
   //for (x = 0; x < 4; x++)
    //{
        //STAT = 0;
        //delay_ms(500);
        //STAT = 1;
        //delay_ms(1500);
    //}
    
    
    
    //Init LCD
    init_lcd();
    
}


//Initializes the 4-bit parallel interface to the HD44780
void init_lcd(void)
{
    uns8 x;
    
    //Wait for LCD busy bit to clear
    LCD_wait();
    
    RS = 0;               
    R_W = 0;


    //Tell the LCD we are using 4bit data communication
    //===============================================================
    delay_ms(100);
    //PORTB = 0b.0000.0011;
    D4 = 1; D5 = 1;
    E = 1; E = 0;


    delay_ms(50);
    //PORTB = 0b.0000.0011;
    E = 1; E = 0;


    delay_ms(10);
    //PORTB = 0b.0000.0011;
    E = 1; E = 0;


    delay_ms(10);
    //PORTB = 0b.0000.0010;
    D4 = 0;
    E = 1; E = 0;


    send_cmd(DISP_ON);
    send_cmd(CLR_DISP);
    //===============================================================
    //LCD Init Complete
    
    //Retrieve last Backlight state
    //===============================================================
    Backlight_Setting = onboard_eeread(LCD_BACKLIGHT_SETTING);
    if (Backlight_Setting > 1) Backlight_Setting = 1; //Default
    onboard_eewrite(Backlight_Setting, LCD_BACKLIGHT_SETTING); //Record to EEPROM
    BL_Control = Backlight_Setting; //Turn on/off the backlight
    //===============================================================
    
    //Retrieve backlight brightness setting
    //===============================================================
    brightness_setting = onboard_eeread(LCD_BRIGHTNESS_SETTING);
    if (brightness_setting > 30) brightness_setting = 30; //Turn LCD backlight on to brightest by default
    onboard_eewrite(brightness_setting, LCD_BRIGHTNESS_SETTING); //Record to EEPROM
    //===============================================================


    //Retrieve last LCD Type
    //===============================================================
    LCD_Width = onboard_eeread(LCD_WIDTH_SETTING);
    if (LCD_Width > 80) LCD_Width = 16; //Default
    onboard_eewrite(LCD_Width, LCD_WIDTH_SETTING); //Record to EEPROM


    //Set end lines
    EL1 = LCD_Width;
    EL2 = 64 + LCD_Width;
    EL3 = LCD_Width + LCD_Width;
    EL4 = 64 + LCD_Width + LCD_Width;


    //Set cursor positions
    CP1 = 0b.1000.0000 | 0;
    CP2 = 0b.1000.0000 | 64;
    CP3 = 0b.1000.0000 | LCD_Width;
    CP4 = 0b.1000.0000 | (LCD_Width + 64);

    
    if (Display_16x2 == 1) LCD_Lines = 2;
    else if (Display_16x4 == 1) LCD_Lines = 4;
    else
    {
        LCD_Lines = onboard_eeread(LCD_LINE_SETTING);
        if (LCD_Lines > 8) LCD_Lines = 2; //Default
        onboard_eewrite(LCD_Lines, LCD_LINE_SETTING); //Record to EEPROM
    }
    
    //===============================================================
    
    //Display splash screen if enabled
    //===============================================================
    splash_enable = onboard_eeread(LCD_SPLASH_ENABLE);
    if (splash_enable > 1) splash_enable = 1; //Default to show splash screen
    onboard_eewrite(splash_enable, LCD_SPLASH_ENABLE); //Record to EEPROM
    
    if (splash_enable == 0)
    {
        delay_ms(1000);
        
        if(RX_In > 0)
        {
            x = RX_Array[1];
            
            if (x == 18)    //^r to reset the baud rate to 9600
            {
                baud = 2;
                set_baud_rate();
                onboard_eewrite(baud, BAUD_RATE);
                
                send_string("Reset to 9600");
                delay_ms(1000);
                send_cmd(CLR_DISP);
                
            }
            
            RX_In = 0;
        }
        
        
        else
        {
            baud = onboard_eeread(BAUD_RATE);
            set_baud_rate();
        }
    }
    


    else if (splash_enable == 1)
    {
        x = onboard_eeread(LINE1_START);
        if (x != 0xFF)
        {
            for (x = 0; x < 16; x++)
            {
                send_char(onboard_eeread(LINE1_START + x));
                
            }
            
            send_cmd(SET_CURSOR + 64);
            
            for (x = 0; x < 16; x++)
            {
                send_char(onboard_eeread(LINE2_START + x));
                
            }
            
            delay_ms(1000);
            send_cmd(CLR_DISP);
        }
            
        else
        {
            send_string("  SparkFun.com");
            send_cmd(SET_CURSOR + 64);
            send_string("  SerLCD v2.5");
            delay_ms(1000);
            send_cmd(CLR_DISP);
        }
            
        
        if(RX_In > 0)
        {
            x = RX_Array[1];
            
            if (x == 18)    //^r to reset the baud rate to 9600
            {
                baud = 2;
                set_baud_rate();
                onboard_eewrite(baud, BAUD_RATE);
                
                send_string("Reset to 9600");
                delay_ms(1000);
                send_cmd(CLR_DISP);
                
            }
            
            RX_In = 0;
        }
        
        
        else
        {
            baud = onboard_eeread(BAUD_RATE);
            set_baud_rate();
        }
        
    }
    
    //===============================================================
    
    for (x = 0; x < 16; x++)
    {
        line1[x] = 32;  //init the line arrays to black spaces
        line2[x] = 32;  //
    }
    
    cursor_position = 0;
}    


//Checks the busy flag and waits until LCD is ready for next command
void LCD_wait(void)
{
    bit i = 1;
    uns8 j;
    
    TRISA |= 0b.0000.1111; //0 = Output, 1 = Input


    R_W = 1; //Tell LCD to output status
    RS = 0;               


    while(i == 1)
    {
        E = 1; 
        //for (j = 0; j < 1; j++);
        i = D7; //Read data bit 7 - Busy Flag
        E = 0;
    
        E = 1;
        //for (j = 0; j < 1; j++);
        E = 0; //Toggle E to get the second four bits of the status byte - but we don't care
    }
    
    TRISA &= 0b.1111.0000; //0 = Output, 1 = Input
    
    //STAT = 0;
    
    //while (1);
}


//Sends an ASCII character to the LCD
void send_char(uns8 c)
{
    LCD_wait();
    
    R_W = 0; //set LCD to write
    RS = 1; //set LCD to data mode
    
    //PORTC = c >> 4;
    D4 = c.4; D5 = c.5; D6 = c.6; D7 = c.7;
    E = 1; E = 0;
    
    //PORTC = c;
    D4 = c.0; D5 = c.1; D6 = c.2; D7 = c.3;
    E = 1; E = 0; //Toggle the Enable Pin
}


//Sends an LCD command
void send_cmd(uns8 d)
{
    LCD_wait();


    //TRISC = 0b.0000.0000;   //0 = Output, 1 = Input


    R_W = 0; //set LCD to write


    //PORTC = d >> 4;
    D4 = d.4; D5 = d.5; D6 = d.6; D7 = d.7;
    E = 1; E = 0;


    //PORTC = d;
    D4 = d.0; D5 = d.1; D6 = d.2; D7 = d.3;
    E = 1; E = 0;
}


//Sends a given string to the LCD. Will start printing from
//current cursor position.
void send_string(const char *incoming_string)
{
    uns8 x;
    
    for(x = 0 ; incoming_string[x] != '\0' ; x++)
        send_char(incoming_string[x]);
}


void set_baud_rate(void)
{
    //changed for 4MHz
    if (baud == 0) SPBRG = 255;             //2400
    else if (baud == 1) SPBRG = 128;        //4800
    else if (baud == 2) SPBRG = 64;         //9600
    else if (baud == 3) SPBRG = 43;         //14400
    else if (baud == 4) SPBRG = 31;         //19200
    else if (baud == 5) SPBRG = 15;         //38400
    
    else
    {
        SPBRG = 64;
        onboard_eewrite(2, BAUD_RATE);
    }
    
    
}


//General short delay
void delay_ms(uns16 x)
{
    //Clocks out at 1006us per 1ms @ 8MHz
    uns8 y, z;
    for ( ; x > 0 ; x--)
        for ( y = 0 ; y < 4 ; y++)
            for ( z = 0 ; z < 69 ; z++);
}


//Reads e_data from the onboard eeprom at e_address
uns8 onboard_eeread(uns8 e_address)
{
    //Do a read
    EEADR = e_address; //Set the address to read
    EEPGD = 0; //Point to EEPROM Memory
    RD = 1; //Read it
    
    return(EEDATA); //Read that EEPROM value
}    


//Write e_data to the onboard eeprom at e_address
void onboard_eewrite(uns8 e_data, uns8 e_address)
{
    bit temp_GIE = GIE;
    
    EEIF = 0; //Clear the write completion intr flag
    EEADR = e_address; //Set the address
    EEDATA = e_data; //Give it the data
    EEPGD = 0; //Point to EEPROM data block?
    WREN = 1; //Enable EE Writes
    GIE = 0; //Disable Intrs
    
    //Specific EEPROM write steps
    EECON2 = 0x55;
    EECON2 = 0xAA;
    WR = 1;
    //Specific EEPROM write steps


    while(EEIF == 0); //Wait for write to complete
    WREN = 0; //Disable EEPROM Writes


    GIE = temp_GIE; //Set GIE to its original state
}

