// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use the enhanced functions
// This demo was made for ARDUCAM_SHIELD_V2.
// It can  continue shooting and store it into the SD card  in JPEG format
// The demo sketch will do the following tasks
// 1. Set the camera to JPEG output mode.
// 2. Capture a JPEG photo and buffer the image to FIFO
// 3.Write the picture data to the SD card
// 5.close the file
//You can change the FRAMES_NUM count to change the number of the picture.
//IF the FRAMES_NUM is 0X00, take one photos
//IF the FRAMES_NUM is 0X01, take two photos
//IF the FRAMES_NUM is 0X02, take three photos
//IF the FRAMES_NUM is 0X03, take four photos
//IF the FRAMES_NUM is 0X04, take five photos
//IF the FRAMES_NUM is 0X05, take six photos
//IF the FRAMES_NUM is 0X06, take seven photos
//IF the FRAMES_NUM is 0X07, continue shooting until the FIFO is full
//You can see the picture in the SD card.
//This program requires the ArduCAM V4.0.0 (or later) library and ARDUCAM_SHIELD_V2
//and use Arduino IDE 1.6.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
//This demo can only work on ARDUCAM_SHIELD_V2  platform.
#if !(defined (ARDUCAM_SHIELD_V2)&&(defined (OV5640_CAM) ||defined (OV5642_CAM) ||defined (OV2640_CAM) ||defined (OV3640_CAM)))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x06
#if defined(ESP8266)
 #define SD_CS 0 
 const int SPI_CS = 16;
#else 
 #define SD_CS 9 
 const int SPI_CS =10;
#endif
bool is_header = false;
int total_time = 0;
#if defined (OV5640_CAM)
  ArduCAM myCAM(OV5640, SPI_CS);
#elif defined (OV5642_CAM)
  ArduCAM myCAM(OV5642, SPI_CS);
#elif defined (OV2640_CAM)
  ArduCAM myCAM(OV2640, SPI_CS);
#elif defined (OV3640_CAM)
  ArduCAM myCAM(OV3640, SPI_CS);
#endif
uint8_t read_fifo_burst(ArduCAM myCAM);
void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
Serial.begin(115200);
Serial.println(F("ArduCAM Start!"));
// set the CS as an output:
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
    Serial.println(F("SPI interface Error!"));
    delay(1000);continue;
  }else{
    Serial.println(F("SPI interface OK!"));break;
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
    }else{
      Serial.println(F("OV2640 detected."));break;
    }
  }
#elif defined (OV3640_CAM)
while(1){
  //Check if the camera module type is OV3640
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
  if((vid != 0x36) || (pid != 0x4C)){
    Serial.println(F("Can't find OV3640 module!"));
    delay(1000);continue;
  }else{
    Serial.println(F("OV3640 detected."));break;
  } 
 } 
#elif defined (OV5640_CAM)
  while(1){
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x40)){
      Serial.println(F("Can't find OV5640 module!"));
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
//Initialize SD Card
while(!SD.begin(SD_CS)){
  Serial.println(F("SD Card Error"));delay(1000);
}
Serial.println(F("SD Card detected."));
myCAM.set_format(JPEG);
myCAM.InitCAM();
#if !(defined (OV2640_CAM) || defined (OV3640_CAM)) 
  myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
#endif
myCAM.clear_fifo_flag();
myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
}
void loop() {
// put your main code here, to run repeatedly:
myCAM.flush_fifo();
myCAM.clear_fifo_flag();
#if defined (OV5640_CAM)
  myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
#elif defined (OV5642_CAM)
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
#elif defined (OV2640_CAM)
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
#elif defined (OV3640_CAM)
  myCAM.OV3640_set_JPEG_size(OV3640_320x240);delay(1000);
#endif
//Start capture
myCAM.start_capture();
Serial.println(F("start capture!"));
total_time = millis();
while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)){
#if defined (ESP8266)
  yield();
#endif
} ; 
Serial.println(F("CAM Capture Done."));
total_time = millis() - total_time;
Serial.print(F("capture total_time used (in miliseconds):"));
Serial.println(total_time, DEC);
total_time = millis();
read_fifo_burst(myCAM);
total_time = millis() - total_time;
Serial.print(F("Save capture total_time used (in miliseconds):"));
Serial.println(total_time, DEC);
//Clear the capture done flag
myCAM.clear_fifo_flag();
delay(5000);
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
static int i = 0;
static int k = 0;
char str[8];
File outFile;
byte buf[256]; 
length = myCAM.read_fifo_length();
Serial.print(F("The fifo length is :"));
Serial.println(length, DEC);
if (length >= MAX_FIFO_SIZE) //8M
{
  Serial.println(F("Over size."));
  return 0 ;
}
if (length == 0 ) //0 kb
{
  Serial.println(F("Size is 0."));
  return 0;
} 
myCAM.CS_LOW();
myCAM.set_fifo_burst();//Set fifo burst mode
i = 0;
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
    Serial.println(F("OK"));
    is_header = false;
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
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
    myCAM.CS_HIGH();
    //Create a avi file
    k = k + 1;
    itoa(k, str, 10);
    strcat(str, ".jpg");
    //Open the new file
    outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
    if (! outFile)
    {
      Serial.println(F("File open failed"));
      while (1);
    }
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();   
    buf[i++] = temp_last;
    buf[i++] = temp;   
  }  
}
myCAM.CS_HIGH();
return 1;
}
