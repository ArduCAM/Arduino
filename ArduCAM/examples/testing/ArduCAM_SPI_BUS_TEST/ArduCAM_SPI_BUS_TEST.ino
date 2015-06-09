// ArduCAM demo (C)2013 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to test the ArduCAM shield SPI communication
// to check out if bus has errors.
//
// This demo was made for Omnivision OV5642 sensor.
// 1. Write ArduCHIP internal test registers.
// 2. Read out ArduCHIP internal test registers and send back to Serial Monitor
// 3. Read out ArduCHIP internal Revision registers and send back to Serial Monitor
// This program requires the ArduCAM V3.0.0 (or above) library and Rev.C ArduCAM shield
// and use Arduino IDE 1.5.2 compiler


#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>

// set pin 10 as the slave select for the ArduCAM shield:
const int slaveSelectPin = 10;


ArduCAM myCAM(OV7670,slaveSelectPin);

void setup()
{
  Serial.begin(115200);
  Serial.println("Test START..."); 
  // initialize SPI:
  SPI.begin(); 
}

void loop()
{
  uint8_t temp1,temp2,revision;
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);		 //Write to test1 register by 0x55
  myCAM.write_reg(ARDUCHIP_TEST2, 0xAA);		 //Write to test1 register by 0xaa
  delay(1000);
  temp1 = myCAM.read_reg(ARDUCHIP_TEST1);                //Read from test1 register 
  temp2 = myCAM.read_reg(ARDUCHIP_TEST2);                //Read from test1 register
  Serial.write(temp1);
  Serial.write(temp2);
  delay(1000);
  myCAM.write_reg(ARDUCHIP_TEST1, 0xAA);		 //Write to test1 register by 0x55
  myCAM.write_reg(ARDUCHIP_TEST2, 0x55);		 //Write to test1 register by 0xaa
  delay(1000);
  temp1 = myCAM.read_reg(ARDUCHIP_TEST1);                //Read from test1 register 
  temp2 = myCAM.read_reg(ARDUCHIP_TEST2);                //Read from test1 register
  Serial.write(temp1);
  Serial.write(temp2);
  delay(1000);
  revision = myCAM.read_reg(ARDUCHIP_REV);                   //Read from REV register
  Serial.write(revision);
  delay(1000);
  
}

   


