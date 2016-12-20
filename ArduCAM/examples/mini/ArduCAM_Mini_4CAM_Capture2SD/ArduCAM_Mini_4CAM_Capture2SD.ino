// ArduCAM Mini demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini Camera.
// It needs to be used in combination with PC software.It can take 4 photos at the same and save them to the SD card
// The demo sketch will do the following tasks:
// 1. Set the 4 cameras to JEPG output mode.
// 2. Read data from Serial port and deal with it
// 3.Save them to SD card.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM Mini camera
// and use Arduino IDE 1.5.2 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP)
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

#define SD_CS 9

// set pin 4,5,6,7 as the slave select for SPI:
const int CS1 = 4;
const int CS2 = 5;
const int CS3 = 6;
const int CS4 = 7;

#if defined (OV2640_MINI_2MP)
  ArduCAM myCAM1(OV2640,CS1);
  ArduCAM myCAM2(OV2640,CS2);
  ArduCAM myCAM3(OV2640,CS3);
  ArduCAM myCAM4(OV2640,CS4);
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
  Wire.begin(); 
 Serial.begin(115200);
 Serial.println("ArduCAM Start!"); 
  // set the CS output:
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);
  pinMode(CS4, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  // initialize SPI:
  SPI.begin(); 

   //Initialize SD Card
  if(!SD.begin(SD_CS)){
    Serial.println("SD Card Error");
  }
  else
  Serial.println("SD Card detected!");
  
  //Check if the 4 ArduCAM Mini 2MP Cameras' SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI1 interface Error!");
  }
  myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM2.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI2 interface Error!");
  }
  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI3 interface Error!");
  }
  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println("SPI4 interface Error!");
  }
 #if defined (OV2640_MINI_2MP)
   //Check if the camera module type is OV2640
   myCAM1.wrSensorReg8_8(0xff, 0x01);
   myCAM1.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
   myCAM1.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
   if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    Serial.println("Can't find OV2640 module!");
    else
    Serial.println("OV2640 detected.");
  #else
   //Check if the camera module type is OV5642
    myCAM1.wrSensorReg16_8(0xff, 0x01);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
     if((vid != 0x56) || (pid != 0x42))
     Serial.println("Can't find OV5642 module!");
     else
     Serial.println("OV5642 detected.");
   #endif
    myCAM1.set_format(JPEG);
    myCAM1.InitCAM();
  #if defined (OV2640_MINI_2MP)
    myCAM1.OV2640_set_JPEG_size(OV2640_320x240);
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
    myCAMSaveToSDFile(myCAM1);
    myCAMSaveToSDFile(myCAM2);
    myCAMSaveToSDFile(myCAM3);
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
  Serial.println("star Capture");
 while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
 Serial.println("Capture Done!");  
 length = myCAM.read_fifo_length();
 Serial.print("The fifo length is :");
 Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //384K
  {
    Serial.println("Over size.");
    return 0;
  }
    if (length == 0 ) //0 kb
  {
    Serial.println("Size is 0.");
    return 0;
  }
 //Construct a file name
 k = k + 1;
 itoa(k, str, 10);
 strcat(str, ".jpg");
 //Open the new file
 outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
 if(!outFile){
  Serial.println("open file faild");
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
        Serial.println("CAM Save OK!");
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
