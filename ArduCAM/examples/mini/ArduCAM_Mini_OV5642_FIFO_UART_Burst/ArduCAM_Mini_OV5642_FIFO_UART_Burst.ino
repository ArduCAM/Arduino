// ArduCAM demo (C)2014 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules.
//
// This demo was made for Omnivision OV5642 sensor.
// 1. Set the sensor to JPEG output mode.
// 2. Capture and buffer the image to FIFO. 
// 3. Transfer the captured JPEG image back to host via Arduino board USB port.
// This program requires the ArduCAM V3.0.1 (or above) library and Rev.C ArduCAM shield
// and use Arduino IDE 1.5.2 compiler

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"

// set pin 10 as the slave select for the digital pot:
const int SPI_CS = 10;


ArduCAM myCAM(OV5642,SPI_CS);

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
    Serial.print(temp, HEX);
    while(1);
  }

  //myCAM.set_mode(MCU2LCD_MODE);

  //Check if the camera module type is OV2640
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x42))
    Serial.println("Can't find OV5642 module!");
  else
    Serial.println("OV5642 detected");

  //Change to JPEG capture mode and initialize the OV2640 module	
  myCAM.set_format(JPEG);

  myCAM.InitCAM();
  
  myCAM.clear_fifo_flag();
  //myCAM.write_reg(0x81, 0x00);
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00); //Bit[2:0]Number of frames to be captured
}

void loop()
{
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;
  static int Ts, Tc, Td, Tt, Tu, T0;
  T0 = millis();
  temp = Serial.read();
  Tu = millis();
  switch(temp)
  {
  case 0x0a:
    myCAM.set_bit(ARDUCHIP_TIM,LOW_POWER_MODE);
    break;
  case 0x0b:
    myCAM.clear_bit(ARDUCHIP_TIM,LOW_POWER_MODE);
    break;
  case 0x0c:
    myCAM.set_bit(ARDUCHIP_FIFO,FIFO_RDPTR_RST_MASK);
    read_fifo_again();
    break;
  case 0x0d:  //reset
    myCAM.set_bit(ARDUCHIP_GPIO,GPIO_RESET_MASK);
    myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_RESET_MASK);
    break;
  case 0x0e:  //pwdn low
    myCAM.set_bit(ARDUCHIP_GPIO,GPIO_POWER_MASK);
    break;   
  case 0x0f:  //pwdn
    myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_POWER_MASK);
    break;   
  case 0x10:
    start_capture = 1;
    Serial.println("Start Capture"); 
    Ts = millis();
    myCAM.flush_fifo();
    break;
  default:
    break;
  }

  if(start_capture)
  {
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();	 
    //Start capture
    myCAM.start_capture();
    start_capture = 0;	
    Tc = millis(); 
  }
  if(myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Td = millis();
    Serial.println("Capture Done!");
    read_fifo_burst();
    Tt = millis();
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
    start_capture = 0;
    Serial.println("");
    Serial.println("-----------------------");
    Serial.println("Time used:");
    Serial.println(Tu-T0, DEC);
    Serial.println(Tc-Ts, DEC);
    Serial.println(Td-Tc, DEC);
    Serial.println(Tt-Td, DEC);

  }
}

uint8_t read_fifo_burst()
{
    uint32_t length = 0;
    uint8_t temp,temp_last;
    length = myCAM.read_fifo_length();
    if(length >= 512000)  //512 kb
    {
      Serial.println("Not found the end.");
      return 0;
    }
    Serial.println(length);
    
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
    length--;
    while( length-- )
    {
	temp_last = temp;
	temp = SPI.transfer(0x00);
	Serial.write(temp);
	if( (temp == 0xD9) && (temp_last == 0xFF) )
		break;
	delayMicroseconds(10);
    }
    myCAM.CS_HIGH();
}

void read_fifo_again()
{
    read_fifo_burst();
}



