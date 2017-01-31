// ArduCAM demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules.
//
//This demo is compatible with ESP8266
//
// This demo was made for Aptina MT9M001 sensor.
// It will turn the ArduCAM into a real digital camera with capture and playback functions.
// 
// 1. Capture and buffer the image to FIFO when shutter pressed quickly.
// 2. Store the image to Micro SD/TF card with RAW format.
// 
// This program requires the ArduCAM V4.0.0 (or above) library and ArduCAM shield V2
// and use Arduino IDE 1.6.8 compiler or above

#include <UTFT_SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#if !(defined (MT9M001_CAM))
#error This demo can only support  MT9M001_CAM.
#endif
#if !(defined ARDUCAM_SHIELD_V2 && defined MT9M001_CAM )                               
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
ArduCAM myCAM(MT9M001, SPI_CS);
UTFT myGLCD(SPI_CS);

void setup()
{
uint16_t vid;
uint8_t temp = 0; 
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
Serial.begin(115200);
Serial.println(F("ArduCAM Start!")); 
// set the SPI_CS as an output:
pinMode(SPI_CS, OUTPUT);
// initialize SPI:
SPI.begin(); 
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55){
    Serial.println(F("SPI interface Error!"));
    delay(1000);continue;
  }else{
    Serial.println(F("SPI interface OK!"));break;
  }
}
while(1){
  //Check if the camera module type is MT9M001
  myCAM.rdSensorReg8_16(0x00, &vid);
  if (vid != 0x8431)
  {
    Serial.println(F("Can't find MT9M001 module!"));
    delay(1000);continue;
  }
  else{
    Serial.println(F("MT9M001 detected"));break;
  }
}
//Change MCU mode
myCAM.set_mode(MCU2LCD_MODE);
//Initialize the LCD Module
myGLCD.InitLCD();
myCAM.InitCAM();
myCAM.write_reg(ARDUCHIP_TIM, PCLK_REVERSE_MASK);
myCAM.wrSensorReg8_16(0x03, 240);
myCAM.wrSensorReg8_16(0x04, 639);
//Initialize SD Card
while(!SD.begin(SD_CS)){
  Serial.println(F("SD Card Error"));delay(1000);
}
Serial.println(F("SD Card detected!"));
}
void loop()
{
char str[8];
static int k = 0;
myCAM.set_mode(CAM2LCD_MODE);		 	//Switch to CAM
while(1)
{
  #if defined(ESP8266)
  yield();
  #endif
  if(!myCAM.get_bit(ARDUCHIP_TRIG,VSYNC_MASK))		//New Frame is coming
  {
    myCAM.set_mode(MCU2LCD_MODE);    	//Switch to MCU
    myGLCD.resetXY();
    myCAM.set_mode(CAM2LCD_MODE);    	//Switch to CAM
    while(!myCAM.get_bit(ARDUCHIP_TRIG,VSYNC_MASK)); 	//Wait for VSYNC is gone
  }
  else if(myCAM.get_bit(ARDUCHIP_TRIG,SHUTTER_MASK))
  {
    k = k + 1;
    itoa(k, str, 10); 
    strcat(str,".raw");				//Generate file name
    myCAM.set_mode(MCU2LCD_MODE);    	//Switch to MCU, freeze the screen 
    GrabImage(str);
  }
}
}
void GrabImage(char* str)
{
File outFile;
char VL;
byte buf[256];
static int k = 0;
int i,j = 0;
outFile = SD.open(str,O_WRITE | O_CREAT | O_TRUNC);
if (! outFile) 
{
  Serial.println(F("File open error"));
  return;
}
myCAM.wrSensorReg8_16(0x03, 1023);
myCAM.wrSensorReg8_16(0x04, 1279);   
//Flush the FIFO 
myCAM.flush_fifo();		 
//Start capture
myCAM.start_capture();
Serial.println(F("Start Capture")); 
//Polling the capture done flag
while(!myCAM.get_bit(ARDUCHIP_TRIG,CAP_DONE_MASK));
Serial.println(F("Capture Done."));
k = 0;
//Read 1280x1024 byte from FIFO
//Save as RAW format
for(i = 0; i < 1280; i++)
for(j = 0; j < 1024; j++)
{
  VL = myCAM.read_fifo();
  buf[k++] = VL;
  #if defined(ESP8266)
    yield();
  #endif
  //Write image data to bufer if not full
  if(k >= 256)
  {
    //Write 256 bytes image data to file from buffer
    outFile.write(buf,256);
    k = 0;
  }
}
//Close the file  
outFile.close(); 
//Clear the capture done flag 
myCAM.clear_fifo_flag();
myCAM.wrSensorReg8_16(0x03, 240);
myCAM.wrSensorReg8_16(0x04, 639);
//Switch to LCD Mode
myCAM.write_reg(ARDUCHIP_TIM, PCLK_REVERSE_MASK);
return;
}   

