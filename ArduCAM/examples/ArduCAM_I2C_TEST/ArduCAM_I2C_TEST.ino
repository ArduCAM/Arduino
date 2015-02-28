// ArduCAM demo (C)2013 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to communicate with camera modules
// via I2C interface and send read data back to Serial Monitor in Arduino IDE.
//
// This demo was made for Omnivision OV5642 sensor.
// 1. Receive commands from Serial Monitor 
// 2. Read Product ID from OV5642 registers
// 3. Send back to Serial Monitor. 
// This program requires the ArduCAM V3.0.0 (or above) library and Rev.C ArduCAM shield
// and use Arduino IDE 1.5.2 compiler

#include <UTFT_SPI.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>

// set pin 10 as the slave select for the ArduCAM shield:
const int slaveSelectPin = 10;

ArduCAM myCAM(OV5642,10);
UTFT myGLCD(slaveSelectPin);

void setup()
{
  Wire.begin(); 
  Serial.begin(115200);
  Serial.println("hello"); 

  // set the slaveSelectPin as an output:
  pinMode(slaveSelectPin, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  myCAM.write_reg(ARDUCHIP_MODE, 0x00);
  delay(2000);
  myGLCD.InitLCD();

  myCAM.set_format(JPEG);
  myCAM.InitCAM();

}

void loop()
{
  uint8_t temp;
  temp = Serial.read();
  switch(temp)
  {
    case 0x11:
      myCAM.rdSensorReg16_8(0x300a, &temp);
      Serial.write(temp);
      break;
    case 0x12:
      myCAM.rdSensorReg16_8(0x300b, &temp);
      Serial.write(temp);
      break;
    default:
      break;
  }
 
}

   


