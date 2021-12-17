Overview
--------------------

* Title : RF_DATA_LINK
* Description : An RF data link between Two ATMEGA328p's 
Sensor data is transferred Via a 433 Mhz transmitter.
and then displayed on a OLED CH1115
* Author: Gavin Lyons
* URL: https://github.com/gavinlyonsrepo/RF_Data_link
* Date: Nov 2021 

Features
---------------------

Set up a RF data link between two ATMEGA328's.

Project uses WL102-341 transmitter and WL101-341 receiver  , 433 MHz ASK modulation RF Link.

The transmitter has a AHT20 Sensor it sends data every 40 seconds and then goes to sleep mode.

The receiver displays this data from transmitter and as well as its own sensor data LM75A
on a OLED CH1115 display. We use software SPI on the OLED as the RF library also uses hardware SPI.

Files 
---------------------

1. RF_data_link.ino receiver side
2. RF_data_link_TX.ino transmitter side

Libraries used are in comments of library section of ino files. 

Connections
------------------------

Connections used are in comments of library section of ino files. 


TODO
-----------------------

Schematic, PCB design + build.
