//********************** HEADER ***********************
/*
  Name : RF_DATA_LINK_TX
  Title : RF data Link project :: Transmitter side
  Author: Gavin Lyons
  URL: https://github.com/gavinlyonsrepo/RF_DATA_LINK
  Date created: 2021-Nov
  IC: arduino barebones IC ATMEGA328P
*/

// ************** Connections ***********************
/*
  Transmitter ATMEGA328P
  1. I2C SDA = AHT10 SDA
  2. I2C SCLK = AHT10 SCLK
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
//#define RF_SERIAL_DEBUG_ON // Comment this in for serial debug information.
#define RF_SERIAL_BAUD 38400

//******************** GLOBALS ********************
// RF
RH_ASK driver;
uint8_t readStatus = 0;
#define INITDELAY 1000

// sensor AHT10
AHT10 myAHT20(AHT10_ADDRESS_0X38, AHT10_SENSOR);

// Sleep mode
Sleep sleep;
unsigned long sleepTime = 59000; //mS how long you want the arduino to sleep between transmit bursts

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

//*********** Function Headers ******************
float getBatteryVolts() ;

//******************* MAIN LOOP *****************
void loop()
{

  float temperatureAHT20 = 0.0;
  float humidityAHT20 = 0.0;
  float batteryVoltage = 0.0;

  char msgTX[20];
  char str_temp[7];
  char str_humid[6];
  char str_batvolt[5];

  temperatureAHT20 = myAHT20.readTemperature();
  humidityAHT20 = myAHT20.readHumidity();
  batteryVoltage = getBatteryVolts();

  dtostrf(temperatureAHT20, 6, 2, str_temp);
  dtostrf(humidityAHT20, 5, 2, str_humid);
  dtostrf(batteryVoltage, 4, 2, str_batvolt);

  strcpy(msgTX, str_temp);
  strcat(msgTX, ",");
  strcat(msgTX, str_humid);
  strcat(msgTX, ",");
  strcat(msgTX, str_batvolt);

  driver.send((uint8_t *)msgTX, strlen(msgTX));
  driver.waitPacketSent();


#ifdef RF_SERIAL_DEBUG_ON
  delay(100); // delays are just for serial print, without serial they can be removed
  Serial.print(F("T "));
  Serial.println( temperatureAHT20);
  Serial.print(F("H "));
  Serial.println(humidityAHT20);
  Serial.print(F("B "));
  Serial.println(batteryVoltage);
  delay(100);  //delay to allow serial to fully print before sleep
#endif

  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(sleepTime); //sleep for: sleepTime

} // END of MAIN


// ********************* FUNCTIONS ************

/* Func Desc: Get Vcc of  ATMega328p
  Param 1: returns float with Vcc i.e 5.04
  notes: You MUST measure the voltage at pin 21 (AREF) using just a simple one line sketch consisting
  of:  analogReference(INTERNAL) analogRead(A0)
  See: https://github.com/RalphBacon/Arduino-Battery-Monitor
  Then use the measured value here. at  const float InternalReferenceVoltage
  Normally this is 1.10 volts in theory.
*/
float getBatteryVolts() {


  const float InternalReferenceVoltage = 1.085; // <- as measured (or 1v1 by default)
  // turn ADC on
  ADCSRA =  bit (ADEN);
  // Prescaler of 128
  ADCSRA |= bit (ADPS0) |  bit (ADPS1) | bit (ADPS2);
  // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG) - Selects channel 14, bandgap voltage, to measure
  ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
  // let it stabilize
  delay (50);
  // start a conversion
  bitSet (ADCSRA, ADSC);
  // Wait for the conversion to finish
  while (bit_is_set(ADCSRA, ADSC))
  {
    ;
  }
  // Float normally reduces precion but works OK here. Add 0.5 for rounding not truncating.
  float results = InternalReferenceVoltage / float (ADC + 0.5) * 1024.0;
  return results;
}

//******************* EOF *****************
