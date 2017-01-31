// ArduCAM demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules.
//This demo can only work on ARDUCAM_SHIELD_V2 platform.
//This demo is compatible with ESP8266
// This demo was made for Omnivision OV2640/OV5640/OV5642/ sensor.
// It will turn the ArduCAM into a real digital camera with capture and playback functions.
// 1. Preview the live video on LCD Screen.
// 2. Capture and buffer the image to FIFO when shutter pressed quickly.
// 3. Store the image to Micro SD/TF card with BMP format.
// 4. Playback the capture photos one by one when shutter buttom hold on for 3 seconds.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM shield V2
// and use Arduino IDE 1.6.8 compiler or above
#include <UTFT_SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo was made for Omnivision MT9D111A/MT9D111B/MT9M112/MT9V111_CAM/
//                                  MT9M001/MT9T112/MT9D112/OV7670/OV7675/
//                                  OV7725/OV2640/OV3640/OV5640/OV5642 sensor.
#if !(defined ARDUCAM_SHIELD_V2 && (defined MT9D111A_CAM|| defined MT9D111B_CAM || defined MT9M112_CAM  \
                                 || defined MT9V111_CAM || defined MT9M001_CAM || defined MT9T112_CAM \
                                 || defined MT9D112_CAM || defined OV7670_CAM  || defined OV7675_CAM  \
                                 || defined OV7725_CAM  || defined OV2640_CAM  || defined OV5640_CAM  \
                                 || defined OV5642_CAM|| defined OV3640_CAM))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#if defined(__arm__)
  #include <itoa.h>
#endif
#if defined(ESP8266)
 #define SD_CS 0 
 const int SPI_CS = 16;
#else 
 #define SD_CS 9 
 const int SPI_CS =10;
#endif
#define BMPIMAGEOFFSET 66
const int bmp_header[BMPIMAGEOFFSET] PROGMEM =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};
#if defined (MT9D111A_CAM)
  ArduCAM myCAM(MT9D111_A, SPI_CS);
#elif defined (MT9D111B_CAM)
  ArduCAM myCAM(MT9D111_B, SPI_CS);
#elif defined (MT9M112_CAM)
  ArduCAM myCAM(MT9M112, SPI_CS);
#elif defined (MT9V111_CAM)
  ArduCAM myCAM(MT9V111, SPI_CS);
#elif defined (MT9M001_CAM)
  ArduCAM myCAM(MT9M001, SPI_CS);
#elif defined (MT9T112_CAM)
  ArduCAM myCAM(MT9T112, SPI_CS);
#elif defined (MT9D112_CAM)
  ArduCAM myCAM(MT9D112, SPI_CS);
#elif defined (OV7670_CAM)
  ArduCAM myCAM(OV7670, SPI_CS);
#elif defined (OV7675_CAM)
  ArduCAM myCAM(OV7675, SPI_CS);
#elif defined (OV7725_CAM)
  ArduCAM myCAM(OV7725, SPI_CS);
#elif defined (OV2640_CAM)
  ArduCAM myCAM(OV2640, SPI_CS);
#elif defined (OV3640_CAM)
  ArduCAM myCAM(OV3640, SPI_CS);
#elif defined (OV5640_CAM)
  ArduCAM myCAM(OV5640, SPI_CS);
#elif defined (OV5642_CAM)
  ArduCAM myCAM(OV5642, SPI_CS);
#endif
UTFT myGLCD(SPI_CS);
void setup()
{
uint8_t vid, pid;
uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
Serial.begin(115200);
Serial.println(F("ArduCAM Start!"));
// set the SPI_CS as an output:
pinMode(SPI_CS, OUTPUT);
//initialize SPI:
SPI.begin();
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55)
  {
    Serial.println(F("SPI interface Error!"));
    delay(1000); continue;    
  } else{
    Serial.println(F("SPI interface OK"));break;
  } 	  
}
#if defined (OV2640_CAM)
while(1){
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println(F("Can't find OV2640 module!"));
    delay(1000);continue;
  } else{
    Serial.println(F("OV2640 detected."));break;
  }
}
#elif defined (OV3640_CAM)  
  while(1){
    //Check if the camera module type is OV3640
    myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
    if ((vid != 0x36) || (pid != 0x4C)){
      Serial.println(F("Can't find OV3640 module!"));
      delay(1000);continue; 
    }else{
      Serial.println(F("OV3640 detected."));break;    
    }
 } 
#elif defined (OV5640_CAM)  
  while(1){
    //Check if the camera module type is OV5640
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40)){
      Serial.println(F("Can't find OV5640 module!"));
      delay(1000);continue; 
    }else{
      Serial.println(F("OV5640 detected."));break;    
    }
 } 
#elif defined (OV5642_CAM)  
while(1){
  //Check if the camera module type is OV5642
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if ((vid != 0x56) || (pid != 0x42)){
    Serial.println(F("Can't find OV5642 module!"));
    delay(1000);continue; 
  }else{
    Serial.println(F("OV5642 detected."));break;    
  }
} 
#endif
//Change MCU mode
myCAM.set_mode(MCU2LCD_MODE);
//Initialize the LCD Module
myGLCD.InitLCD();
myCAM.InitCAM();
//Initialize SD Card
while(!SD.begin(SD_CS)){
  Serial.println(F("SD Card Error"));delay(1000);
}
 Serial.println(F("SD Card detected!"));
}
void loop()
{
char str[8];
unsigned long previous_time = 0;
static int k = 0;
myCAM.set_mode(CAM2LCD_MODE);		 	//Switch to CAM
while (1)
{
  #if defined(ESP8266)
    yield();
  #endif
  if (!myCAM.get_bit(ARDUCHIP_TRIG, VSYNC_MASK))		//New Frame is coming
  {
    myCAM.set_mode(MCU2LCD_MODE);    	//Switch to MCU
    myGLCD.resetXY();
    myCAM.set_mode(CAM2LCD_MODE);    	//Switch to CAM
    while (!myCAM.get_bit(ARDUCHIP_TRIG, VSYNC_MASK)); 	//Wait for VSYNC is gone
  }
  else if (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
  {
    previous_time = millis();
    while (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
    {
      if ((millis() - previous_time) > 1500)
      {
        Playback();
      }
    }
    if ((millis() - previous_time) < 1500)
    {
      k = k + 1;
      itoa(k, str, 10);
      strcat(str, ".bmp");				//Generate file name
      myCAM.set_mode(MCU2LCD_MODE);    	//Switch to MCU, freeze the screen
      GrabImage(str);
    }
  }
}
}
void GrabImage(char* str)
{
File outFile;
char VH, VL;
byte buf[256];
static int k = 0;
int i, j = 0;
outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
if (! outFile)
{
  Serial.println(F("File open error"));
  return;
}
//Flush the FIFO
myCAM.flush_fifo();
//Start capture
myCAM.start_capture();
Serial.println(F("Start Capture"));
//Polling the capture done flag
while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
Serial.println(F("Capture Done."));
k = 0;
//Write the BMP header
for ( i = 0; i < BMPIMAGEOFFSET; i++)
{
  char ch = pgm_read_byte(&bmp_header[i]);
  buf[k++] = ch;
}
outFile.write(buf, k);
k = 0;
//Read 320x240x2 byte from FIFO
//Save as RGB565 bmp format
for (i = 0; i < 240; i++)
for (j = 0; j < 320; j++)
{
  VH = myCAM.read_fifo();
  VL = myCAM.read_fifo();
  buf[k++] = VL;
  buf[k++] = VH;
  #if defined(ESP8266)
    yield();
  #endif
  //Write image data to bufer if not full
  if (k >= 256)
  {
    //Write 256 bytes image data to file from buffer
    outFile.write(buf, 256);
    k = 0;
  }
}
//Close the file
outFile.close();
//Clear the capture done flag
myCAM.clear_fifo_flag();
//Switch to LCD Mode
myCAM.write_reg(ARDUCHIP_TIM, 0);
return;
}
void Playback()
{
  File inFile;
  char str[8];
  int k = 0;
  myCAM.set_mode(MCU2LCD_MODE);    		//Switch to MCU
  myGLCD.InitLCD(PORTRAIT);
  while (1)
  {
    k = k + 1;
    itoa(k, str, 10);
    strcat(str, ".bmp");
    inFile = SD.open(str, FILE_READ);
    if (! inFile)
    return;
    myGLCD.clrScr();
    //myGLCD.resetXY();
    dispBitmap(inFile);
    inFile.close();
    delay(2000);
  }
}
//Only support RGB565 bmp format
void dispBitmap(File inFile)
{ 
  char VH = 0, VL = 0;
  int i, j = 0;
  for (i = 0 ; i < BMPIMAGEOFFSET; i++)
  inFile.read();
  for (i = 0; i < 320; i++)
  for (j = 0; j < 240; j++)
  {
    VL = inFile.read();
    //Serial.write(VL);
    VH = inFile.read();
    //Serial.write(VH);
    #if defined(ESP8266)
      yield();
    #endif
    myGLCD.LCD_Write_DATA(VH, VL);
  }
  myGLCD.clrXY();
}
