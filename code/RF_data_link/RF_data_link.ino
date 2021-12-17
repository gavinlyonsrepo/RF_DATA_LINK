//********************** HEADER ***********************
/*
  Name : Arduino File template
  Title : RF data Link project ::  Receiver side
  Author: Gavin Lyons
  URL:https://github.com/gavinlyonsrepo/RF_DATA_LINK
  Date created: 2021-Nov
  IC: ATMEGA328P
*/

// ********************Connections ****************
/*
Receiver ATMEGA328P

1. I2C SDA  = LM75A SDA
2. I2C SCLK = LM75A SCLK
3. D11 = 433Mhz Rx Data in 
4. D10 OLED_DC 
5. D9 OLED_RES 9 
6. D8 OLED_CS 8  
7. D4 OLED_SCLK 4 
8. D5 OLED_SDATA 5
*/

//******************** LIBRARIES ******************
#include <M2M_LM75A.h> // [URL](https://github.com/m2m-solutions/M2M_LM75A)
#include "ER_OLEDM1_CH1115.h" // [URL](https://github.com/gavinlyonsrepo/ER_OLEDM1_CH1115)
#include <RH_ASK.h> // [URL](https://www.airspayce.com/mikem/arduino/RadioHead/)
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

//******************** GLOBALS ********************

// Serial
//#define RF_SERIAL_DEBUG_ON // comment in for serial debug 
#define RF_SERIAL_BAUD 38400

// Test timing
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to update testcount (milliseconds)
uint8_t TestCount = 0;
#define READSENSOR 30 // sensor internal polled seconds (number of testcounts)
#define INITDELAY  1000   // initial delay mS

// LM75 sensor
M2M_LM75A lm75a;

// OLED
#define OLEDcontrast 0x80 //Contrast 00 to FF , 0x80 is default. user adjust
#define myOLEDheight 64
#define myOLEDwidth  128
#define OLED_DC 10 // GPIO pin number pick any you want 
#define OLED_RES 9 // "
#define OLED_CS 8  // "
#define OLED_SCLK 4 // "
#define OLED_SDATA 5 // "
ERMCH1115  myOLED(OLED_DC, OLED_RES, OLED_CS, OLED_SCLK, OLED_SDATA);  // GPIO 5-wire Software SPI interface

// RF 433 mHz ask
RH_ASK driver;
uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
uint8_t buflen = sizeof(buf);
uint16_t RXCount = 0;

//******************** SETUP ************************
void setup() {
  delay(INITDELAY);

  // LM75 setup
  lm75a.begin();

  // OLED setup
  myOLED.OLEDbegin(OLEDcontrast); // initialize the OLED
  myOLED.OLEDFillScreen(0x00, 0);

  // Serial Setup
#ifdef RF_SERIAL_DEBUG_ON
  while (!Serial) {}
  Serial.begin(RF_SERIAL_BAUD);
  Serial.println(F("==== RF_DATA_LINK -- UP ===="));
#endif

  // RF Setup
  if (!driver.init())
#ifdef RH_HAVE_SERIAL
    Serial.println("init failed");
#else
    ;
#endif

}


//******************* MAIN LOOP *****************
void loop() {

  //define a buffer to cover half  screen
  uint8_t  screenBuffer[(myOLEDwidth * (myOLEDheight / 8)) / 2]; //(128 * 8)/2 = 512 bytes

  MultiBuffer top_side;
  top_side.screenbitmap = (uint8_t*) &screenBuffer;
  top_side.width = (myOLEDwidth) ;
  top_side.height = myOLEDheight / 2;
  top_side.xoffset = 0;
  top_side.yoffset = 0;

  MultiBuffer bot_side;
  bot_side.screenbitmap = (uint8_t*) &screenBuffer;
  bot_side.width = (myOLEDwidth);
  bot_side.height = myOLEDheight / 2;
  bot_side.xoffset = 0 ;
  bot_side.yoffset = (myOLEDheight / 2);

  DisplayInternal(&top_side); //first display
  DisplayExternal(&bot_side, true);

  while (true) // Test loop
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      TestCount++; // update every second
    }


    if ( TestCount >=  READSENSOR ) // Read Internal sensor every READSENSOR time
    {
      TestCount = 0;
      DisplayInternal(&top_side);
    }

    if (driver.recv(buf, &buflen)) // Non-blocking, Did data come from transmitter?
    {
      RXCount++;
      if (RXCount == 9999) RXCount = 0;
      DisplayExternal(&bot_side, false);
    }

  }// test while

} // END of main


// ********************* FUNCTIONS ************

// Func Desc ::  function to display the external data on OLED
// Param1 A pointer to the  OLED buffer struct
// Param2 bool if true just display text and no data.
 
void DisplayExternal(MultiBuffer* targetbuffer, bool NoDataDisplay)
{

  myOLED.ActiveBuffer = targetbuffer; // set target buffer object
  myOLED.OLEDclearBuffer();
  myOLED.setCursor(0, 0);
  myOLED.setTextSize(1);
  myOLED.print("Ext Data :RX = ");
  myOLED.print(RXCount);

  if (NoDataDisplay == true)
  {
    myOLED.setCursor(0, 14);
    myOLED.print("No Data");
    myOLED.OLEDupdate();
    return;
  }
  String str_humid;
  String str_temp;
  String str_out;

  // Message with a good checksum received, dump it.
#ifdef RF_SERIAL_DEBUG_ON
  driver.printBuffer("RX data Got:", buf, buflen);
#endif

  str_out = String((char*)buf);
  // Split string into two values
  for (uint16_t i = 0; i < str_out.length(); i++) {
    if (str_out.substring(i, i + 1) == ",") {
      str_humid = str_out.substring(0, i);
      str_temp = str_out.substring(i + 1);
      break;
    }
  }

#ifdef RF_SERIAL_DEBUG_ON
  Serial.print("Humidity: ");
  Serial.print(str_humid);
  Serial.print("  - Temperature: ");
  Serial.println(str_temp);
#endif

  myOLED.setCursor(0, 14);
  if (str_temp.toInt() == 255 || str_humid.toInt() == 255)
  {
    myOLED.print("Sensor error");
  } else if ( (str_temp.toInt() <= -50) || (str_temp.toInt() >= 95) )
  {
    myOLED.print("Out of range");
  } else if ( (str_humid.toInt() <= -5) || (str_temp.toInt() >= 105) )
  {
    myOLED.print("Out of range");
  }
  else
  {
    myOLED.setTextSize(2);
    myOLED.print(str_temp);

    myOLED.setCursor(0, 0);
    myOLED.setTextSize(1);
    myOLED.setCursor(55, 22);
    myOLED.print(" C ");
    myOLED.print(str_humid);
    myOLED.print(" %");
  }

  myOLED.OLEDupdate();  //write to active buffer
}

// Func Desc ::  function to display the internal data on OLED
// Param1 A pointer to the  OLED buffer struct

void DisplayInternal(MultiBuffer* targetbuffer)
{
  float temperatureReading = 0.0;
  temperatureReading = lm75a.getTemperature();
  myOLED.ActiveBuffer = targetbuffer; // set target buffer object
  myOLED.OLEDclearBuffer();
  myOLED.setCursor(0, 0);
  myOLED.setTextSize(1);
  myOLED.print("Internal Temperature");
  myOLED.setCursor(0, 14);

  if (temperatureReading == -1000.0)
  {
    myOLED.print("Sensor error");
  } else if ((temperatureReading <= -60.0) || (temperatureReading >= 130.0) )
  {
    myOLED.print("Out of range");
  } else
  {

    myOLED.setTextSize(2);
    myOLED.print(temperatureReading);
    myOLED.print(" C");
  }

  myOLED.OLEDupdate();  //write to active buffer

}

//******************* EOF *****************
