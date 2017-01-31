// ArduCAM demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules, and can run on any Arduino platform.
//
// This demo was made for ARDUCAM_SHIELD_REVC.
// It will run the ArduCAM 2MP/5MP as a real 2MP/5MP digital camera, provide both JPEG capture.
// The demo sketch will do the following tasks:
// 1. Set the sensor to JPEG mode.
// 2. Capture and buffer the image to FIFO every 5 seconds 
// 3. Store the image to Micro SD/TF card with JPEG format in sequential.
// 4. Resolution can be changed by myCAM.set_JPEG_size() function.
// This program requires the ArduCAM V4.0.0 (or later) library and ARDUCAM_SHIELD_REVC
// and use Arduino IDE 1.6.8 compiler or above
#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
//This demo can only work on ARDUCAM_SHIELD_REVC  platform.
#if !(defined (ARDUCAM_SHIELD_REVC)&&(defined (OV5642_CAM)||defined (OV2640_CAM)||defined (OV5640_CAM)))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h outFile
#endif
#if defined(ESP8266)
 #define SD_CS 0 
 const int SPI_CS = 16;
#else 
 #define SD_CS 9 
 const int SPI_CS =10;
#endif
#if defined (OV2640_CAM)
  ArduCAM myCAM( OV2640, SPI_CS );
#elif defined (OV5640_CAM)
  ArduCAM myCAM( OV5640, SPI_CS );
#elif defined (OV5642_CAM)
  ArduCAM myCAM( OV5642, SPI_CS );
#endif
void myCAMSaveToSDFile(){
char str[8];
byte buf[256];
static int i = 0;
static int k = 0;
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
bool is_header = false;
File outFile;
//Flush the FIFO
myCAM.flush_fifo();
//Clear the capture done flag
myCAM.clear_fifo_flag();
//Start capture
myCAM.start_capture();
Serial.println(F("start Capture."));
while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
Serial.println(F("Capture Done."));  
length = myCAM.read_fifo_length();
Serial.print(F("The fifo length is :"));
Serial.println(length, DEC);
if (length >= MAX_FIFO_SIZE) 
{
  Serial.println(F("Over size."));
  return ;
}
if (length == 0 ) //0 kb
{
  Serial.println(F("Size is 0."));
  return ;
}
//Construct a outFile name
k = k + 1;
itoa(k, str, 10);
strcat(str, ".jpg");
//Open the new outFile
outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
if(! outFile){
  Serial.println(F("File open faild"));
  return;
}
i = 0;
myCAM.CS_LOW();
myCAM.set_fifo_burst();
//Read JPEG data from FIFO
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
    Serial.println(F("Image Save Done!"));
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
void setup(){
uint8_t vid = 0, pid = 0;
uint8_t temp = 0;
Wire.begin();
Serial.begin(115200);
Serial.println(F("ArduCAM Start!"));
//set the CS as an output:
pinMode(SPI_CS,OUTPUT);
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("SPI interface Error!"));
    delay(1000);continue;
  }else{
    Serial.println(F("SPI interface OK."));break;
  }
}
//Initialize SD Card
while(!SD.begin(SD_CS)){
  Serial.println(F("SD Card Error!"));delay(1000);
}
Serial.println(F("SD Card detected."));
#if defined (OV2640_CAM)
while(1){
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println(F("Can't find OV2640 module!"));
    delay(1000);continue;
  }else{
    Serial.println(F("OV2640 detected."));break;
  }
} 
#elif defined (OV5640_CAM)
  while(1){
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x40)){
      Serial.println(F("Can't find OV5640 module!");
      delay(1000);continue;
    }else{
      Serial.println(F("OV5640 detected."));break;
    } 
  }
#elif defined (OV5642_CAM)
  while(1){
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000);continue;
    } else{
     Serial.println(F("OV5642 detected.")); break;
    }
  }
#endif
myCAM.set_format(JPEG);
myCAM.InitCAM();
#if defined (OV2640_CAM)
myCAM.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
#elif defined (OV5640_CAM)
myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
#elif defined (OV5642_CAM)
myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
#endif
delay(1000);
}
void loop(){
myCAMSaveToSDFile();
delay(5000);
}


