#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

const int CS1 = 4;
const int CS2 = 5;
const int CS3 = 6;
const int CS4 = 7;
bool cam1=true,cam2=true,cam3=true,cam4=true;
bool is_header = false;
byte flag[5]={0xFF,0xAA,0x01,0xFF,0x55};

ArduCAM myCAM1(OV5642,CS1);
ArduCAM myCAM2(OV5642,CS2);
ArduCAM myCAM3(OV5642,CS3);
ArduCAM myCAM4(OV5642,CS4);
uint8_t read_fifo_burst(ArduCAM myCAM);

void setup() {
  // put your setup code here, to run once:
  uint8_t vid,pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(921600);
  Serial.println("ArduCAM Start!"); 

  // set the SPI_CS as an output:
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);
  pinMode(CS4, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
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
  //Serial.println(temp);
  if(temp != 0x55)
  {
    Serial.println("SPI2 interface Error!");
    cam2 = false;
    //while(1);
  }
  
  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
    Serial.println("SPI3 interface Error!");
    cam3 = false;
    //while(1);
  }
  
  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
    Serial.println("SPI4 interface Error!");
    cam4 = false;
    //while(1);
  }

  //Check if the camera module type is OV5642
  myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

  if((vid != 0x56) || (pid != 0x42))
    Serial.println("Can't find OV5642 module!");
  else
    Serial.println("OV5642 detected");
  
  //Change to JPEG capture mode and initialize the OV5642 module	
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.OV5642_set_JPEG_size(OV5642_320x240);//set 4 cams resolution
  
  myCAM1.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM2.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM3.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM4.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  
  myCAM1.clear_fifo_flag();
  myCAM2.clear_fifo_flag();
  myCAM3.clear_fifo_flag();
  myCAM4.clear_fifo_flag();
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;

  temp = Serial.read();
  switch(temp)
  {
    case 0:
      myCAM1.OV5642_set_JPEG_size(OV5642_320x240);
      break;
    case 1:
      myCAM1.OV5642_set_JPEG_size(OV5642_640x480);
      break;
    case 2:
      myCAM1.OV5642_set_JPEG_size(OV5642_1280x720);
      break;
    case 3:
      myCAM1.OV5642_set_JPEG_size(OV5642_1920x1080);
      break;
    case 4:
      myCAM1.OV5642_set_JPEG_size(OV5642_2048x1563);
      break;
    case 5:
      myCAM1.OV5642_set_JPEG_size(OV5642_2592x1944);
      break;
    case 0x11:
      start_capture = 1;
      Serial.println("CAM1 Start Capture"); 
      myCAM1.flush_fifo();
      break;
    case 0x12:
      start_capture = 2;
      Serial.println("CAM2 Start Capture"); 
      myCAM2.flush_fifo();
      break;
    case 0x13:
      start_capture = 3;
      Serial.println("CAM3 Start Capture"); 
      myCAM3.flush_fifo();
      break;
    case 0x14:
      start_capture = 4;
      Serial.println("CAM4 Start Capture"); 
      myCAM4.flush_fifo();
      break;
    case 0x15:
      start_capture = 5;
      Serial.println("4CAMS Start Capture"); 
      myCAM1.flush_fifo();
      myCAM2.flush_fifo();
      myCAM3.flush_fifo();
      myCAM4.flush_fifo();
      break;
  }
  
  if(start_capture==5)
  {
    //Clear the capture done flag 
    myCAM1.clear_fifo_flag();	 
    //Start capture
    myCAM1.start_capture();
    //Clear the capture done flag 
    myCAM2.clear_fifo_flag();	 
    //Start capture
    myCAM2.start_capture();
    //Clear the capture done flag 
    myCAM3.clear_fifo_flag();	 
    //Start capture
    myCAM3.start_capture();
    //Clear the capture done flag 
    myCAM4.clear_fifo_flag();	 
    //Start capture
    myCAM4.start_capture();
    start_capture = 0;	 
  }
  
  if(start_capture==1)
  {
    //Clear the capture done flag 
    myCAM1.clear_fifo_flag();	 
    //Start capture
    myCAM1.start_capture();
   start_capture = 0; 
  }
  if(myCAM1.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam1)
  {
    Serial.println("CAM1 Capture Done!");
    uint32_t length = 0;
    length = myCAM1.read_fifo_length();
    if(length >= 524288)  //512 kb
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
    flag[2]=0x01;
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM1);
  }
  if(start_capture==2)
  {
    //Clear the capture done flag 
    myCAM2.clear_fifo_flag();	 
    //Start capture
    myCAM2.start_capture();
    start_capture = 0;	 
  }
  if(myCAM2.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam2)
  {
    Serial.println("CAM2 Capture Done!");
    uint32_t length = 0;
    length = myCAM2.read_fifo_length();
    if(length >= 524288)  //512 kb
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
    flag[2]=0x02;
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM2);
  }
  
  if(start_capture==3)
  {
    //Clear the capture done flag 
    myCAM3.clear_fifo_flag();	 
    //Start capture
    myCAM3.start_capture();
    start_capture = 0;	 
  }
  if(myCAM3.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam3)
  {
    Serial.println("CAM3 Capture Done!");
    uint32_t length = 0;
    length = myCAM3.read_fifo_length();
    if(length >= 524288)  //512 kb
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
    flag[2]=0x03;
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM3);
  }
  
  if(start_capture==4)
  {
    //Clear the capture done flag 
    myCAM4.clear_fifo_flag();	 
    //Start capture
    myCAM4.start_capture();
    start_capture = 0;	 
  }
  if(myCAM4.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && cam4)
  {
    Serial.println("CAM4 Capture Done!");
    uint32_t length = 0;
    length = myCAM4.read_fifo_length();
    if(length >= 524288)  //512 kb
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
    flag[2]=0x04;
    for(int m=0;m<5;m++)
    {
      Serial.write(flag[m]);
    }
    read_fifo_burst(myCAM4);
  }
}

uint8_t read_fifo_burst(ArduCAM myCAM)
{
    uint8_t temp,temp_last;
    uint32_t length = 0;
    length = myCAM.read_fifo_length();
    //Serial.println(length);
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
    SPI.transfer(0x00);//First byte is 0xC0 ,not 0xff
    length--;
    while( length-- )
    {
      temp_last = temp;
      temp =  SPI.transfer(0x00);
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
      delayMicroseconds(12);
    }
    myCAM.CS_HIGH();
    is_header = false;
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
}
