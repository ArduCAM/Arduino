#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

// set pin 10 as the slave select for the digital pot:
const int CS1 = 4;
int mode = 0;

ArduCAM myCAM1(OV2640,CS1);

void setup() {
  // put your setup code here, to run once:
  uint8_t vid,pid;
  uint8_t temp;
#if defined (__AVR__)
  Wire.begin(); 
#endif
#if defined(__arm__)
  Wire1.begin();
#endif 
  Serial.begin(921600);
  Serial.println("ArduCAM Start!"); 

  // set the SPI_CS as an output:
  pinMode(CS1, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
  	Serial.println("SPI1 interface Error!");
  	//while(1);
  }
  
  //myCAM.set_mode(MCU2LCD_MODE);

  //Check if the camera module type is OV2640
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);

  if((vid != 0x26) || (pid != 0x42))
  	Serial.println("Can't find OV2640 module!");
  else
  	Serial.println("OV2640 detected.");
  
  //Change to JPEG capture mode and initialize the OV2640 module	
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.OV2640_set_JPEG_size(OV2640_320x240);
  myCAM1.clear_fifo_flag();
  myCAM1.write_reg(ARDUCHIP_FRAMES,0x00);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;

  temp = Serial.read();
  switch(temp)
  {
    case 0:
      myCAM1.OV2640_set_JPEG_size(OV2640_160x120);
      break;
    case 1:
      myCAM1.OV2640_set_JPEG_size(OV2640_176x144);
      break;
    case 2:
      myCAM1.OV2640_set_JPEG_size(OV2640_320x240);
      break;
    case 3:
      myCAM1.OV2640_set_JPEG_size(OV2640_352x288);
      break;
    case 4:
      myCAM1.OV2640_set_JPEG_size(OV2640_640x480);
      break;
    case 5:
      myCAM1.OV2640_set_JPEG_size(OV2640_800x600);
      break;
    case 6:
      myCAM1.OV2640_set_JPEG_size(OV2640_1024x768);
      break;
    case 7:
      myCAM1.OV2640_set_JPEG_size(OV2640_1280x1024);
      break;
    case 8:
      myCAM1.OV2640_set_JPEG_size(OV2640_1600x1200);
      break;
      
    case 0x10:
      mode = 1;
      start_capture = 1;  
      Serial.println("CAM1 start single shot."); 
      break;
    case 0x20:
      mode = 2;
      start_capture = 2;
      Serial.println("CAM1 start contrues shots.");
      break;
    default:
      break;
  }
  if(mode==1)
  {
    if(start_capture == 1)
    {
      myCAM1.flush_fifo();
      myCAM1.clear_fifo_flag();	 
      //Start capture
      myCAM1.start_capture();
    }
    if(myCAM1.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    {
      Serial.println("CAM1 Capture Done!");
      temp = 0;
      while( (temp != 0xD9) | (temp_last != 0xFF) )
      {
        temp_last = temp;
  	temp = myCAM1.read_fifo();
  	Serial.write(temp);
        delayMicroseconds(10);
      }
      //Clear the capture done flag 
      myCAM1.clear_fifo_flag();
      start_capture = 0;
    }
  } 
  else if(mode == 2)
  {
    while(1)
    {
      temp = Serial.read();
      if(temp == 0x21)
      {
        start_capture = 0;
        mode = 0;
        Serial.println("CAM1 stop continuous shots!");
        break;
      }
      if(start_capture == 2)
      {
        myCAM1.flush_fifo();
        myCAM1.clear_fifo_flag();	 
        //Start capture
        myCAM1.start_capture();
        start_capture = 0;
      }
      if(myCAM1.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
      {
        //Serial.println("CAM1 Capture Done!");
        temp = 0;
        while( (temp != 0xD9) | (temp_last != 0xFF) )
        {
          temp_last = temp;
    	  temp = myCAM1.read_fifo();
    	  Serial.write(temp);
          delayMicroseconds(10);
        }
      
        start_capture = 2;
        //Clear the capture done flag 
        myCAM1.clear_fifo_flag();
      }
    }
  }
}
