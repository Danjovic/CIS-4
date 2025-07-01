

/***
 *     $$$$$$\  $$$$$$\  $$$$$$\         $$\   $$\       
 *    $$  __$$\ \_$$  _|$$  __$$\        $$ |  $$ |      
 *    $$ /  \__|  $$ |  $$ /  \__|       $$ |  $$ |      
 *    $$ |        $$ |  \$$$$$$\ $$$$$$\ $$$$$$$$ |      
 *    $$ |        $$ |   \____$$\\______|\_____$$ |      
 *    $$ |  $$\   $$ |  $$\   $$ |             $$ |      
 *    \$$$$$$  |$$$$$$\ \$$$$$$  |             $$ |      
 *     \______/ \______| \______/              \__|      
 *                                                       
 *                                                       
 *                                                       
 *     $$$$$$\  $$\                     $$\              
 *    $$  __$$\ $$ |                    $$ |             
 *    $$ /  \__|$$ | $$$$$$\   $$$$$$$\ $$ |  $$\        
 *    $$ |      $$ |$$  __$$\ $$  _____|$$ | $$  |       
 *    $$ |      $$ |$$ /  $$ |$$ /      $$$$$$  /        
 *    $$ |  $$\ $$ |$$ |  $$ |$$ |      $$  _$$<         
 *    \$$$$$$  |$$ |\$$$$$$  |\$$$$$$$\ $$ | \$$\        
 *     \______/ \__| \______/  \_______|\__|  \__|       
 *                                                       
 *                                                       
 *    Minimalist representation (4 led) Cistercian Number clock
 *    Danjovic 2025
 *    Released under GPL 3.0   
 *
 *    Version 1.1 - June, 2025
 *    Added Autoflip, - flip displayu mode at 15 and 45 seconds of every minute
 */


/*  Digispark Pinout
    0  SDA, I2C
    1  Neopixel display
    2  SCL, I2C
    3  Buzzer
    4  Push Button
    5  LDR, analog
*/

/*  LED Matrix arrangement
    03 07 11 15     tn tn un un   un = units 
    02 06 10 14     tn tn un un   tn = tens
    01 05 09 13     th th hu hu   hu = hundreds
    00 04 08 12     th th hu hu   th = thousands
*/

/*  Cistercian numbers representation
         0    1    2    3    4    5    6    7    8    9
    01s . .  O O  . .  O .  . O  O O  . O  O O  . O  O O
        . .  . .  O O  . O  O .  O .  . O  . O  O O  O O
		
    10s . .  O O  . .  . O  O .  O O  O .  O O  O .  O O
        . .  . .  O O  O .  . O  . O  O .  O .  O O  O O
		
   100s . .  . .  O O  . O  O .  O .  . O  . O  O O  O O
        . .  O O  . .  O .  . O  O O  . O  O O  . O  O O
		
  1000s . .  . .  O O  O .  . O  . O  O .  O .  O O  O O
        . .  O O  . .  . O  O .  O O  O .  O O  O .  O O
  
  
     1         20       300       4000      5555      6789      9394
  . . O O   . . . .   . . . .   . . . .   O O O O   O . O O   O O . O
  . . . .   O O . .   . . . .   . . . .   . O O .   O O O O   O O O .
  . . . .   . . . .   . . . O   . O . .   . O O .   O . . O   O O . O
  . . . .   . . . .   . . O .   O . . .   O O O O   O . O O   O O O .
      
*/

/*
  Config byte on EEPROM address 0x0f
  Bits   Function  
    7    Decimal (0) / Cistercian (1)
    6    Hourly Beep Off(0) / On (1)
    5    Autoflip Off(0) / On (1) 
   4-3   Not used
   2-0   Color mode: 0 0 0 random  (changes every 10 seconds)
                     0 0 1 Blue
                     0 1 0 Green
                     0 1 1 Cyan
                     1 0 0 Red
                     1 0 1 Magenta
                     1 1 0 Yellow
                     1 1 1 White
*/

//    _ _ _                 _
//   | (_) |__ _ _ __ _ _ _(_)___ ___
//   | | | '_ \ '_/ _` | '_| / -_|_-<
//   |_|_|_.__/_| \__,_|_| |_\___/__/
//
#include <tinyNeoPixel.h>
#include <EEPROM.h>
#include <util/delay.h>

//       _      __ _      _ _   _
//    __| |___ / _(_)_ _ (_) |_(_)___ _ _  ___
//   / _` / -_)  _| | ' \| |  _| / _ \ ' \(_-<
//   \__,_\___|_| |_|_||_|_|\__|_\___/_||_/__/
//

// pin connected to the NeoPixels
#define neopixelPIN 1
//#define NUMPIXELS 16 // size of NeoPixels display
#define NUMPIXELS 25  // TODO CHANGE to 16 for 4x4 matrix

// pin connected to buzzer
#define buzzerPin 4

// pin connected to button. main tick is 10ms
#define buttonPin 3
#define LONG_PRESS_THRESHOLD 50  // 500ms second in 1/10th of seconds
#define SHORT_PRESS_THRESHOLD 5  // 50ms

// pin connected to the light sensor
#define lightSensorPin 5        // PB5
#define lightSensorArduinoPin A0 // necesasry for analogRead

// eeprom addres where configuration is stored
#define EEPROM_CONFIG_ADDRESS 15

// I2C stuff
#define PORT_I2C PORTB
#define DDR_I2C DDRB
#define PIN_I2C PINB
#define bSCL 2
#define bSDA 0
#define sendACK true
#define sendNACK false
#define RTC_ADDRESS 0x68

#define MAX_SETUP_MODES 4

//
//    _ __  __ _ __ _ _ ___ ___
//   | '  \/ _` / _| '_/ _ (_-<
//   |_|_|_\__,_\__|_| \___/__/
//

// I2C related
#define sdaHigh() \
  do { \
    DDR_I2C &= ~(1 << bSDA); \
    PORT_I2C |= (1 << bSDA); \
  } while (0)
#define sdaLow() \
  do { \
    DDR_I2C |= (1 << bSDA); \
    PORT_I2C &= ~(1 << bSDA); \
  } while (0)
#define sdaGet() ((PIN_I2C & (1 << bSDA)) != 0)
#define sclHigh() \
  do { \
    DDR_I2C &= ~(1 << bSCL); \
    PORT_I2C |= (1 << bSCL); \
  } while (0)
#define sclLow() \
  do { \
    DDR_I2C |= (1 << bSCL); \
    PORT_I2C &= ~(1 << bSCL); \
  } while (0)
#define I2Cdelay() _delay_us(5)



//       _      _             _               _
//    __| |__ _| |_ __ _   __| |_ _ _ _  _ __| |_ _  _ _ _ ___ ___
//   / _` / _` |  _/ _` | (_-<  _| '_| || / _|  _| || | '_/ -_|_-<
//   \__,_\__,_|\__\__,_| /__/\__|_|  \_,_\__|\__|\_,_|_| \___/__/
//

// state Machine
typedef enum {
  staINIT = 0,
  staSHOW_TIME,
  staSHOW_FLIP_MODE,
  staCHANGE_MODE,
  staSETUP,
  staSET_COLOR,
  staSET_BEEP,
  staSET_AUTO,
  staSET_HOUR,
  staSET_MINUTE
} t_states;

// button events
typedef enum {
  btNONE = 0,
  btLONG,
  btPULSE
} t_buttonEvents;

// RTC
typedef struct {
  unsigned units : 4;
  unsigned tens : 3;
  unsigned : 1;
} t_seconds;

typedef struct {
  unsigned units : 4;
  unsigned tens : 3;
  unsigned : 1;
} t_minutes;

typedef struct {
  unsigned units : 4;
  unsigned tens : 2;
  unsigned op12_24 : 1;
  unsigned : 1;
} t_hours;

typedef struct {
  unsigned dow : 3;  // day of week
  unsigned : 5;
} t_wkDays;

typedef struct {
  unsigned units : 4;
  unsigned tens : 2;
  unsigned : 2;
} t_dates;

typedef struct {
  unsigned units : 4;
  unsigned tens : 1;
  unsigned : 2;
  unsigned century : 1;
} t_monthsCty;

typedef struct {
  unsigned units : 4;
  unsigned tens : 4;
} t_years;

typedef struct {
  t_seconds seconds;
  t_seconds minutes;
  t_hours hours;
  t_wkDays wkDay;
  t_dates date;
  t_monthsCty monthCty;
  t_years year;
} t_timeAndDate;

typedef union {
  uint8_t rawdata[7];
  t_timeAndDate datetime;
} t_ds3231records;



//                 _       _
//    _ __ _ _ ___| |_ ___| |_ _  _ _ __  ___ ___
//   | '_ \ '_/ _ \  _/ _ \  _| || | '_ \/ -_|_-<
//   | .__/_| \___/\__\___/\__|\_, | .__/\___/__/
//   |_|                       |__/|_|

// State Machine
void runINIT(void);            // initialize the state machine with settings retrieved from eeprom
void runSHOW_TIME(void);       // show time in current mode
void runSHOW_FLIP_MODE(void);  // show other mode temporarily
void runCHANGE_MODE(void);     // switch Cistercian/Decimal mode
void runSETUP(void);           // main hub for changing 
void runSET_COLOR(void);       // change display colors
void runSET_BEEP(void);        // beep every hour
void runSET_AUTO(void);        // Auto flip mode 
void runSET_HOUR(void);        // adjust hours 
void runSET_MINUTE(void);      // adjust minutes

// Button handling
t_buttonEvents getButtonEvent(uint8_t pin);  // scan button

// Display
void randomizeColors(void);             // assign random colors to display quadrants
void initializeColors(uint8_t color);   // fulfill display color table
void fulfillCistercian(void);           // fulfill display with Cistercian digits
void fulfillDecimal(void);              // fullfill decimal display buffer with decimal digits
void refreshDisplay(void);              // refresh display
uint16_t scrollDisplay(uint8_t index);  // return the pixmap for the nth position of the decimal buffer index from 0 to 19

// RTC
void retrieveTime(void);  // retrieve time from RTC
bool readRtc(t_ds3231records *t);    // read RTC into buffer
bool writeRtc(t_ds3231records *t);   // write buffer to RTC

// I2C
void I2Cstart(void);
void I2Cstop(void);
bool I2Cwrite(uint8_t d);
uint8_t I2Cread(bool nack);



//                   _            _
//    __ ___ _ _  __| |_ __ _ _ _| |_ ___
//   / _/ _ \ ' \(_-<  _/ _` | ' \  _(_-<
//   \__\___/_||_/__/\__\__,_|_||_\__/__/
//

const PROGMEM uint16_t cisUnits[10] = {
  0,                                             // 0
  (1 << 11) | (1 << 15),                         // 1
  (1 << 10) | (1 << 14),                         // 2
  (1 << 11) | (1 << 14),                         // 3
  (1 << 10) | (1 << 15),                         // 4
  (1 << 10) | (1 << 11) | (1 << 15),             // 5
  (1 << 14) | (1 << 15),                         // 6
  (1 << 11) | (1 << 14) | (1 << 15),             // 7
  (1 << 10) | (1 << 14) | (1 << 15),             // 8
  (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15)  // 9
};

const PROGMEM uint16_t cisTenths[10] = {
  0,                                         // 0
  (1 << 3) | (1 << 7),                       // 1
  (1 << 2) | (1 << 6),                       // 2
  (1 << 2) | (1 << 7),                       // 3
  (1 << 3) | (1 << 6),                       // 4
  (1 << 3) | (1 << 6) | (1 << 7),            // 5
  (1 << 2) | (1 << 3),                       // 6
  (1 << 2) | (1 << 3) | (1 << 7),            // 7
  (1 << 2) | (1 << 3) | (1 << 6),            // 8
  (1 << 2) | (1 << 3) | (1 << 6) | (1 << 7)  // 9
};

const PROGMEM uint16_t cisHundreds[10] = {
  0,                                           // 0
  (1 << 8) | (1 << 12),                        // 1
  (1 << 9) | (1 << 13),                        // 2
  (1 << 8) | (1 << 13),                        // 3
  (1 << 9) | (1 << 12),                        // 4
  (1 << 8) | (1 << 9) | (1 << 12),             // 5
  (1 << 12) | (1 << 13),                       // 6
  (1 << 8) | (1 << 12) | (1 << 13),            // 7
  (1 << 9) | (1 << 12) | (1 << 13),            // 8
  (1 << 8) | (1 << 9) | (1 << 12) | (1 << 13)  // 9
};

const PROGMEM uint16_t cisThousands[10] = {
  0,                                         // 0
  (1 << 0) | (1 << 4),                       // 1
  (1 << 1) | (1 << 5),                       // 2
  (1 << 1) | (1 << 4),                       // 3
  (1 << 0) | (1 << 5),                       // 4
  (1 << 0) | (1 << 4) | (1 << 5),            // 5
  (1 << 0) | (1 << 1),                       // 6
  (1 << 0) | (1 << 1) | (1 << 4),            // 7
  (1 << 0) | (1 << 1) | (1 << 5),            // 8
  (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5)  // 9
};

const PROGMEM uint16_t decDigits[10] = {
  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11),             // 0
  (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7),                                                                                 // 1
  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 10) | (1 << 11),                        // 2
  (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11),                        // 3
  (1 << 1) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11),                                   // 4
  (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 11),                         // 5
  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 11),              // 6
  (1 << 3) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11),                                                         // 7
  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11),  // 8
  (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11)              // 9
};

const PROGMEM uint32_t colorTable[7] = { 0x0000ff, 0x00ff00, 0x00ffff, 0xff0000, 0xff00ff, 0xffff00, 0xffffff };

//const PROGMEM uint8_t translate[16] = { 0, 9, 10, 19, 1, 8, 11, 18, 2, 7, 12, 17, 3, 6, 13, 16 };  // TODO change for 4x4 display
//const PROGMEM uint8_t translate[16] = { 0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 10, 11, 15, 14, 13, 12 };
const PROGMEM uint8_t translate[16] = { 0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15 };

const PROGMEM uint16_t setup_letters[MAX_SETUP_MODES] = {
  (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 11),                                              // letter T
  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 7) | (1 << 8) | (1 << 11),                        // letter C
  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9)  | (1 << 10),  // letter B
  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11)  // letter A  
};



//        _     _
//    ___| |__ (_)___ __| |_ ___
//   / _ \ '_ \| / -_) _|  _(_-<
//   \___/_.__// \___\__|\__/__/
//           |__/

tinyNeoPixel pixels(NUMPIXELS, neopixelPIN, NEO_GRB + NEO_KHZ800);


//                 _      _    _
//   __ ____ _ _ _(_)__ _| |__| |___ ___
//   \ V / _` | '_| / _` | '_ \ / -_|_-<
//    \_/\__,_|_| |_\__,_|_.__/_\___/__/
//

// clock configuration and control
bool displayMode;          // Decimal / Cistercian 4
bool beepMode;             // Hourly beep on / off
bool autoMode;             // Autoflip on / off
uint8_t baseColor;         // Display colors
uint8_t setupMode = 0;     // setup mode (time,beep,color,autoflip)
t_states state = staINIT;  // main machine state

// RTC
t_ds3231records rtc;
uint8_t hours, minutes, seconds;
uint8_t timeUnits, timeTenths, timeHundreds, timeThousands;

// display
bool displayBlink = false;
uint16_t displayPixels;      // neopixel state (on/off)
uint32_t displayColors[16];  // neopixel color attributes
uint16_t displayDecimal[6];  // decimal framebuffer, first and last are blank
uint8_t index = 0;           // used by display scroll
uint8_t delayticks = 0;      // ditto

// button
t_buttonEvents buttonEvent;



//                 _       _
//    _ __ _ _ ___| |_ ___| |_ _  _ _ __  ___ ___
//   | '_ \ '_/ _ \  _/ _ \  _| || | '_ \/ -_|_-<
//   | .__/_| \___/\__\___/\__|\_, | .__/\___/__/
//   |_|                       |__/|_|

// State Machine
void runINIT(void);            // initialize the state machine with settings retrieved from eeprom
void runSHOW_TIME(void);       // show time in current mode
void runSHOW_FLIP_MODE(void);  // show other mode temporarily
void runCHANGE_MODE(void);     // switch Cistercian/Decimal mode
void runSETUP(void);           // main hub for changing 
void runSET_COLOR(void);       // change display colors
void runSET_BEEP(void);        // beep every hour
void runSET_HOUR(void);        // adjust hours 
void runSET_MINUTE(void);      // adjust minutes

// Button handling
t_buttonEvents getButtonEvent(uint8_t pin);  // scan button

// Display
void randomizeColors(void);             // assign random colors to display quadrants
void initializeColors(uint8_t color);   // fulfill display color table
void fulfillCistercian(void);           // fulfill display with Cistercian digits
void fulfillDecimal(void);              // fullfill decimal display buffer with decimal digits
void refreshDisplay(void);              // refresh display
uint16_t scrollDisplay(uint8_t index);  // return the pixmap for the nth position of the decimal buffer index from 0 to 19

// RTC
void retrieveTime(void);  // retrieve time from RTC
bool readRtc(t_ds3231records *t);    // read RTC into buffer
bool writeRtc(t_ds3231records *t);   // write buffer to RTC

// I2C
void I2Cstart(void);
void I2Cstop(void);
bool I2Cwrite(uint8_t d);
uint8_t I2Cread(bool nack);


//            _
//    ___ ___| |_ _  _ _ __
//   (_-</ -_)  _| || | '_ \
//   /__/\___|\__|\_,_| .__/
//                    |_|
void setup() {

  // Initialize I/O ports
  DDRB = ((0 << bSCL) |            // Input
          (0 << bSDA) |            // Input
          (1 << buzzerPin) |       // Output
          (1 << neopixelPIN) |     // Output
          (0 << buttonPin) |       // Input
          (0 << lightSensorPin));  // Input

  PORTB = ((1 << bSCL) |            // Pullup
           (1 << bSDA) |            // Pullup
           (0 << buzzerPin) |       // Low
           (0 << neopixelPIN) |     // Low
           (1 << buttonPin) |       // Pullup
           (0 << lightSensorPin));  // no Pullup

  pixels.begin();  // This initializes the NeoPixel library.
}
//


//    _
//   | |___  ___ _ __
//   | / _ \/ _ \ '_ \
//   |_\___/\___/ .__/
//              |_|
void loop() {

  // run state machine
  switch (state) {
    case staINIT:  // restore configurations after powerup
      runINIT();
      break;
    case staSHOW_TIME:
      runSHOW_TIME();
      break;
    case staSHOW_FLIP_MODE:
      runSHOW_FLIP_MODE();
      break;
    case staCHANGE_MODE:
      runCHANGE_MODE();
      break;
    case staSETUP:
      runSETUP();
      break;
    case staSET_COLOR:
      runSET_COLOR();
      break;
    case staSET_BEEP:
      runSET_BEEP();
      break;
    case staSET_AUTO:
      runSET_AUTO();
      break;
    case staSET_HOUR:
      runSET_HOUR();
      break;
    case staSET_MINUTE:
      runSET_MINUTE();
      break;
  }

  // retrieve time from RTC
  retrieveTime();

  // scan button
  buttonEvent = getButtonEvent(buttonPin);

  // refresh display
  refreshDisplay();

  // tick 10ms
  delay(10);  // TODO use a timer to set the base time
}
//



//    _            _                   _        _   _
//   (_)_ __  _ __| |___ _ __  ___ _ _| |_ __ _| |_(_)___ _ _
//   | | '  \| '_ \ / -_) '  \/ -_) ' \  _/ _` |  _| / _ \ ' \ 
//   |_|_|_|_| .__/_\___|_|_|_\___|_||_\__\__,_|\__|_\___/_||_|
//           |_|

// ******************************************************************************************************************
//
// State Machine implementation
//

// initialize the state machine with settings retrieved from eeprom
void runINIT(void) {
  uint8_t configValue = EEPROM.read(EEPROM_CONFIG_ADDRESS);

  //initialize colors
  baseColor = configValue & 0x07;  // only bits 2..0
  initializeColors(baseColor);
  //EEPROM.write(EEPROM_CONFIG_ADDRESS, value);

  // initialize autoflip mode
  autoMode = (configValue & (1 << 5) ? true : false);

  // initialize beep mode
  beepMode = (configValue & (1 << 6) ? true : false);

  // inidialize main display mode
  displayMode = (configValue & (1 << 7) ? true : false);

  // define next state
  state = staSHOW_TIME;

}
//


// show time in current mode
void runSHOW_TIME(void) {
  if (displayMode) {
    // show Cistercian
    fulfillCistercian();
  } else {
    // show decimal
    if ((index + delayticks) == 0) {  // first entry
      // fullfill decimal data
      fulfillDecimal();
    }

    // display current slice
    displayPixels = scrollDisplay(index);

    // advance slice pointer
    delayticks++;
    if (delayticks > 30) {
      delayticks = 0;
      index++;
      if (index == 20) {
        index = 0;
      }
    }
  }


  if ( (buttonEvent == btPULSE) || (autoMode==true && ( (seconds==15) || (seconds==45) ) ) )  {
//  if (buttonEvent == btPULSE) {
    delayticks = 0;
    index = 0;
    state = staSHOW_FLIP_MODE;
  }
  if (buttonEvent == btLONG) {
    state = staSETUP;
  }
}
//


// show other mode temporarily
void runSHOW_FLIP_MODE(void) {
  if (displayMode) {  // if cistercian then show decimal
    // show decimal
    if ((index + delayticks) == 0) {  // first entry
      // fullfill decimal data
      fulfillDecimal();
    }
    // display current slice
    displayPixels = scrollDisplay(index);

  } else {  // show cistercian
    // show Cistercian
    fulfillCistercian();
  }

  // advance slice pointer
  delayticks++;
  if (delayticks > 30) {
    delayticks = 0;
    index++;
    if (index == 20) {
      index = 0;
      state = staSHOW_TIME;
    }
  }

  // check buttons
  if (buttonEvent == btPULSE) {
    index = 0;
    delayticks = 0;
    state = staSHOW_TIME;
  }

  if (buttonEvent == btLONG) {
    delayticks = 0;
    state = staCHANGE_MODE;
  }
}
//


// switch Cistercian/Decimal mode
void runCHANGE_MODE(void) {
  if (delayticks == 0) {

    displayMode = !displayMode;                                            // flip the display mode
    uint8_t configValue = EEPROM.read(EEPROM_CONFIG_ADDRESS) & ~(1 << 7);  // mask bit7
    configValue |= (displayMode ? (1 << 7) : 0);                           // set or reset beep bit accordingly
    EEPROM.write(EEPROM_CONFIG_ADDRESS, configValue);                      // save on EEPROM
    displayPixels = 0xffff;
  } else if (delayticks > 80) {
    state = staSHOW_TIME;  // always return to show time
    delayticks = 0;
  }
  delayticks++;
}
//


// show time in current mode
void runSETUP(void) {
  displayPixels = pgm_read_dword(setup_letters + setupMode);

  if (buttonEvent == btPULSE) {
    if (++setupMode == MAX_SETUP_MODES ) setupMode = 0;
    delayticks = 0;
  }

  if (buttonEvent == btLONG) {
    switch (setupMode) {
      case 0:
        state = staSET_HOUR;
        break;
      case 1:
        state = staSET_COLOR;
        break;
      case 2: 
        state = staSET_BEEP;
        break;
      case 3: 
      default:
        state = staSET_AUTO;
        break;
    }
  }
}
//


// change display colors
void runSET_COLOR(void) {
  // set blink mode
  displayBlink = true;

  // advance color
  if (buttonEvent == btPULSE) {
    if (++baseColor > 7) baseColor = 0;
    initializeColors(baseColor);
  }

  if (buttonEvent == btLONG) {
    // reset blink mode
    displayBlink = false;
    // save color in eeprom
    uint8_t configValue = EEPROM.read(EEPROM_CONFIG_ADDRESS) & 0xf8;        // mask bits 2..0
    EEPROM.write(EEPROM_CONFIG_ADDRESS, configValue | (baseColor & 0x07));  //  add base color

    state = staSHOW_TIME;
  }
}
//


// beep every hour
void runSET_BEEP(void) {
  // set blink mode
  displayBlink = true;

  // show 0 / 1 number according with last state
  displayPixels = pgm_read_word(decDigits + (beepMode ? 1 : 0));

  // change mode
  if (buttonEvent == btPULSE) {
    beepMode = !beepMode;
  }

  if (buttonEvent == btLONG) {
    // reset blink mode
    displayBlink = false;
    // save beep state  in eeprom
    uint8_t configValue = EEPROM.read(EEPROM_CONFIG_ADDRESS) & ~(1 << 6);  // mask bit 6
    configValue |= (beepMode ? (1 << 6) : 0);                              // set or reset beep bit accordingly
    EEPROM.write(EEPROM_CONFIG_ADDRESS, configValue);                      //  write config bit
    state = staSHOW_TIME;
  }
}
//



// Enable/Disable Autoflip display mode
void runSET_AUTO(void) {
  // set blink mode
  displayBlink = true;

  // show 0 / 1 number according with last state
  displayPixels = pgm_read_word(decDigits + (autoMode ? 1 : 0));

  // change mode
  if (buttonEvent == btPULSE) {
    autoMode = !autoMode;
  }

  if (buttonEvent == btLONG) {
    // reset blink mode
    displayBlink = false;
    // save autoflip state  in eeprom
    uint8_t configValue = EEPROM.read(EEPROM_CONFIG_ADDRESS) & ~(1 << 5);  // mask bit 5
    configValue |= (autoMode ? (1 << 5) : 0);                              // set or reset autoflip bit accordingly
    EEPROM.write(EEPROM_CONFIG_ADDRESS, configValue);                      //  write config bit
    state = staSHOW_TIME;
  }
}
//









// adjust hours 
void runSET_HOUR(void) {
  if (displayMode) {
     // set blink mode, only in cistercian mode
     displayBlink = true;

    // show Cistercian of hours
    fulfillCistercian();  // fullfill and mask to save code
    displayPixels &= ~((1 << 2) | (1 << 3) | (1 << 6) | (1 << 7) | (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15));

  } else {
    // show decimal

    // fullfill decimal data
    displayDecimal[4] = 0;
    displayDecimal[3] = pgm_read_word_near(decDigits + timeHundreds);
    displayDecimal[2] = pgm_read_word_near(decDigits + timeThousands);
    displayDecimal[1] = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 6) | (1 << 8) | (1 << 9) | (1 << 10);  // letter h

    // display current slice
    displayPixels = scrollDisplay(index);

    // advance slice pointer
    delayticks++;
    if (delayticks > 30) {
      delayticks = 0;
      index++;
      if (index == 15) {
        index = 0;
      }
    }
  }

  if (buttonEvent == btPULSE) {
    readRtc(&rtc);
    uint8_t d = ((1 + 10 * (uint8_t)rtc.datetime.hours.tens + (uint8_t)rtc.datetime.hours.units)) % 24;

    rtc.datetime.hours.tens = ((uint8_t)d / 10) & 0b00000011;
    rtc.datetime.hours.units = ((uint8_t)d % 10) & 0b00001111;

    // update RTC
    writeRtc(&rtc);
  }
  if (buttonEvent == btLONG) {
    index = 0;
    delayticks = 0;
    state = staSET_MINUTE;
  }
}
//


// adjust minutes
void runSET_MINUTE(void) {
  if (displayMode) {
    // show Cistercian of minutes
    fulfillCistercian();  // fullfill and mask to save code
    displayPixels &= ~((1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 12) | (1 << 13));

  } else {
    // show decimal

    // fullfill decimal data
    displayDecimal[4] = 0;
    displayDecimal[3] = pgm_read_word_near(decDigits + timeUnits);
    displayDecimal[2] = pgm_read_word_near(decDigits + timeTenths);
    displayDecimal[1] = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 6) | (1 << 8) | (1 << 9) | (1 << 10);  // letter m

    // display current slice
    displayPixels = scrollDisplay(index);

    // advance slice pointer
    delayticks++;
    if (delayticks > 30) {
      delayticks = 0;
      index++;
      if (index == 15) {
        index = 0;
      }
    }
  }

  if (buttonEvent == btPULSE) {
    readRtc(&rtc);
    uint8_t d = (1 + 10 * (uint8_t)rtc.datetime.minutes.tens + (uint8_t)rtc.datetime.minutes.units) % 60;
    rtc.datetime.minutes.tens = ((uint8_t)d / 10) & 0b00000111;
    rtc.datetime.minutes.units = ((uint8_t)d % 10) & 0b00001111;
    
    // update RTC
    writeRtc(&rtc);
  }
  if (buttonEvent == btLONG) {
    index = 0;
    delayticks = 0;
    state = staSHOW_TIME;
    displayBlink = false;    // turn off blink 
  }
}
//



// ******************************************************************************************************************
//
// Button Handling
//

// scan button
t_buttonEvents getButtonEvent(uint8_t pin) {
  static uint8_t pressCount = 0;

  bool buttonPress = (!digitalRead(pin));
  if (!buttonPress) {  // release
    if (pressCount > SHORT_PRESS_THRESHOLD && pressCount < LONG_PRESS_THRESHOLD) {
      pressCount = 0;
      return btPULSE;
    }
    pressCount = 0;
  } else {                               // press
    if (pressCount < 250) pressCount++;  // saturate
    if (pressCount == LONG_PRESS_THRESHOLD) return btLONG;
  }
  return btNONE;
}
//



// ******************************************************************************************************************
//
// Display handling
//

// assign random colors to display quadrants
void randomizeColors(void) {
  uint8_t n, color1, color2, color3, color4;

  // fulfill color 1
  color1 = rand() % 7;
  for (n = 0; n < 4; n++)
    displayColors[n & 9 | (n & 2) << 1 | (n & 4) >> 1] = pgm_read_dword_near(colorTable + color1);

  // fulfill color 2
  do {
    color2 = rand() % 7;
  } while (color1 == color2);
  for (n = 4; n < 8; n++)
    displayColors[n & 9 | (n & 2) << 1 | (n & 4) >> 1] = pgm_read_dword_near(colorTable + color2);

  // fulfill color 3
  do {
    color3 = rand() % 7;
  } while (color3 == color1 || color3 == color2);  // here colors 1,2 not same from each other
  for (n = 8; n < 12; n++)
    displayColors[n & 9 | (n & 2) << 1 | (n & 4) >> 1] = pgm_read_dword_near(colorTable + color3);

  // fulfill color 4
  do {
    color4 = rand() % 7;
  } while (color4 == color1 || color4 == color2 || color4 == color3);  // here colors 1,2,3 not same from each other
  for (n = 12; n < 16; n++)
    displayColors[n & 9 | (n & 2) << 1 | (n & 4) >> 1] = pgm_read_dword_near(colorTable + color4);
}
//


// fulfill display color table
void initializeColors(uint8_t color) {
  if (color) {  // Solid Color
    for (uint8_t i = 0; i < 16; i++)
      displayColors[i] = pgm_read_dword_near(colorTable + (color - 1));

  } else {  // Random Color
    randomizeColors();
  }
}
//


// fulfill display with Cistercian digits
void fulfillCistercian(void) {
  displayPixels = pgm_read_word_near(cisUnits + timeUnits);
  displayPixels |= pgm_read_word_near(cisTenths + timeTenths);
  displayPixels |= pgm_read_word_near(cisHundreds + timeHundreds);
  displayPixels |= pgm_read_word_near(cisThousands + timeThousands);
}
//


// fullfill decimal display buffer with decimal digits
void fulfillDecimal(void) {
  displayDecimal[4] = pgm_read_word_near(decDigits + timeUnits);
  displayDecimal[3] = pgm_read_word_near(decDigits + timeTenths);
  displayDecimal[2] = pgm_read_word_near(decDigits + timeHundreds);
  displayDecimal[1] = pgm_read_word_near(decDigits + timeThousands);
}
//


// refresh display
void refreshDisplay(void) {
  static uint8_t blinkCounter = 0;
  static uint8_t brightnessFilter = 0;

  // auto adjust brightness every 1 second
  if (++brightnessFilter == 100) {
    uint8_t brightness = map(analogRead(lightSensorArduinoPin), 340, 800, 10, 100);
    pixels.setBrightness(brightness);
    brightnessFilter = 0;
  }

  // lit pixels
  uint16_t mask = 1;
  for (int i = 0; i < 16; i++) {
    if (displayPixels & mask) {
      // if pixel is lit apply color
      pixels.setPixelColor(pgm_read_byte_near(translate + i), displayColors[i]);
    } else {
      // otherwise turn it off
      pixels.setPixelColor(pgm_read_byte_near(translate + i), 0);
    }
    mask <<= 1;  // update mask
  }

  // erase display on blink
  if (displayBlink && blinkCounter > 30) pixels.clear();
  if (++blinkCounter == 50) blinkCounter = 0;

  pixels.show();  // Sends the updated colors to the neopixels
}
//


// return the pixmap for the nth position of the decimal buffer index from 0 to 19
uint16_t scrollDisplay(uint8_t index) {
  return (displayDecimal[index / 4] >> (4 * (index % 4)) | displayDecimal[(index / 4) + 1] << (16 - 4 * (index % 4)));
}
//


// ******************************************************************************************************************
//
// RTC chip handling
//

// retrieve time from RTC
void retrieveTime(void) {
  static uint8_t beepCounter = 0;
  static uint8_t sampleTimer = 0;

 
  if (sampleTimer == 0) {  // sample every quarter of second
    readRtc(&rtc);
  }
  if (sampleTimer++ > 25) sampleTimer = 0;

  // fulfill time units
  timeUnits = (uint8_t)rtc.datetime.minutes.units;
  timeTenths = (uint8_t)rtc.datetime.minutes.tens;
  timeHundreds = (uint8_t)rtc.datetime.hours.units;
  timeThousands = (uint8_t)rtc.datetime.hours.tens;

  // fulfill time variables
  hours = (uint8_t)rtc.datetime.hours.tens * 10 + (uint8_t)rtc.datetime.hours.units;
  minutes = (uint8_t)rtc.datetime.minutes.tens * 10 + (uint8_t)rtc.datetime.minutes.units;
  seconds = (uint8_t)rtc.datetime.seconds.tens * 10 + (uint8_t)rtc.datetime.seconds.units;


  // Beep every integer hour
  if (beepMode && ((hours + minutes + seconds) == 0) ) {

    if (beepCounter++ < 20) {
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(buzzerPin, LOW);
    }
  } else {
    beepCounter = 0;
    digitalWrite(buzzerPin, LOW);
  }
}
//


// read time data from clock Chip using I2C
bool readRtc(t_ds3231records *t) {
  uint8_t i;
  I2Cstart();
  if (!I2Cwrite((uint8_t)(RTC_ADDRESS << 1))) {
    I2Cwrite(0x00);  // register address, 1st clock register
    I2Cstart();      // repeated start
    I2Cwrite((uint8_t)(RTC_ADDRESS << 1) | 1);
    for (i = 0; i < 6; i++) {
      t->rawdata[i] = I2Cread(sendACK);
    }
    t->rawdata[i] = I2Cread(sendNACK);  // NACK on last bit
    I2Cstop();
    return true;
  } else {
    I2Cstop();
    I2Cstop();
  }
  return false;
}
//


// write time data on clock Chip using I2C
bool writeRtc(t_ds3231records *t) {
  uint8_t i;
  I2Cstart();
  if (!I2Cwrite((uint8_t)(RTC_ADDRESS << 1))) {
    I2Cwrite(0x00);  // register address, 1st clock register
    for (i = 0; i < 7; i++)
      I2Cwrite(t->rawdata[i]);
    I2Cstop();
    return true;
  } else {
    I2Cstop();
    I2Cstop();
  }
  return false;
}
//


// ******************************************************************************************************************
//
// Soft I2C
//
void I2Cstart() {
  sdaHigh();
  I2Cdelay();
  sclHigh();
  I2Cdelay();  // sda = 1;  scl = 1;
  sdaLow();
  I2Cdelay();  // sda = 0;
  sclLow();
}
//

void I2Cstop() {
  sdaLow();
  I2Cdelay();  // sda = 0;  sda = 0;
  sclHigh();
  I2Cdelay();  // scl = 1;  scl = 1;
  sdaHigh();   // sda = 1;
}
//

bool I2Cwrite(uint8_t d) {
  uint8_t i;
  bool nack;
  for (i = 0; i < 8; i++) {
    if (d & 0x80)  // write data bit, msb first
      sdaHigh();
    else sdaLow();
    I2Cdelay();  // give time to settle data
    sclHigh();
    I2Cdelay();
    sclLow();    // pulse clock
    d = d << 1;  // next bit
  }
  // now get the ack
  sdaHigh();
  I2Cdelay();  // release data line
  sclHigh();
  I2Cdelay();       // release clock line
  nack = sdaGet();  // get nack bit
  sclLow();         // clock low
  return nack;
}
//

uint8_t I2Cread(bool nack) {
  uint8_t i, d;

  d = 0;
  sdaHigh();  // release data line and
  sclLow();
  I2Cdelay();  // pull down clock line and wait to write a bit
  for (i = 0; i < 8; i++) {
    sclHigh();
    I2Cdelay();  // release clock line to read the data
    d = d << 1;
    if (sdaGet()) d |= 1;  // read data bit, msb first
    sclLow();
    I2Cdelay();  // pull clock down to allow next bit
  }
  // give ACK / NACK
  if (nack) sdaLow();
  else sdaHigh();

  sclHigh();
  I2Cdelay();  // Pulse clock
  sclLow();
  I2Cdelay();  //

  sdaHigh();  // release the data line
  return d;
}


//
//      . . o o    o o . o
//      . . o o    o o . o
//      . . . .    . . o o
//      o o o o    o o o o
