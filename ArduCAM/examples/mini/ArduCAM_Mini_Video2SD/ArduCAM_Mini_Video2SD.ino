// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini camera, and can run on any Arduino platform.
// This demo was made for ArduCAM Mini Camera.
//This demo timed 5 seconds to record video.
// It can shoot video and store it into the SD card
// The demo sketch will do the following tasks
// 1. Set the camera to JPEG output mode.
// 2. Capture a JPEG photo and buffer the image to FIFO
// 3.Write AVI Header
// 4.Write the video data to the SD card
// 5.More updates AVI file header
// 6.close the file
//The file header introduction
//00-03 :RIFF
//04-07 :The size of the data
//08-0B :File identifier
//0C-0F :The first list of identification number
//10-13 :The size of the first list
//14-17 :The hdr1 of identification
//18-1B :Hdr1 contains avih piece of identification 
//1C-1F :The size of the avih
//20-23 :Maintain time per frame picture

// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM Mini camera
// and use Arduino IDE 1.6.8 compiler or above

#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP|| defined OV3640_MINI_3MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define SD_CS 9
#define rate 0x05
// set the num of picture
#define pic_num 200
//set pin 7 as the slave select for SPI:
const int SPI_CS = 7;
#define AVIOFFSET 240
unsigned long movi_size = 0;
unsigned long jpeg_size = 0;
const char zero_buf[4] = {0x00, 0x00, 0x00, 0x00};
const int avi_header[AVIOFFSET] PROGMEM ={
  0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, rate, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54,
  0x10, 0x00, 0x00, 0x00, 0x6F, 0x64, 0x6D, 0x6C, 0x64, 0x6D, 0x6C, 0x68, 0x04, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};
#if defined (OV2640_MINI_2MP)
  ArduCAM myCAM( OV2640, SPI_CS );
#elif defined (OV3640_MINI_3MP)
  ArduCAM myCAM( OV3640, SPI_CS );
#else
  ArduCAM myCAM( OV5642, SPI_CS );
#endif
void print_quartet(unsigned long i,File fd){
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);
}
void Video2SD(){
char str[8];
File outFile;
byte buf[256];
static int i = 0;
static int k = 0;
uint8_t temp = 0, temp_last = 0;
unsigned long position = 0;
uint16_t frame_cnt = 0;
uint8_t remnant = 0;
uint32_t length = 0;
bool is_header = false;
//Create a avi file
k = k + 1;
itoa(k, str, 10);
strcat(str, ".avi");
//Open the new file
outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
if (! outFile)
{
  Serial.println(F("File open failed"));
  while (1);
  return;
}
//Write AVI Header

for ( i = 0; i < AVIOFFSET; i++)
{
  char ch = pgm_read_byte(&avi_header[i]);
  buf[i] = ch;
}
outFile.write(buf, AVIOFFSET);
//Write video data
Serial.println(F("Recording video, please wait..."));
for ( frame_cnt = 0; frame_cnt < pic_num; frame_cnt ++)
{
  #if defined (ESP8266)
  yield();
  #endif
  temp_last = 0;temp = 0;
  //Capture a frame            
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  length = myCAM.read_fifo_length();
  outFile.write("00dc");
  outFile.write(zero_buf, 4);
  i = 0;
  jpeg_size = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  while ( length-- )
  {
    #if defined (ESP8266)
    yield();
    #endif
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9     
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);    
      is_header = false;
      jpeg_size += i;
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
        jpeg_size += 256;
      }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  }
  remnant = (4 - (jpeg_size & 0x00000003)) & 0x00000003;
  jpeg_size = jpeg_size + remnant;
  movi_size = movi_size + jpeg_size;
  if (remnant > 0)
  outFile.write(zero_buf, remnant); 
  position = outFile.position();
  outFile.seek(position - 4 - jpeg_size);
  print_quartet(jpeg_size, outFile);
  position = outFile.position();
  outFile.seek(position + 6);
  outFile.write("AVI1", 4);
  position = outFile.position();
  outFile.seek(position + jpeg_size - 10);
}
//Modify the MJPEG header from the beginning of the file
outFile.seek(4);
print_quartet(movi_size +12*frame_cnt+4, outFile);//riff file size
//overwrite hdrl
unsigned long us_per_frame = 1000000 / rate; //(per_usec); //hdrl.avih.us_per_frame
outFile.seek(0x20);
print_quartet(us_per_frame, outFile);
unsigned long max_bytes_per_sec = movi_size * rate/ frame_cnt; //hdrl.avih.max_bytes_per_sec
outFile.seek(0x24);
print_quartet(max_bytes_per_sec, outFile);
unsigned long tot_frames = frame_cnt;    //hdrl.avih.tot_frames
outFile.seek(0x30);
print_quartet(tot_frames, outFile);
unsigned long frames =frame_cnt;// (TOTALFRAMES); //hdrl.strl.list_odml.frames
outFile.seek(0xe0);
print_quartet(frames, outFile);
outFile.seek(0xe8);
print_quartet(movi_size, outFile);// size again
myCAM.CS_HIGH();
//Close the file
outFile.close();
Serial.println(F("Record video OK."));
}
void setup(){
uint8_t vid, pid;
uint8_t temp;
Wire.begin();
Serial.begin(115200);
Serial.println(F("ArduCAM Start!"));
// set the SPI_CS as an output:
pinMode(SPI_CS, OUTPUT);
delay(1000);
// initialize SPI:
SPI.begin();
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55)
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
#if defined (OV2640_MINI_2MP)
while(1){
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println(F("Can't find OV2640 module!"));
    delay(1000);continue;
  }
  else{
   Serial.println(F("OV2640 detected."));break;
  } 
}
#elif defined (OV3640_MINI_3MP)
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
#else
while(1){
  //Check if the camera module type is OV5642
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x42)){
    Serial.println(F("Can't find OV5642 module!"));
    delay(1000);continue;
  }
  else{
    Serial.println(F("OV5642 detected."));break;
  } 
}
#endif
myCAM.set_format(JPEG);
myCAM.InitCAM();
#if defined (OV2640_MINI_2MP)
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
#elif defined (OV3640_MINI_3MP)
  myCAM.OV3640_set_JPEG_size(OV3640_320x240);
#else
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
#endif
delay(1000);
}
void loop(){
Video2SD(); 
delay(5000);
}






