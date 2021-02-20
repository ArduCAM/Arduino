// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 5MP Plus camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini Camera.
// It needs to be used in combination with PC software.It can take 4 photos at the same and save them to the SD card
// The demo sketch will do the following tasks:
// 1. Set the 4 cameras to JPEG output mode.
// 2. Read data from Serial port and deal with it
// 3.Save the images to SD card.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM Mini 5MP Plus camera
// and use Arduino IDE 1.6.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5640_MINI_5MP_PLUS)||defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x00
#define SD_CS 9

// set pin 4,5,6,7 as the slave select for SPI:
const int CS1 = 4;
const int CS2 = 5;
const int CS3 = 6;
const int CS4 = 7;
bool CAM1_EXIST = false; 
bool CAM2_EXIST = false;
bool CAM3_EXIST = false;
bool CAM4_EXIST = false;

#if defined (OV5640_MINI_5MP_PLUS)
  ArduCAM myCAM1(OV5640, CS1);
  ArduCAM myCAM2(OV5640, CS2);
  ArduCAM myCAM3(OV5640, CS3);
  ArduCAM myCAM4(OV5640, CS4);
#else
  ArduCAM myCAM1(OV5642, CS1);
  ArduCAM myCAM2(OV5642, CS2);
  ArduCAM myCAM3(OV5642, CS3);
  ArduCAM myCAM4(OV5642, CS4);
#endif

void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp;
Wire.begin(); 
Serial.begin(115200);
Serial.println(F("ArduCAM Start!")); 
// set the CS output:
pinMode(CS1, OUTPUT);
digitalWrite(CS1, HIGH);
pinMode(CS2, OUTPUT);
digitalWrite(CS2, HIGH);
pinMode(CS3, OUTPUT);
digitalWrite(CS3, HIGH);
pinMode(CS4, OUTPUT);
digitalWrite(CS4, HIGH);
pinMode(SD_CS, OUTPUT);
// initialize SPI:
SPI.begin(); 
//Reset the CPLD
myCAM1.write_reg(0x07, 0x80);
delay(100);
myCAM1.write_reg(0x07, 0x00);
delay(100); 
myCAM2.write_reg(0x07, 0x80);
delay(100);
myCAM2.write_reg(0x07, 0x00);
delay(100); 
myCAM3.write_reg(0x07, 0x80);
delay(100);
myCAM3.write_reg(0x07, 0x00);
delay(100); 
myCAM4.write_reg(0x07, 0x80);
delay(100);
myCAM4.write_reg(0x07, 0x00);
delay(100); 

//Check if the 4 ArduCAM Mini 5MP PLus Cameras' SPI bus is OK
while(1){
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("SPI1 interface Error!"));
  }else{
    CAM1_EXIST = true;
    Serial.println(F("SPI1 interface OK."));
  }
  myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM2.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("SPI2 interface Error!"));
  }else{
    CAM2_EXIST = true;
    Serial.println(F("SPI2 interface OK."));
  }
  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("SPI3 interface Error!"));
  }else{
    CAM3_EXIST = true;
    Serial.println(F("SPI3 interface OK."));
  }
  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("SPI4 interface Error!"));
  }else{
    CAM4_EXIST = true;
    Serial.println(F("SPI4 interface OK."));
  }
  if(!(CAM1_EXIST||CAM2_EXIST||CAM3_EXIST||CAM4_EXIST)){
  delay(1000);continue;
  }else
  break;
}
//Initialize SD Card
while(!SD.begin(SD_CS)){
  Serial.println(F("SD Card Error"));delay(1000);
}
Serial.println(F("SD Card detected."));
#if defined (OV5640_MINI_5MP_PLUS)
  while(1){
    //Check if the camera module type is OV5640
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40)){
      Serial.println(F("Can't find OV5640 module!"));
      delay(1000);continue;
    }else{
      Serial.println(F("OV5640 detected."));break;
    }   
  }
#else
  while(1){
    //Check if the camera module type is OV5642
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000);continue;
    }else{
      Serial.println(F("OV5642 detected."));break;
    }  
  }
#endif
//Change to JPEG capture mode and initialize the OV5640 module
myCAM1.set_format(JPEG);
myCAM1.InitCAM();
myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM2.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM3.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM4.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM1.clear_fifo_flag();
myCAM1.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
myCAM2.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
myCAM3.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
myCAM4.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
#if defined (OV5640_MINI_5MP_PLUS)
  myCAM1.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
#else
  myCAM1.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
#endif
delay(1000);
myCAM1.clear_fifo_flag();
myCAM2.clear_fifo_flag();
myCAM3.clear_fifo_flag();
myCAM4.clear_fifo_flag();
}
void loop() {
  if(CAM1_EXIST)
    myCAMSaveToSDFile(myCAM1);
  if(CAM2_EXIST)
    myCAMSaveToSDFile(myCAM2);
  if(CAM3_EXIST)
    myCAMSaveToSDFile(myCAM3);
  if(CAM4_EXIST)
    myCAMSaveToSDFile(myCAM4);
  delay(5000);
}
void myCAMSaveToSDFile(ArduCAM myCAM){
char str[8];
byte buf[256];
static int i = 0;
static int k = 0;
uint8_t temp = 0,temp_last=0;
uint32_t length = 0;
bool is_header = false;
File outFile;
//Flush the FIFO
myCAM.flush_fifo();
//Clear the capture done flag
myCAM.clear_fifo_flag();
//Start capture
myCAM.start_capture();
Serial.println(F("start Capture"));
while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
Serial.println(F("Capture Done."));  
length = myCAM.read_fifo_length();
Serial.print(F("The fifo length is :"));
Serial.println(length, DEC);
if (length >= MAX_FIFO_SIZE) //8M
{
  Serial.println(F("Over size."));
  return ;
}
if (length == 0 ) //0 kb
{
  Serial.println(F("Size is 0."));
  return ;
}
//Construct a file name
k = k + 1;
itoa(k, str, 10);
strcat(str, ".jpg");
//Open the new file
outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
if(!outFile){
  Serial.println(F("File open faild"));
  return;
}
myCAM.CS_LOW();
myCAM.set_fifo_burst();
while ( length-- )
{
  temp_last = temp;
  temp =  SPI.transfer(0x00);
  //Read JPEG data from FIFO
  if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
  {
    buf[i++] = temp;  //save the last  0XD9     
    //Write the remain bytes in the buffer
    myCAM.CS_HIGH();
    outFile.write(buf, i);    
    //Close the file
    outFile.close();
    Serial.println(F("Image save OK."));
    is_header = false;
    i = 0;
  }  
  if (is_header == true)
  { 
    //Write image data to buffer if not full
    if (i < 256)
      buf[i++] = temp;
    else
    {
      //Write 256 bytes image data to file
      myCAM.CS_HIGH();
      outFile.write(buf, 256);
      i = 0;
      buf[i++] = temp;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
    }        
  }
  else if ((temp == 0xD8) & (temp_last == 0xFF))
  {
    is_header = true;
    buf[i++] = temp_last;
    buf[i++] = temp;   
  } 
} 
}
