// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// This demo was made for ArduCAM_Mini platform.
// It needs to be used in combination with PC software.
// It can take photo continuously as video streaming.
// The demo support low power mode.
// The demo sketch will do the following tasks:
// 1. Set the camera to JPEG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x10,camera will capture a JPEG photo and buffer the image to FIFO.Then write datas to Serial port.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini 2MP or 5MP
// and use Arduino IDE 1.6.8 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP|| defined OV3640_MINI_3MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
const int SPI_CS = 7;
#if defined (OV2640_MINI_2MP)
  ArduCAM myCAM( OV2640, SPI_CS );
#elif defined (OV3640_MINI_3MP)
  ArduCAM myCAM( OV3640, SPI_CS );
#else
  ArduCAM myCAM( OV5642, SPI_CS );
#endif
void setup() {
uint8_t vid, pid;
uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
  Serial.begin(115200);
#else
  Wire.begin();
  Serial.begin(921600);
#endif
Serial.println(F("ACK CMD ArduCAM Start! END")); 
// set the SPI_CS as an output:
pinMode(SPI_CS, OUTPUT);
digitalWrite(SPI_CS, HIGH);
// initialize SPI:
SPI.begin(); 
  
//Reset the CPLD
myCAM.write_reg(0x07, 0x80);
delay(100);
myCAM.write_reg(0x07, 0x00);
delay(100);
  
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55){
    Serial.println(F("ACK CMD SPI interface Error! END"));
    delay(1000);continue;
  }else{
    Serial.println(F("ACK CMD SPI interface OK. END"));break;
  }
}
#if defined (OV2640_MINI_2MP)
  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println(F("ACK CMD Can't find OV2640 module! END"));
      delay(1000);continue;
    }
    else{
      Serial.println(F("ACK CMD OV2640 detected. END"));break;
    } 
  }
#elif defined (OV3640_MINI_3MP)
  while(1){
    //Check if the camera module type is OV3640
    myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
    if ((vid != 0x36) || (pid != 0x4C)){
      Serial.println(F("ACK CMD Can't find OV3640 module! END"));
      delay(1000);continue; 
    }else{
      Serial.println(F("ACK CMD OV3640 detected. END"));break;    
    }
 } 
#else
  while(1){
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("ACK CMD Can't find OV5642 module! END"));
      delay(1000);continue;
    }
    else{
     Serial.println(F("ACK CMD OV5642 detected. END"));break;
    } 
  }
#endif
//Change to JPEG capture mode and initialize the OV5642 module	
myCAM.set_format(JPEG);
myCAM.InitCAM();
#if defined (OV2640_MINI_2MP)
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
#elif defined (OV3640_MINI_3MP)
  myCAM.OV3640_set_JPEG_size(OV3640_320x240);
#else
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
#endif
myCAM.clear_fifo_flag();
#if !(defined (OV2640_MINI_2MP))
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
#endif
myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
}

void loop() {
uint8_t temp = 0xff, temp_last = 0;
uint8_t start_capture = 0;
temp = Serial.read();
switch(temp)
{
  case 0:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  #if defined (OV2640_MINI_2MP) 
    myCAM.OV2640_set_JPEG_size(OV2640_160x120);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_160x120 END")); 
 #elif defined (OV3640_MINI_3MP)
    myCAM.OV3640_set_JPEG_size(OV3640_176x144);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_160x120 END"));
  #else
    myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_320x240 END"));
  #endif 
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); 
  break;
  case 1:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  #if defined (OV2640_MINI_2MP)
    myCAM.OV2640_set_JPEG_size(OV2640_176x144);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_176x144 END"));
  #elif defined (OV3640_MINI_3MP)
      myCAM.OV3640_set_JPEG_size(OV3640_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_320x240 END"));       
  #else
    myCAM.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_640x480 END"));
  #endif
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  break;
  
  case 2:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  #if defined (OV2640_MINI_2MP)   
    myCAM.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_320x240 END")); 
  #elif defined (OV3640_MINI_3MP)
      myCAM.OV3640_set_JPEG_size(OV3640_352x288);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_352x288 END"));   
  #else   
    myCAM.OV5642_set_JPEG_size(OV5642_1024x768);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_1024x768 END"));  
  #endif 
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);  
  break;
  
  case 3:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  #if defined (OV2640_MINI_2MP)      
    myCAM.OV2640_set_JPEG_size(OV2640_352x288);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_352x288 END")); 
  #elif defined (OV3640_MINI_3MP)
      myCAM.OV3640_set_JPEG_size(OV3640_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_640x480 END"));     
  #else
    myCAM.OV5642_set_JPEG_size(OV5642_1280x960);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_1280x960 END"));
  #endif
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  break;
  
  case 4:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);   
  #if defined (OV2640_MINI_2MP)        
    myCAM.OV2640_set_JPEG_size(OV2640_640x480);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_640x480 END"));
 #elif defined (OV3640_MINI_3MP)
      myCAM.OV3640_set_JPEG_size(OV3640_800x600);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_800x600 END")); 
  #else
    myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_1600x1200 END"));
  #endif
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); 
  break;
  case 5:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); 
  #if defined (OV2640_MINI_2MP)
    myCAM.OV2640_set_JPEG_size(OV2640_800x600);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_800x600 END")); 
 #elif defined (OV3640_MINI_3MP)
      myCAM.OV3640_set_JPEG_size(OV3640_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1024x768 END")); 
  #else
    myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_2048x1536 END"));
  #endif
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  break;
  
  case 6:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);  
  #if defined (OV2640_MINI_2MP)  
    myCAM.OV2640_set_JPEG_size(OV2640_1024x768);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_1024x768 END")); 
 #elif defined (OV3640_MINI_3MP)
    myCAM.OV3640_set_JPEG_size(OV3640_1280x960);delay(1000);
    Serial.println(F("ACK CMD switch to OV3640_1280x960 END"));       
  #else
    myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
    Serial.println(F("ACK CMD switch to OV5642_2592x1944 END"));
  #endif
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); 
  break;
  case 7:
  temp = 0xff;
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  #if defined (OV2640_MINI_2MP)
    myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_1280x1024 END"));
  #else
    myCAM.OV3640_set_JPEG_size(OV3640_1600x1200);delay(1000);
    Serial.println(F("ACK CMD switch to OV3640_1600x1200 END"));
   #endif
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  break;
  case 8:
  temp = 0xff;
   #if defined (OV2640_MINI_2MP)
     myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);delay(1000);
     Serial.println(F("ACK CMD switch to OV2640_1600x1200"));
   #else
     myCAM.OV3640_set_JPEG_size(OV3640_2048x1536);delay(1000);
     Serial.println(F("ACK CMD switch to OV3640_2048x1536 END"));
   #endif
   myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
  break;
  case 0x10:
  temp = 0xff;
  start_capture = 1;  
  Serial.println(F("ACK CMD CAM start single shot. END"));
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
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
  Serial.println(F("ACK CMD CAM Capture Done. END"));delay(50);
  temp = 0;
  Serial.println(F("ACK IMG  END"));
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
