//********************** HEADER ***********************
/*
  Name : RF_DATA_LINK_TX
  Title : RF data Link project :: Transmitter side
  Author: Gavin Lyons
  URL: https://github.com/gavinlyonsrepo/RF_DATA_LINK
  Date created: 2021-Nov
  IC: ATMEGA328P
*/

// ************** Connections ***********************
/*
Transmitter ATMEGA328P
1. I2C SDA = AHT20 SDA 
2. I2C SCLK = AHT20 SCLK
3. D12 = 433Mhz TX Data Out 
*/ 

//******************** LIBRARIES ******************
#include <AHT10.h>  // [URL](https://github.com/enjoyneering/AHT10)
#include <RH_ASK.h> [URL](https://www.airspayce.com/mikem/arduino/RadioHead/)
#include <Sleep_n0m1.h> //https://github.com/n0m1/Sleep_n0m1
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

//****************** DEFINES ****************
//#define RF_SERIAL_DEBUG_ON
#define RF_SERIAL_BAUD 9600

//******************** GLOBALS ********************
// RF
RH_ASK driver;
uint8_t readStatus = 0;
#define INITDELAY 1000

// sensor AHT20
AHT10 myAHT20(AHT10_ADDRESS_0X38, AHT20_SENSOR);

// Sleep mode
Sleep sleep;
unsigned long sleepTime = 40000; //mS how long you want the arduino to sleep between transmit bursts

//******************** SETUP ************************
void setup()
{
  delay(INITDELAY);

// Serial
#ifdef RF_SERIAL_DEBUG_ON
  while (!Serial) {}
  Serial.begin(RF_SERIAL_BAUD);
  Serial.println("===== TX DATA LINK UP =====");
#endif

#ifdef RH_HAVE_SERIAL
  Serial.begin(RF_SERIAL_BAUD);	  // Debugging only
#endif

// RF
  if (!driver.init())
#ifdef RH_HAVE_SERIAL
    Serial.println("init failed");
#else
  {}
#endif

// Sensor AHT20
  while (myAHT20.begin() != true)
  {
#ifdef RF_SERIAL_DEBUG_ON
    Serial.println(F("AHT20 not connected or fail to load calibration coefficient"));
#endif
    delay(5000);
  }

  delay(INITDELAY);
}

//******************* MAIN LOOP *****************
void loop()
{
  float temperatureAHT20 = 0.0;
  float humidityAHT20 = 0.0;
  String str_humid;
  String str_temp;
  String str_out;

  humidityAHT20 = myAHT20.readHumidity();  // Get Humidity value
  temperatureAHT20 = myAHT20.readTemperature();  // Get Temperature value

  str_humid = String(humidityAHT20);
  str_temp = String(temperatureAHT20);
  str_out = str_humid + "," + str_temp;
  char *msg = str_out.c_str();

  driver.send((uint8_t *)msg, strlen(msg));
  driver.waitPacketSent();

#ifdef RF_SERIAL_DEBUG_ON
  delay(100); // delays are just for serial print, without serial they can be removed
  Serial.print(F("Temperature: "));
  Serial.print( temperatureAHT20);
  Serial.print(F(" Humidity: "));
  Serial.println(humidityAHT20);
  delay(100);  //delay to allow serial to fully print before sleep
#endif

  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(sleepTime); //sleep for: sleepTime
} // END of main 


// ********************* FUNCTIONS ************

//******************* EOF *****************
