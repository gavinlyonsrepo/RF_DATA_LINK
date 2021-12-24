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
  1. A4 I2C SDA  , PC4 , 28 = AHT10 SDA
  2. A5 I2C SCLK , PC5 , 27 = AHT10 SCLK
  3. D12 ,PB4 , 18 433Mhz TX Data Out
  4. D5 , PD5 , 11 = Status LED
  5. D6 , PD6 , 12 = 433 Mhz Vcc enable
*/

//******************** LIBRARIES ******************
#include <AHT10.h>  // https://github.com/enjoyneering/AHT10
#include <RH_ASK.h> // https://www.airspayce.com/mikem/arduino/RadioHead/
#include "LowPower.h" // https://github.com/rocketscream/Low-Power
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

//******************GLOBALS + DEFINES ****************
// Serial optional debug only Comment in first line for serial debug information.
//#define RF_SERIAL_DEBUG_ON
#define RF_SERIAL_BAUD 38400

// Status LED
#define LED_D5_ON 5

// RF 433 Mhz module
RH_ASK driver;
uint8_t readStatus = 0;
#define INITDELAY 1000
#define EMIT_POWER_PIN 6

// sensor AHT10
AHT10 myAHT10(AHT10_ADDRESS_0X38, AHT10_SENSOR);

// Sleep mode
const uint16_t TimeToSleep = 59; // Set time to sleep (approx) in seconds, 59 for production code

//******************** SETUP ************************
void setup()
{
  //  LED and Transmitter ON
  outputPins_ON_OFF(true);
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

  // RF  433 Mhz
  if (!driver.init())
  {
#ifdef RH_HAVE_SERIAL
    Serial.println("init failed");
#endif
  }

  // Sensor AHT10
  while (myAHT10.begin() != true)
  {
#ifdef RF_SERIAL_DEBUG_ON
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient"));
#endif
  }

  delay(INITDELAY);
  digitalWrite(LED_D5_ON, LOW);
}

//*********** Function Headers ******************
float getBatteryVolts(void) ;
char * readAHT10Data(void);
void outputPins_ON_OFF(bool);

//******************* MAIN LOOP *****************
void loop()
{

  char* msgTx = readAHT10Data(); 

  outputPins_ON_OFF(true);   // begin emitting

  driver.send((uint8_t *)msgTx, strlen(msgTx));
  driver.waitPacketSent();

  outputPins_ON_OFF(false);   //d eactivate the transmitter + LED

  sleepSeconds(TimeToSleep);  // sleep for x seconds

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

// Func Desc , Puts Atmega328 to sleep
// Param 1 : uint16_t number of seconds to sleep for
void sleepSeconds(uint16_t seconds)
{
  for (uint16_t i = 0; i < seconds; i++) {
    // Enter power down state for 1s with ADC and BOD module disabled
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  }
}

// Func Desc  : turns on or Off status LED and transmitter
// Param 1 bool true turn on , false turn off
void outputPins_ON_OFF(bool outputPinsON)
{
  if (outputPinsON == true)
  {
    pinMode(LED_D5_ON, OUTPUT);
    digitalWrite(LED_D5_ON, HIGH);
    pinMode(EMIT_POWER_PIN, OUTPUT);
    digitalWrite(EMIT_POWER_PIN, HIGH);
  } else if (outputPinsON == false)
  {
    driver. setModeIdle();
    pinMode(EMIT_POWER_PIN, INPUT);
    digitalWrite(LED_D5_ON, LOW);
    pinMode(LED_D5_ON, INPUT);
  }
}


// Func Desc:
// Reads AHt10 sensor data + battery voltage 
// Returns: a pointer to a Char array with all data as such
// XTT.TT,HH.HH,V.VV 
// where X is " " or -  
char* readAHT10Data(void)
{
  float temperatureAHT10 = 0.0;
  float humidityAHT10 = 0.0;
  float batteryVoltage = 0.0;
  
  static char msgTX[20];
  char str_temp[7];
  char str_humid[6];
  char str_batvolt[5];

  temperatureAHT10 = myAHT10.readTemperature();
  humidityAHT10 = myAHT10.readHumidity();
  batteryVoltage = getBatteryVolts();

  dtostrf(temperatureAHT10, 6, 2, str_temp);
  dtostrf(humidityAHT10, 5, 2, str_humid);
  dtostrf(batteryVoltage, 4, 2, str_batvolt);

  strcpy(msgTX, str_temp);
  strcat(msgTX, ",");
  strcat(msgTX, str_humid);
  strcat(msgTX, ",");
  strcat(msgTX, str_batvolt);

#ifdef RF_SERIAL_DEBUG_ON
  delay(100); // delays are just for serial print, without serial they can be removed
  Serial.print(F("T "));
  Serial.println( temperatureAHT10);
  Serial.print(F("H "));
  Serial.println(humidityAHT10);
  Serial.print(F("B "));
  Serial.println(batteryVoltage);
  delay(100);  //delay to allow serial to fully print before sleep
#endif

return msgTX ; //&msgTX[0];
}

//******************* EOF *****************
