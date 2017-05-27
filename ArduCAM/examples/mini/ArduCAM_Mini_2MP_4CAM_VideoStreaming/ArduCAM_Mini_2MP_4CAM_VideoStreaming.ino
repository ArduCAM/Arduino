// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 2MP camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini Camera.
// It needs to be used in combination with PC software.It can take 4 photos at the same time with 4 cameras.
// The demo sketch will do the following tasks:
// 1. Set the 4 cameras to JPEG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x15,cameras will capture and buffer the image to FIFO. 
// 5. Check the CAP_DONE_MASK bit and write datas to Serial port.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM Minicamera
// and use Arduino IDE 1.6.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP|| defined OV3640_MINI_3MP)
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

// set pin 4,5,6,7 as the slave select for SPI:
const int CS1 = 4;
const int CS2 = 5;
const int CS3 = 6;
const int CS4 = 7;

//the falgs of camera modules
bool cam1=true,cam2=true,cam3=true,cam4=true;
//the flag of JEPG data header
bool is_header;
//the falg data of 4 cameras' data
byte flag[5]={0xFF,0xAA,0x01,0xFF,0x55};
int count = 0;
#if defined (OV2640_MINI_2MP)
ArduCAM myCAM1(OV2640, CS1);
ArduCAM myCAM2(OV2640, CS2);
ArduCAM myCAM3(OV2640, CS3);
ArduCAM myCAM4(OV2640, CS4);
#elif defined (OV3640_MINI_3MP)
ArduCAM myCAM1(OV3640, CS1);
ArduCAM myCAM2(OV3640, CS2);
ArduCAM myCAM3(OV3640, CS3);
ArduCAM myCAM4(OV3640, CS4);
#else
ArduCAM myCAM1(OV5642, CS1);
ArduCAM myCAM2(OV5642, CS2);
ArduCAM myCAM3(OV5642, CS3);
ArduCAM myCAM4(OV5642, CS4);
#endif

void setup() {
// put your setup code here, to run once:
uint8_t vid,pid;
uint8_t temp;
Wire.begin(); 
Serial.begin(921600);
Serial.println("ArduCAM Start!"); 
// set the CS output:
pinMode(CS1, OUTPUT);
pinMode(CS2, OUTPUT);
pinMode(CS3, OUTPUT);
pinMode(CS4, OUTPUT);

// initialize SPI:
SPI.begin(); 
while(1){
  //Check if the 4 ArduCAM Mini 2MP Cameras' SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("ACK CMD SPI1 interface Error!"));
    cam1 = false;
  } 
  myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM2.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("ACK CMD SPI2 interface Error!"));
    cam2 = false;
  }
    
  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("ACK CMD SPI3 interface Error!"));
    cam3 = false;
  }
    
  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("ACK CMD SPI4 interface Error!"));
    cam4 = false;
  }
  if(!(cam1||cam2||cam3||cam4)){
    delay(1000);continue;
    }else
    break;
}
#if defined (OV2640_MINI_2MP)
  while(1){
  //Check if the camera module type is OV2640
  myCAM1.wrSensorReg8_8(0xff, 0x01);
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println(F("ACK CMD Can't find OV2640 module!"));
    delay(1000);continue;
  }else{
    Serial.println(F("ACK CMD OV2640 detected."));break;
   } 
}
#elif defined (OV3640_MINI_3MP)
  while(1){
    //Check if the camera module type is OV3640
    myCAM1.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
    if ((vid != 0x36) || (pid != 0x4C)){
      Serial.println(F("Can't find OV3640 module!"));
      delay(1000);continue; 
    }else{
      Serial.println(F("OV3640 detected."));break;    
    }
 } 
#else
  while(1){
    //Check if the camera module type is OV5642
    myCAM1.wrSensorReg16_8(0xff, 0x01);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42)){
    Serial.println(F("ACK CMD Can't find OV5642 module!"));
    delay(1000);continue;
    }else{
    Serial.println(F("ACK CMD OV5642 detected."));break;
    } 
  }  
#endif
myCAM1.set_format(JPEG);
myCAM1.InitCAM();
#if defined (OV2640_MINI_2MP)
myCAM1.OV2640_set_JPEG_size(OV2640_320x240);
#elif defined (OV3640_MINI_3MP)
  myCAM1.OV3640_set_JPEG_size(OV3640_320x240);
#else
myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM2.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM3.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM4.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM1.OV5642_set_JPEG_size(OV5642_320x240);
#endif
delay(1000);
myCAM1.clear_fifo_flag();
myCAM2.clear_fifo_flag();
myCAM3.clear_fifo_flag();
myCAM4.clear_fifo_flag();
}

void loop() {
// put your main code here, to run repeatedly:
uint8_t temp = 0xff, temp_last = 0;
uint8_t start_capture = 0;
uint8_t finish_count;
if (Serial.available()){
temp = Serial.read();
switch(temp)
{
 case 0:
    #if defined (OV2640_MINI_2MP)
      myCAM.OV2640_set_JPEG_size(OV2640_160x120);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_160x120"));
    #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_176x144);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_160x120"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_320x240"));
    #endif
    temp = 0xff;
    break;
    case 1:
    #if defined (OV2640_MINI_2MP)
      myCAM1.OV2640_set_JPEG_size(OV2640_176x144);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_176x144"));
    #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_320x240"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_640x480"));
    #endif
    temp = 0xff;
    break;
    case 2: 
    #if defined (OV2640_MINI_2MP)
      myCAM1.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_320x240"));
     #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_352x288);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_352x288"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_1024x768"));
    #endif
    temp = 0xff;
    break;
    case 3:
    temp = 0xff;
    #if defined (OV2640_MINI_2MP)
      myCAM1.OV2640_set_JPEG_size(OV2640_352x288);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_352x288"));
    #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_640x480"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_1280x960);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_1280x960"));
    #endif
    break;
    case 4:
    temp = 0xff;
    #if defined (OV2640_MINI_2MP)
      myCAM1.OV2640_set_JPEG_size(OV2640_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_640x480"));
   #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_800x600);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_800x600"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_1600x1200"));
    #endif
    break;
    case 5:
    temp = 0xff;
    #if defined (OV2640_MINI_2MP)
      myCAM1.OV2640_set_JPEG_size(OV2640_800x600);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_800x600"));
    #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1024x768"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_2048x1536"));
    #endif
    break;
    case 6:
    temp = 0xff;
    #if defined (OV2640_MINI_2MP)
      myCAM1.OV2640_set_JPEG_size(OV2640_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_1024x768"));
   #elif defined (OV3640_MINI_3MP)
      myCAM1.OV3640_set_JPEG_size(OV3640_1280x960);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1280x960"));
    #else
      myCAM1.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_2592x1944"));
    #endif
    break;
    case 7:
    temp = 0xff;
   #if defined (OV2640_MINI_2MP)
    myCAM1.OV2640_set_JPEG_size(OV2640_1280x1024);delay(1000);
    Serial.println(F("ACK CMD switch to OV2640_1280x1024"));
   #else
      myCAM1.OV3640_set_JPEG_size(OV3640_1600x1200);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1600x1200"));
   #endif
    break;
    case 8:
    temp = 0xff;
   #if defined (OV2640_MINI_2MP)
     myCAM1.OV2640_set_JPEG_size(OV2640_1600x1200);delay(1000);
     Serial.println(F("ACK CMD switch to OV2640_1600x1200"));
   #else
     myCAM1.OV3640_set_JPEG_size(OV3640_2048x1536);delay(1000);
     Serial.println(F("ACK CMD switch to OV3640_2048x1536"));
   #endif
    break;
  case 0x10: 
  if(cam1){
    flag[2]=0x01;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM1);
  }
  if(cam2){
     flag[2]=0x02;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM2); 
  }
  if(cam3){
     flag[2]=0x03;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM3); 
  } 
  if(cam4){
    flag[2]=0x04;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM4);
  }  
  break;
  case 0x20: 
  while(1){
    if (Serial.available()){
      temp = Serial.read();
      if (temp == 0x21)break;
}
   if(cam1){
    flag[2]=0x01;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM1);
  }
  if(cam2){
     flag[2]=0x02;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM2); 
  }
  if(cam3){
     flag[2]=0x03;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM3); 
  } 
  if(cam4){
    
    flag[2]=0x04;//flag of cam1
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM4);
  }  
    }
    break;
    
  }
}
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
uint8_t temp,temp_last;
uint32_t length;

myCAM.flush_fifo(); 
myCAM.clear_fifo_flag();   
myCAM.start_capture(); 
while(!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
length = myCAM.read_fifo_length();
myCAM.CS_LOW();
myCAM.set_fifo_burst();
length--;
while( length-- )
{
  temp_last = temp;
  temp =  SPI.transfer(0x00);//read a byte from spi
  if(is_header == true)
  {
    Serial.write(temp);
  }
  else if((temp == 0xD8) & (temp_last == 0xFF))
  {
    Serial.println(F("ACK IMG"));
    is_header = true;
    Serial.write(temp_last);
    Serial.write(temp);
  }
  if( (temp == 0xD9) && (temp_last == 0xFF) )
  break;
  delayMicroseconds(15);
}
myCAM.CS_HIGH();
is_header = false;
//Clear the capture done flag 
myCAM.clear_fifo_flag();
}
