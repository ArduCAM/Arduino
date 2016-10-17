// ArduCAM Mini demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// This demo was made for ArduCAM_Mini_5MP_Plus.
// It needs to be used in combination with PC software.
// It can take photo continuously as video streaming.
// The demo support low power mode.
// The demo sketch will do the following tasks:
// 1. Set the camera to JEPG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x10,camera will capture a JPEG photo and buffer the image to FIFO.Then write datas to Serial port.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini_5MP_Plus
// and use Arduino IDE 1.5.2 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5640_MINI_5MP_PLUS)||defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
// set pin 7 as the slave select for the digital pot:
const int CS = 7;
#if defined (OV5640_MINI_5MP_PLUS)
ArduCAM myCAM(OV5640, CS);
#else
ArduCAM myCAM(OV5642, CS);
#endif
void setup() {
  uint8_t vid,pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
  Serial.begin(115200);
#else
  Wire.begin();
  Serial.begin(921600);
#endif
  Serial.println("ACK CMD ArduCAM Start!"); 
  // set the SPI_CS as an output:
  pinMode(CS, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
    Serial.println("ACK CMD SPI1 interface Error!");
    Serial.println("ACK CMD Check your wiring, make sure using the correct SPI port and chipselect pin");
    //while(1);
  }

  #if defined (OV5640_MINI_5MP_PLUS)
    //Check if the camera module type is OV5640
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40))
      Serial.println("ACK CMD Can't find OV5640 module!");
    else
      Serial.println("ACK CMD OV5640 detected.");
  #else
    //Check if the camera module type is OV5642
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42))
      Serial.println("ACK CMD Can't find OV5642 module!");
    else
      Serial.println("ACK CMD OV5642 detected.");
  #endif
  
  //Change to JPEG capture mode and initialize the OV5642 module  
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//enable low power
}

void loop() {
  uint8_t temp = 0xff,temp_last = 0;
  uint8_t start_capture = 0;

  temp = Serial.read();
  switch(temp)
  {
    case 0:
    temp = 0xff;
     myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
     #if defined (OV5640_MINI_5MP_PLUS)
      myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
      #else
       myCAM.OV5642_set_JPEG_size(OV5642_320x240);
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
       #endif
        break;
      case 1:
      temp = 0xff;
       myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
       #if defined (OV5640_MINI_5MP_PLUS)
       myCAM.OV5640_set_JPEG_size(OV5640_352x288);delay(1000);
      #else
       myCAM.OV5642_set_JPEG_size(OV5642_640x480);      
      #endif
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 2:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
       #if defined (OV5640_MINI_5MP_PLUS)        
       myCAM.OV5640_set_JPEG_size(OV5640_640x480);delay(1000);
       #else
        myCAM.OV5642_set_JPEG_size(OV5642_1280x960);      
       #endif
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 3:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      temp = 0xff;
      #if defined (OV5640_MINI_5MP_PLUS)    
       myCAM.OV5640_set_JPEG_size(OV5640_800x480);delay(1000);
       #else
        myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);      
       #endif
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 4:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #if defined (OV5640_MINI_5MP_PLUS)      
       myCAM.OV5640_set_JPEG_size(OV5640_1024x768);delay(1000);
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);        
      #endif
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
       break;
      case 5:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #if defined (OV5640_MINI_5MP_PLUS)   
       myCAM.OV5640_set_JPEG_size(OV5640_1280x960);delay(1000);
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);    
        #endif
         myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
       #if defined (OV5640_MINI_5MP_PLUS)
        case 6:
        temp = 0xff;
        myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
        myCAM.OV5640_set_JPEG_size(OV5640_1600x1200);delay(1000);
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 7:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
        myCAM.OV5640_set_JPEG_size(OV5640_2048x1536);delay(1000);
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 8:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
        myCAM.OV5640_set_JPEG_size(OV5640_2592x1944);delay(1000);
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      #endif
    case 0x10:
    temp = 0xff;
      start_capture = 1;  
      Serial.println("ACK CMD CAM start single shot.");
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//disable low power  
      delay(800);
      break;
    default:
      break;
  }
  if(start_capture == 1)
  {
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();   
    //Start capture
    myCAM.start_capture();
    start_capture = 0;
  }
  if(myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
    Serial.println("ACK CMD CAM Capture Done!");
    temp = 0;
      Serial.println("ACK IMG");
    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
      temp_last = temp;
      temp = myCAM.read_fifo();
      Serial.write(temp);
      delayMicroseconds(10);
    }
    //Clear the capture done flag 
    myCAM.clear_fifo_flag(); 
  }
}
