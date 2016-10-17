// ArduCAM Mini demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 5MP Plus camera, and can run on any Arduino platform.
//This demo support low power mode if you enable the LP.
// This demo was made for ArduCAM Mini 5MP Plus Camera.
// It needs to be used in combination with PC software.It can take 4 photos at the same time with 4 cameras.
// The demo sketch will do the following tasks:
// 1. Set the 4 cameras to JEPG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x15,cameras will capture and buffer the image to FIFO. 
// 5. Check the CAP_DONE_MASK bit and write datas to Serial port.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM Mini 2MP camera
// and use Arduino IDE 1.5.2 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//You can enable the LP to support the low power mode.
//Default is disable.

//#define LP

//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5640_MINI_5MP_PLUS)||defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
// set pin 4,5,6,7 as the slave select for SPI:
const int CS1 = 4;
const int CS2 = 5;
const int CS3 = 6;
const int CS4 = 7;

//the falgs of camera modules
bool cam1=true,cam2=true,cam3=true,cam4=true;
bool cam1done = false,cam2done = false,cam3done = false,cam4done = false;
//the flag of JEPG data header
bool is_header;
//the falg data of 4 cameras' data
byte flag[5]={0xFF,0xAA,0x01,0xFF,0x55};
int count = 0;
#if defined (OV5640_CAM)
ArduCAM myCAM1(OV5640,CS1);
ArduCAM myCAM2(OV5640,CS2);
ArduCAM myCAM3(OV5640,CS3);
ArduCAM myCAM4(OV5640,CS4);
#else
ArduCAM myCAM1(OV5642,CS1);
ArduCAM myCAM2(OV5642,CS2);
ArduCAM myCAM3(OV5642,CS3);
ArduCAM myCAM4(OV5642,CS4);
#endif
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

  // set the CS output:
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);
  pinMode(CS4, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the 4 ArduCAM Mini 2MP Cameras' SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI1 interface Error!");
    cam1 = false;
    //while(1);
  }
  
  myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM2.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI2 interface Error!");
    cam2 = false;
    //while(1);
  }
  
  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI3 interface Error!");
    cam3 = false;
    //while(1);
  }
  
  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI4 interface Error!");
    cam4 = false;
    //while(1);
  }

   #if defined (OV5640_MINI_5MP_PLUS)
    //Check if the camera module type is OV5640
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40))
      Serial.println("Can't find OV5640 module!");
    else
      Serial.println("OV5640 detected.");
  #else
    //Check if the camera module type is OV5642
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42))
      Serial.println("Can't find OV5642 module!");
    else
      Serial.println("OV5642 detected.");
  #endif
  
  //Set to JPEG capture mode and initialize the OV5640 module  
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
 #if defined (OV5640_MINI_5MP_PLUS)
  myCAM1.OV5640_set_JPEG_size(OV5640_320x240);//set 4 cams resolution
  #else
   myCAM1.OV5642_set_JPEG_size(OV5642_320x240);//set 4 cams resolution
  #endif
  myCAM1.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM2.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM3.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM4.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  
  myCAM1.clear_fifo_flag();
  myCAM2.clear_fifo_flag();
  myCAM3.clear_fifo_flag();
  myCAM4.clear_fifo_flag();
  #if defined (LP)
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//enable low power
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp=0;
  temp = Serial.read();
  switch(temp)
  {
     case 0:
        #if defined (LP)
          myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
        #endif
        #if defined (OV5640_CAM)
          myCAM1.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
          Serial.println("switch to OV5640_320x240");
        #else
          myCAM1.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
          Serial.println("switch to OV5642_320x240");
       #endif
       #if defined (LP)
          myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
       #endif
   break;
   case 1:
      #if defined (LP)
        myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      #if defined (OV5640_CAM)
        myCAM1.OV5640_set_JPEG_size(OV5640_352x288);delay(1000);
        Serial.println("switch to OV5640_352x288");
      #else
       myCAM1.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
       Serial.println("switch to OV5642_640x480");
     #endif
     #if defined (LP)
       myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
     #endif
  break;
  case 2:
      #if defined (LP)
        myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      #if defined (OV5640_CAM)
        myCAM1.OV5640_set_JPEG_size(OV5640_640x480);delay(1000);
        Serial.println("switch to OV5640_640x480");
      #else
        myCAM1.OV5642_set_JPEG_size(OV5642_1280x960);delay(1000);
        Serial.println("switch to OV5642_1280x960");
      #endif
      #if defined (LP)
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #endif
  break;
 case 3:
      #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      #if defined (OV5640_CAM)
       myCAM1.OV5640_set_JPEG_size(OV5640_800x480);delay(1000);
       Serial.println("switch to OV5640_800x480");
      #else
        myCAM1.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);
        Serial.println("switch to OV5642_1600x1200");
      #endif
      #if defined (LP)
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #endif
       break;
  case 4:
      #if defined (LP)
       myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      #if defined (OV5640_CAM)
       myCAM1.OV5640_set_JPEG_size(OV5640_1024x768);delay(1000);
       Serial.println("switch to OV5640_1024x768");
      #else
        myCAM1.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);
        Serial.println("switch to OV5642_2048x1536");
      #endif
       #if defined (LP)
     myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
     #endif
  break;
  case 5:
      #if defined (LP)
       myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      #if defined (OV5640_CAM)
       myCAM1.OV5640_set_JPEG_size(OV5640_1280x960);delay(1000);
       Serial.println("switch to OV5640_1280x960");
      #else
        myCAM1.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
        Serial.println("switch to OV5642_2592x1944");
       #endif
        #if defined (LP)
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
       #endif
  break;      
  #if defined (OV5640_CAM)
  case 6:
       #if defined (LP)
        myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
       #endif
        myCAM1.OV5640_set_JPEG_size(OV5640_1600x1200);delay(1000);
        Serial.println("switch to OV5640_1600x1200");
      #if defined (LP)
        myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
        #endif
   break;
  case 7:
      #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
       myCAM1.OV5640_set_JPEG_size(OV5640_2048x1536);delay(1000);
       Serial.println("switch to OV5640_2048x1536")
     #if defined (LP)
       myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
     #endif
  break;
  case 8:
      #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      myCAM1.OV5640_set_JPEG_size(OV5640_2592x1944);delay(1000);
      Serial.println("switch to OV5640_2592x1944")
      #if defined (LP)
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #endif
      break;
     #endif
    case 0x15:
    if(cam1){
      #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      Serial.println("CAM1 start Capture"); 
      myCAM1.flush_fifo(); 
      myCAM1.clear_fifo_flag();   
      myCAM1.start_capture(); 
    }
    if(cam2){
       #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      Serial.println("CAM2 start Capture"); 
      myCAM2.flush_fifo(); 
      myCAM2.clear_fifo_flag();   
      myCAM2.start_capture();  
    }
    if(cam3){
      #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      Serial.println("CAM3 start Capture"); 
      myCAM3.flush_fifo();
      myCAM3.clear_fifo_flag();   
      myCAM3.start_capture();   
    } 
    if(cam4){
       #if defined (LP)
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);delay(1000);
      #endif
      Serial.println("CAM4 start Capture"); 
      myCAM4.flush_fifo();  
      myCAM4.clear_fifo_flag();   
      myCAM4.start_capture(); 
    }  
      break;  
  }
  

  if(myCAM1.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam1 && (!cam1done))
  {
    cam1done = true;
    myCAM1.clear_fifo_flag();
    count ++;
  }
  if(myCAM2.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam2 && (!cam2done))
  {
    cam2done = true;
    myCAM2.clear_fifo_flag();
    count ++;
  }
  if(myCAM3.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam3 && (!cam3done))
  {
    cam3done = true;
    myCAM3.clear_fifo_flag();
    count ++;
  }
    if(myCAM4.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam4 && (!cam4done))
  {
    cam4done = true;
    myCAM4.clear_fifo_flag();
    count ++;
  }
  if(count >= 1)
  {
    if(cam1done==true)
    {
      #if defined (LP)
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      #endif
       count--;
      Serial.println("CAM1 Capture Done!");
      uint32_t length = 0;
      length = myCAM1.read_fifo_length();
      if(length >= MAX_FIFO_SIZE)  //8M
      {
        Serial.println("Over size.");
        myCAM1.clear_fifo_flag();
        return;
      }
      if(length == 0)  //0 kb
      {
        Serial.println("Size is 0.");
        myCAM1.clear_fifo_flag();
        return;
      }
      flag[2]=0x01;//flag of cam1
      for(int m=0;m<5;m++)
      {
        Serial.write(flag[m]);
      }
      read_fifo_burst(myCAM1);
      cam1done = false;
    } 
    if(cam2done == true)
    {
       count--;
      Serial.println("CAM2 Capture Done!");
      uint32_t length = 0;
      length = myCAM2.read_fifo_length();
      if(length >= MAX_FIFO_SIZE)  //8M
      {
        Serial.println("Over size.");
        myCAM2.clear_fifo_flag();
        return;
      }
      if(length == 0)  //0 kb
      {
        Serial.println("Size is 0.");
        myCAM2.clear_fifo_flag();
        return;
      }
      flag[2]=0x02;//flag of cam2
      for(int m=0;m<5;m++)
      {
        Serial.write(flag[m]);
      }
      read_fifo_burst(myCAM2);
      cam2done = false;
    }
    if(cam3done == true)
    {
       count--;
      Serial.println("CAM3 Capture Done!");
      uint32_t length = 0;
      length = myCAM3.read_fifo_length();
     if(length >= MAX_FIFO_SIZE)  //8M
      {
        Serial.println("Over size.");
        myCAM3.clear_fifo_flag();
        return;
      }
      if(length == 0)  //0 kb
      {
        Serial.println("Size is 0.");
        myCAM3.clear_fifo_flag();
        return;
      }
      flag[2]=0x03;//flag of cam3
      for(int m=0;m<5;m++)
      {
        Serial.write(flag[m]);
      }
      read_fifo_burst(myCAM3);
      cam3done = false;
    }
     if(cam4done == true)
    {
       count--;
      Serial.println("CAM4 Capture Done!");
      uint32_t length = 0;
      length = myCAM4.read_fifo_length();
     if(length >= MAX_FIFO_SIZE)  //8M
      {
        Serial.println("Over size.");
        myCAM4.clear_fifo_flag();
        return;
      }
      if(length == 0)  //0 kb
      {
        Serial.println("Size is 0.");
        myCAM4.clear_fifo_flag();
        return;
      }
      flag[2]=0x04;//flag of cam4
      for(int m=0;m<5;m++)
      {
        Serial.write(flag[m]);
      }
      read_fifo_burst(myCAM4);
      cam4done = false;
    }
  }
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
    uint8_t temp = 0,temp_last = 0;
    uint32_t length = 0;
    length = myCAM.read_fifo_length();
    Serial.print("Length is ");
    Serial.println(length);
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
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
        is_header = true;
        Serial.write(temp_last);
        Serial.write(temp);
      }
      if( (temp == 0xD9) && (temp_last == 0xFF) )
        break;
      delayMicroseconds(30);
    }
    myCAM.CS_HIGH();
    is_header = false;
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
    return 1;
}
