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

An RF data link between Two ATMEGA328p's ,
Sensor data is transferred Via a 433 Mhz transmitter.
and then displayed on a OLED CH1115 at Receiver end.

* Author: Gavin Lyons
* URL: https://github.com/gavinlyonsrepo/RF_Data_link
* Date: Nov 2021 

Features
---------------------

Set up a RF data link between two ATMEGA328's.
Project uses WL102-341 transmitter and WL101-341 receiver  , 433 MHz ASK modulation RF Link.

The transmitter has a AHT10 Sensor it sends data once a minute and then the TX ATMEGA328 goes to sleep mode.

The receiver displays this data from transmitter and as well as its own sensor data LM75A
on an OLED CH1115 display. Software SPI is used to drive OLED.

In addition to transmitted sensor data . The Voltage supply of the transmitter is also sent so
battery voltage of transmitter unit can be monitored and a count of receives is displayed.

  
 ![4](https://github.com/gavinlyonsrepo/RF_DATA_LINK/blob/main/image/433.jpg) 
  
Files 
---------------------

1. RF_data_link.ino receiver side
2. RF_data_link_TX.ino transmitter side

Libraries used are in comments of HW connection header section of ino files. 

Schematic 
-----------------------

Transmitter 

![t](https://github.com/gavinlyonsrepo/RF_DATA_LINK/blob/main/image/tx.png)

Receiver 

![r](https://github.com/gavinlyonsrepo/RF_DATA_LINK/blob/main/image/rx.png)


TODO
--------------------

Final testing.

PCB design + final build. 
