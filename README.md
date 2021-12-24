Table of contents
---------------------------

  * [Overview](#overview)
  * [Features](#features)
  * [Files](#files)
  * [Schematic](schematic) 

Overview
--------------------

* Title : RF_DATA_LINK
* Description : 

An low power RF data link between two Arduino based ATmega328p's units,
Sensor data is transferred via a 433 MHz RF data link
and then displayed on a OLED CH1115 at Receiver end.
Project uses WL102-341 transmitter and WL101-341 receiver  , 433 MHz ASK modulation RF Link.

* Author: Gavin Lyons
* URL: https://github.com/gavinlyonsrepo/RF_Data_link
* Date: Nov 2021 

Features
---------------------

**Receiver unit :**

The receiver displays the data from transmitter and as well as its own sensor data LM75A
on an OLED CH1115 display. Software SPI is used to drive OLED. 
In addition to transmitted sensor data, the voltage supply of the transmitter unit is also sent so battery voltage of transmitter unit can be monitored and a count of receives is displayed. Possible OLED ERROR Messages are in file in documentation section.
The receiver is currently based around an 
Arduino nano board will be powered from a PSU wall plug adaptor, may change this to a low power battery design with a lower power display at later date.
  
**Transmitter unit :**

The transmitter unit is battery powered and is based around  a "barebones" ATmega328 IC
A status LED turns on during power on setup and during each data burst very briefly.  The transmitter has an AHT10 Sensor it sends data once a minute and then it goes to sleep mode.  

1. Transmitter current consumption sleep period ~20uA
2. Transmitter current consumption wake period ~30mA 

The data packet sent :  XTT.TT,HH.HH,V.VV . X is + or - T is temperature, H is humdity and V is battery voltage , 17 bytes. 

 WL102-341 transmitter and WL101-341 receiver units used : 
  
 ![4](https://github.com/gavinlyonsrepo/RF_DATA_LINK/blob/main/image/433.jpg) 
  
Files 
---------------------

1. RF_data_link.ino : receiver side
2. RF_data_link_TX.ino : transmitter side

Libraries URL's used are listed in comments of library section of ino files. 

Schematic 
-----------------------

Transmitter unit :

![t](https://github.com/gavinlyonsrepo/RF_DATA_LINK/blob/main/image/tx.png)

Receiver unit:

![r](https://github.com/gavinlyonsrepo/RF_DATA_LINK/blob/main/image/rx.png)

