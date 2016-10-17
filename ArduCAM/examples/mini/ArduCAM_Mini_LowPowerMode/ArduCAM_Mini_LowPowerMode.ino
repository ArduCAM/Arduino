// ArduCAM Mini demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// This demo was made for ArduCAM_Mini platform.
// It needs to be used in combination with PC software.
// It can take photo continuously as video streaming.
// The demo support low power mode.
// The demo sketch will do the following tasks:
// 1. Set the camera to JEPG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x10,camera will capture a JPEG photo and buffer the image to FIFO.Then write datas to Serial port.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini 2MP or 5MP
// and use Arduino IDE 1.5.2 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED
//or OV5640_MINI_5MP_PLUS or ARDUCAM_SHIELD_V2 platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

const int SPI_CS = 7;
#if defined (OV2640_MINI_2MP)
ArduCAM myCAM( OV2640, SPI_CS );
#else
ArduCAM myCAM( OV5642, SPI_CS );
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
  pinMode(SPI_CS, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
    Serial.println("ACK CMD SPI1 interface Error!");
    while(1);
  }

 #if defined (OV2640_MINI_2MP)
     //Check if the camera module type is OV2640
     myCAM.wrSensorReg8_8(0xff, 0x01);  
     myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
     myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
     if ((vid != 0x26) || (pid != 0x42))
      Serial.println("Can't find OV2640 module!");
     else
      Serial.println("OV2640 detected.");
  #else
   //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
     if((vid != 0x56) || (pid != 0x42))
     Serial.println("Can't find OV5642 module!");
     else
     Serial.println("OV5642 detected.");
  #endif
  
  //Change to JPEG capture mode and initialize the OV5642 module	
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  #if defined (OV2640_MINI_2MP)
   myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  #else
   myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
   myCAM.OV5642_set_JPEG_size(OV5642_320x240);
 #endif
  
  myCAM.clear_fifo_flag();
  #if !(defined (OV2640_MINI_2MP))
   myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
  #endif
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
    myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #if defined (OV2640_MINI_2MP)
        myCAM.OV2640_set_JPEG_size(OV2640_160x120);delay(1000);
        Serial.println("ACK CMD switch to OV2640_160x120");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
        Serial.println("ACK CMD switch to OV5642_320x240");
      #endif
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 1:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #if defined (OV2640_MINI_2MP)
         myCAM.OV2640_set_JPEG_size(OV2640_176x144);delay(1000);
         Serial.println("ACK CMD switch to OV2640_176x144");
      #else
         myCAM.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
         Serial.println("ACK CMD switch to OV5642_640x480");
      #endif
         myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 2:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #if defined (OV2640_MINI_2MP)
        myCAM.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
        Serial.println("ACK CMD switch to OV2640_320x240");
       #else
        myCAM.OV5642_set_JPEG_size(OV5642_1280x960);delay(1000);
        Serial.println("ACK CMD switch to OV5642_1280x960");
       #endif
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 3:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #if defined (OV2640_MINI_2MP)
        myCAM.OV2640_set_JPEG_size(OV2640_352x288);delay(1000);
        Serial.println("ACK CMD switch to OV2640_352x288");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);
        Serial.println("ACK CMD switch to OV5642_1600x1200");
      #endif
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 4:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #if defined (OV2640_MINI_2MP)
        myCAM.OV2640_set_JPEG_size(OV2640_640x480);delay(1000);
        Serial.println("ACK CMD switch to OV2640_640x480");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);
        Serial.println("ACK CMD switch to OV5642_2048x1536");
      #endif
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
       break;
      case 5:
      temp = 0xff;
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
     #if defined (OV2640_MINI_2MP)
        myCAM.OV2640_set_JPEG_size(OV2640_800x600);delay(1000);
        Serial.println("ACK CMD switch to OV2640_800x600");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
        Serial.println("ACK CMD switch to OV5642_2592x1944");
        #endif
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
       #if defined (OV2640_MINI_2MP)
        case 6:
        temp = 0xff;
         myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
         myCAM.OV2640_set_JPEG_size(OV2640_1024x768);delay(1000);
         Serial.println("ACK CMD switch to OV2640_1024x768");
         myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
        case 7:
        temp = 0xff;
        myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);delay(1000);
        Serial.println("ACK CMD switch to OV2640_1280x1024");
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      case 8:
      temp = 0xff;
       myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
       myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);delay(1000);
       Serial.println("ACK CMD switch to OV2640_1600x1200");
       myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        break;
      #endif
      case 0x10:
      temp = 0xff;
       start_capture = 1;  
      Serial.println("ACK CMD CAM1 start single shot.");
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
