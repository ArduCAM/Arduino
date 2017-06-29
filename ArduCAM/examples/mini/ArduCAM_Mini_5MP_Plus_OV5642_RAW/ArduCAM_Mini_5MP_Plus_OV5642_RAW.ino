// ArduCAM demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to capture image in RAW format
// 1. Capture and buffer the image to FIFO every 5 seconds 
// 2. Store the image to Micro SD/TF card with RAW format.
//3. You can change the resolution by change the "resolution = OV5642_640x480"
// This program requires the ArduCAM V4.0.0 (or above) library and ArduCAM shield V2
// and use Arduino IDE 1.6.8 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x00
// set pin 7 as the slave select for the digital pot:
const int CS = 7;
#define SD_CS 9
bool is_header = false;
int total_time = 0;
uint8_t resolution = OV5642_2592x1944;
uint32_t line,column;
  ArduCAM myCAM(OV5642, CS);
uint8_t saveRAW(void);
void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp ;

#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
Serial.begin(115200);
Serial.println(F("ArduCAM Start!"));
// set the CS as an output:
pinMode(CS, OUTPUT);
// initialize SPI:
SPI.begin();
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
//Initialize SD Card
while(!SD.begin(SD_CS))
{
  Serial.println(F("SD Card Error!"));delay(1000);
}
Serial.println(F("SD Card detected."));
//Change to JPEG capture mode and initialize the OV5640 module
myCAM.set_format(RAW);
myCAM.InitCAM();
myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
}

void loop() {
   File outFile;
    char VL;
    char str[8];
    byte buf[256];
    static int k = 0,m = 0;
    int i,j = 0;
// put your main code here, to run repeatedly:
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.OV5642_set_RAW_size(resolution);delay(1000);  
//Start capture
myCAM.start_capture();
Serial.println(F("start capture."));
total_time = millis();
while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
Serial.println(F("CAM Capture Done."));
total_time = millis() - total_time;
Serial.print(F("capture total_time used (in miliseconds):"));
Serial.println(total_time, DEC);
Serial.println("Saving the image,please waitting...");
total_time = millis();
     k = k + 1;
    itoa(k, str, 10); 
    strcat(str,".raw");        //Generate file name
    outFile = SD.open(str,O_WRITE | O_CREAT | O_TRUNC);
    if (! outFile) 
    {
      Serial.println(F("File open error"));
      return;
    }
    if(resolution == OV5642_640x480 ){
      line = 640;column = 480;
      }else if( resolution == OV5642_1280x960 ){
        line = 1280;column = 960;
        }else if( resolution == OV5642_1920x1080 ){
           line = 1920;column = 1080;
          }else if( resolution == OV5642_2592x1944 ){
            line = 2592;column = 1944;
            }
    //Save as RAW format
    for(i = 0; i < line; i++)
    for(j = 0; j < column; j++)
    {
      VL = myCAM.read_fifo();
      buf[m++] = VL;
      if(m >= 256)
      {
        //Write 256 bytes image data to file from buffer
        outFile.write(buf,256);
        m = 0;
      }
    }
    if(m > 0 )//Write the left image data to file from buffer
      outFile.write( buf, m );m = 0;
    //Close the file  
    outFile.close(); 
    Serial.println("Image save OK.");
//Clear the capture done flag
myCAM.clear_fifo_flag();
delay(5000);
}
