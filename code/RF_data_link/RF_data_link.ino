//********************** HEADER ***********************
/*
  Name : RF_DATA_LINK
  Title : RF data Link project ::  Receiver side
  Author: Gavin Lyons
  URL:https://github.com/gavinlyonsrepo/RF_DATA_LINK
  Date created: 2021-Nov
  IC: arduino nano ATMEGA328P
*/


// ********************Connections ****************
/*
  Receiver ATMEGA328P

  1. I2C SDA  = LM75A SDA
  2. I2C SCLK = LM75A SCLK
  3. D11 = 433Mhz Rx Data in
  4. D10 OLED_DC 10
  5. D9 OLED_RES 9
  6. D8 OLED_CS 8
  7. D4 OLED_SCLK 4
  8. D5 OLED_SDATA 5
*/

//******************** LIBRARIES ******************
#include <M2M_LM75A.h> // [URL](https://github.com/m2m-solutions/M2M_LM75A)
#include "ER_OLEDM1_CH1115.hpp" // [URL](https://github.com/gavinlyonsrepo/ER_OLEDM1_CH1115) v1.3
#include <RH_ASK.h> // [URL](https://www.airspayce.com/mikem/arduino/RadioHead/)
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

//******************** GLOBALS + DEFINES********************

// Serial
//#define RF_SERIAL_DEBUG_ON // comment in for serial debug
#define RF_SERIAL_BAUD 38400

// Test timing
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to update testcount (milliseconds)
uint8_t TestCount = 0;
#define READSENSOR 30 // sensor internal polled seconds (number of testcounts intervals)
#define INITDELAY  1000   // initial delay mS

// LM75 sensor
M2M_LM75A lm75a;

// OLED
#define OLEDcontrast 0x80 //Contrast 00 to FF , 0x80 is default. user adjust
#define MYOLEDHEIGHT 64
#define MYOLEDWIDTH 128
#define OLED_DC 10 // GPIO pin number pick any you want 
#define OLED_RES 9 // "
#define OLED_CS 8  // "
#define OLED_SCLK 4 // "
#define OLED_SDATA 5 // "
ERMCH1115  myOLED(OLED_DC, OLED_RES, OLED_CS, OLED_SCLK, OLED_SDATA);  // GPIO 5-wire Software SPI interface


// RF 433 mHz ask
RH_ASK driver;

char bufRX[21];
uint8_t bufRXlen = sizeof(bufRX);
uint16_t RXCount = 0;

// ************ Function Headers ***********************
void DisplayInternal(MultiBuffer* );
void DisplayExternal(MultiBuffer* , bool );

//******************** SETUP LOOP************************
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

} // End of Setup


//******************* MAIN LOOP *****************
void loop() {

  //define a buffer to cover half  screen
  uint8_t  screenBuffer[(MYOLEDWIDTH * (MYOLEDHEIGHT / 8)) / 2]; //(128 * 8)/2 = 512 bytes

  // Define half size screen buffer for top side of OLED
  MultiBuffer top_side;
 // Intialise that struct with buffer details (&struct,  buffer, w, h, x-offset,y-offset)
  myOLED.OLEDinitBufferStruct(&top_side, screenBuffer, MYOLEDWIDTH, MYOLEDHEIGHT/2, 0, 0);
  
   // Define half size screen buffer for Bottom side of OLED
  MultiBuffer bot_side;
  // Intialise that struct with buffer details (&struct,  buffer, w, h, x-offset,y-offset)
  myOLED.OLEDinitBufferStruct(&bot_side, screenBuffer, MYOLEDWIDTH, MYOLEDHEIGHT/2,  0, MYOLEDHEIGHT/2);
  
  //first pass display
  DisplayInternal(&top_side);
  DisplayExternal(&bot_side, true);

  while (true) // Test loop forever
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

    if (driver.recv(bufRX, &bufRXlen)) // Non-blocking, Did data come from transmitter?
    {
      RXCount++;
      if (RXCount == 9999) RXCount = 0;
      DisplayExternal(&bot_side, false);
    }

  }// test while

} // END of main


// ********************* FUNCTIONS SPACE ************

// Func Desc ::  function to display the external data on OLED
// Param1 A pointer to the  OLED buffer struct
// Param2 bool if true just display text and no data.

void DisplayExternal(MultiBuffer* targetbuffer, bool NoDataDisplay)
{
  Serial.print(RXCount);
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


  // Message with a good checksum received, dump it.
#ifdef RF_SERIAL_DEBUG_ON
  driver.printBuffer("RX data Got:", bufRX, bufRXlen);
#endif
  char *token;
  uint8_t LoopCount = 1;
  char str_temp[7];
  char str_humid[6];
  char str_batVolts[5];

  // Spilt the incoming char array RX msg uisng strtok
  token = strtok (bufRX, ",");
  while (token  != NULL)
  {
    switch (LoopCount)
    {
      case 1: sprintf(str_temp, token); break;
      case 2: sprintf(str_humid, token); break;
      case 3: sprintf(str_batVolts, token); break;
    }
    LoopCount++;
    token = strtok (NULL, ",");
  }

#ifdef RF_SERIAL_DEBUG_ON
  Serial.print("Humidity: ");
  Serial.println(str_humid);
  Serial.print("Temperature: ");
  Serial.println(str_temp);
  Serial.print("Battery Voltage: ");
  Serial.println(str_batVolts);
#endif

  myOLED.setCursor(0, 14);
  if (atoi(str_temp) == 255 || atoi(str_humid) == 255)
  {
    myOLED.print("Sensor error");
  } else if ((atoi(str_temp) <= -50) || (atoi(str_temp) >= 95) )
  {
    myOLED.print("Out of range T");
  } else if ((atoi(str_humid) <= -5) || (atoi(str_humid) >= 105) )
  {
    myOLED.print("Out of range H");
  }
  else
  {
    myOLED.setTextSize(2);
    myOLED.print(str_temp);
    myOLED.setCursor(0, 0);
    myOLED.setTextSize(1);
    myOLED.setCursor(65, 22);
    myOLED.print(" C ");
    myOLED.print(str_humid);
    myOLED.print("%");
    myOLED.setCursor(90, 12);
    myOLED.print(str_batVolts);
    myOLED.print("v");
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
    myOLED.setTextSize(1);
    myOLED.setCursor(70, 22);
    myOLED.print(" C");
  }

  myOLED.OLEDupdate();  //write to active buffer

}

//******************* EOF *****************
