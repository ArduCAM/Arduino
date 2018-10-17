# Introduction
**ArduCAM Library** is an open source code library that supports the [ArduCAM](http://www.arducam.com/) series of still and video camera products for the Arduino and Raspberry Pi (RPi) platforms.

ArduCAM products come in two basic forms. The **ArduCAM shield** contains a hardware driver that interfaces with your own camera sensor modules. The **ArduCAM mini** combines the ArduCAM shield with one of a number of popular camera sensors to produce an all-in-one, plug-and-play camera shield.

The ArduCAM system relies on the [Serial Peripheral Interface](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface), or SPI, to connect to its host platform. In theory, any ArduCAM product should work with any system that supports SPI and [I2C](https://en.wikipedia.org/wiki/I%C2%B2C). The ArduCAM Library has been designed to be ported to any Arduino or RPi platform that includes these interfaces.

## Currently supported cameras
-	OV7660		0.3MP
-	OV7670		0.3MP
-	OV7675		0.3MP
-	OV7725		0.3MP
-	MT9V111		0.3MP
-	MT9M112		1.3MP	
-	MT9M001		1.3MP 	
-	MT9D111		2MP
-	OV2640		2MP	JPEG
-	MT9T112		3MP
-	OV3640		3MP
-	OV5642		5MP	JPEG
-	OV5640		5MP JPEG

## Supported MCU platforms
ArduCAM theoretically supports all Arduino families, but has been tested and is known to work on the following platforms:

-	Arduino UNO R3
-	Arduino MEGA2560 R3
-	Arduino Leonardo R3
-	Arduino Nano
-	Arduino DUE
-	Arduino Genuion 101
-	Feather M0
-	ESP8266-12 (see notes below)
-	Raspberry Pi

**Note:** The ArduCAM Library for ESP8266 is maintained in another repository [ESP8266](https://github.com/ArduCAM/ArduCAM_ESP8266_UNO) using the [json board manager script](http://www.arducam.com/downloads/ESP8266_UNO/package_ArduCAM_index.json).

## Library structure
The ArduCAM Library code is organized into two sub-libraries; the first is "ArduCAM" and the second is "UTFT4ArduCAM_SPI". In order to be recognized by the Arduino IDE, these two libraries should be [copied into the library directory](https://www.arduino.cc/en/Guide/Libraries) for your sketchbook.

The "ArduCAM" library is the core library for ArduCAM shields. It contains the supported image sensor drivers and user-land API functions used to issue capture or image data read commands. There is also an example directory inside the ArduCAM library which illustrates most functions of the ArduCAM shields. The examples are plug-and-play without the need to write a single line of code.

The "UTFT4ArduCAM\_SPI" library includes a modified version of UTFT by [Henning Karlsen](http://www.henningkarlsen.com/electronics). It combines the basic ArduCAM library with UTFT in order to support the [ArduCAM-LF](https://www.robotshop.com/ca/en/arducam-lf-revc-camera-module-32-lcd-arduino.html), an ArduCAM shield with a built-in LCD display. UTFT4ArduCAM\_SPI is only needed when working with the ArduCAM-LF.

# Compiling the libraries
There is some basic configuration required before the libraries can be successfully compiled.

## 1. Edit memorysaver.h file
Open the `memorysaver.h` file in the ArduCAM folder and enable the hardware platform and camera module which matches to your hardware by commenting or uncommenting the right macro definition in the file.

For example, if you have an ArduCAM-Mini-2MP you 
should uncomment the line:

`#define OV2640_MINI_2MP`

and comment-out all the other lines. On the other hand, if you have an ArduCAM-Shield-V2 and a OV5642 camera module, you should uncomment these two lines:

`#define ARDUCAM_SHIELD_V2`  
`#define OV5642_CAM`

and comment-out all the other lines. The simplest way to comment out a line is to add two slashes to the front of the line, "//".

## 2. Choose the correct CS pin for your camera
Open one of the examples, wiring SPI and I2C interface especially CS pins to ArduCAM shield according to the examples.
Hardware and software should be consistent to run the examples correctly.

## 3. Upload the examples
In the example folder there are seven sub-directories for different ArduCAM models and the host application.

The `Mini` folder is for ArduCAM-Mini-2MP and ArduCAM-Mini-5MP modules.  
The `Mini_5MP_Plus` folder is for ArduCAM-Mini-5MP-Plus (OV5640/OV5642) modules.  
The `RevC` folder is for ArduCAM-Shield-RevC or ArduCAM-Shield-RevC+ shields.  
The `Shield_V2` folder is for ArduCAM-Shield-V2 shield.  
The `host_app` folder is host capture and display application for all of ArduCAM modules.  
The `RaspberryPi` folder is examples used for Raspberry Pi platform, see [more instruction](https://github.com/ArduCAM/Arduino/tree/master/ArduCAM/examples/RaspberryPi).  
The `ESP8266` folder is for ArduCAM-ESP8266-UNO board examples for library compatibility.

If you are using the ESP8266, please use the  [ESP8266](https://github.com/ArduCAM/ArduCAM_ESP8266_UNO) repository instead, which uses the [json board manager script](http://www.arducam.com/downloads/ESP8266_UNO/package_ArduCAM_index.json).

To use these examples, simply edit the code to select the correct COM port and Arduino board, and then upload the sketch to your board.

# Video tutorials
There are two video tutorials available on YouTube:

## ArduCAM mini tutorial for Arduino
[![IMAGE ALT TEXT](https://github.com/UCTRONICS/pic/blob/master/Arducam_MINI_Camera.jpeg)](https://youtu.be/hybQpjwJ4aA "Arducam MINI Camera Demo Tutorial for Arduino")

## Arducam shield tutorial for Arduino
[![IMAGE ALT TEXT](https://github.com/UCTRONICS/pic/blob/master/Arducam_Shield_V2_Camera.jpeg)](https://youtu.be/XMik38TNqGk "Arducam MINI Camera Demo Tutorial for Arduino")

