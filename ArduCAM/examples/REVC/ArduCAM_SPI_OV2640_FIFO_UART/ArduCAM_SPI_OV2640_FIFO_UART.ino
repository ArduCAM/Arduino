// ArduCAM demo (C)2014 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules.
//
// This demo was made for Omnivision OV2640 sensor.
// 1. Set the sensor to JPEG output mode.
// 2. Capture and buffer the image to FIFO. 
// 3. Transfer the captured JPEG image back to host via Arduino board USB port.
// 4. Resolution can be changed by myCAM.OV2640_set_JPEG_size() function.
// This program requires the ArduCAM V3.0.1 (or above) library and Rev.C ArduCAM shield
// and use Arduino IDE 1.5.2 compiler

#include <UTFT_SPI.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"

// set pin 10 as the slave select for the digital pot:
const int SPI_CS = 10;


ArduCAM myCAM(OV2640,10);
UTFT myGLCD(SPI_CS);

void setup()
{
  uint8_t vid,pid;
  uint8_t temp;
  #if defined (__AVR__)
    Wire.begin(); 
  #endif
  #if defined(__arm__)
    Wire1.begin(); 
  #endif
  Serial.begin(115200);

  Serial.println("ArduCAM Start!"); 

  // set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
  	Serial.println("SPI interface Error!");
        Serial.println(temp,HEX);
  	while(1);
  }
  
  myCAM.set_mode(MCU2LCD_MODE);

  //Check if the camera module type is OV2640
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if((vid != 0x26) || (pid != 0x42))
  	Serial.println("Can't find OV2640 module!");
  else
  	Serial.println("OV2640 detected");
  
  //Change to JPEG capture mode and initialize the OV2640 module	
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
}

void loop()
{
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;

  temp = Serial.read();
  switch(temp)
  {
    case 0:
      myCAM.OV2640_set_JPEG_size(OV2640_160x120);
      break;
    case 1:
      myCAM.OV2640_set_JPEG_size(OV2640_176x144);
      break;
    case 2:
      myCAM.OV2640_set_JPEG_size(OV2640_320x240);
      break;
    case 3:
      myCAM.OV2640_set_JPEG_size(OV2640_352x288);
      break;
    case 4:
      myCAM.OV2640_set_JPEG_size(OV2640_640x480);
      break;
    case 5:
      myCAM.OV2640_set_JPEG_size(OV2640_800x600);
      break;
    case 6:
      myCAM.OV2640_set_JPEG_size(OV2640_1024x768);
      break;
    case 7:
      myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);
      break;
    case 8:
      myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
      break;
      
    case 0x10:
      start_capture = 1;
      Serial.println("Start Capture"); 
      myCAM.flush_fifo();
      break;
    default:
      break;
  }
  //start_capture = 1;
  if(start_capture)
  {
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();	 
    //Start capture
    myCAM.start_capture();	 
  }
  if(myCAM.get_bit(ARDUCHIP_TRIG,CAP_DONE_MASK))
  {
    Serial.println("Capture Done!");

    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
        temp_last = temp;
	temp = myCAM.read_fifo();
	Serial.write(temp);
    }
  

    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
    start_capture = 0;
  }
}

   


