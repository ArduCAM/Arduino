// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com.
// This demo was made for ArduCAM_Mini_5MP_Plus.
// It can  continue shooting  and store it into the SD card  in AVI format
// The demo sketch will do the following tasks
// 1.Shoot video button, began to shoot video 
// 2. Set the camera to JPEG output mode.
// 3. Capture a JPEG photo and buffer the image to FIFO
// 4.Write AVI Header
// 5.Write the video data to the SD card
// 6.More updates AVI file header
// 7.close the file
// The file header introduction
// 00-03 :RIFF
// 04-07 :The size of the data
// 08-0B :File identifier
// 0C-0F :The first list of identification number
// 10-13 :The size of the first list
// 14-17 :The hdr1 of identification
// 18-1B :Hdr1 contains avih piece of identification 
// 1C-1F :The size of the avih
// 20-23 :Maintain time per frame picture
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini_5MP_Plus
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

#define   FRAMES_NUM    0x07
#define   rate     0x0a
#define SD_CS 9
#define KEY 2
#define AVIOFFSET 240
// set pin 7 as the slave select for the digital pot:
const int CS = 7;
bool is_header = false;
uint32_t total_time = 0;
unsigned long movi_size = 0;
unsigned long jpeg_size = 0;
const char zero_buf[4] = {0x00, 0x00, 0x00, 0x00};
const int avi_header[AVIOFFSET] PROGMEM ={
  0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x02, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, rate, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54,
  0x10, 0x00, 0x00, 0x00, 0x6F, 0x64, 0x6D, 0x6C, 0x64, 0x6D, 0x6C, 0x68, 0x04, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};
void print_quartet(unsigned long i,File fd){
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);
}
#if defined (OV5640_MINI_5MP_PLUS)
  ArduCAM myCAM(OV5640, CS);
#else
  ArduCAM myCAM(OV5642, CS);
#endif
uint8_t read_fifo_burst();
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
pinMode(CS, OUTPUT);
pinMode(SD_CS, OUTPUT);
pinMode(KEY, INPUT);
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
#if defined (OV5640_MINI_5MP_PLUS)
while(1){
  //Check if the camera module type is OV5640
  myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
  if ((vid != 0x56) || (pid != 0x40)){
    Serial.println(F("Can't find OV5640 module!"));
    delay(1000); continue;
  }else{
    Serial.println(F("OV5640 detected."));break;      
  }
}
#else
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

//Initialize SD Card
while(!SD.begin(SD_CS))
{
  Serial.println(F("SD Card Error!"));delay(1000);
}
Serial.println(F("SD Card detected."));

//Change to JPEG capture mode and initialize the OV5640 module
myCAM.set_format(JPEG);
myCAM.InitCAM();
myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
#if defined (OV5640_MINI_5MP_PLUS)
  myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
#else
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
#endif
myCAM.clear_fifo_flag();
myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
}
boolean isCaptureFlag = false;
bool keyState;
uint8_t temp, temp_last;
uint32_t length = 0;
uint32_t flash_time = 0;
void loop() {
// put your main code here, to run repeatedly:
keyState= digitalRead(KEY);
if(!keyState)  {
  isCaptureFlag = true;
  while(!digitalRead(KEY));  
}
if(isCaptureFlag){
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start capture."));
  total_time = millis();
  flash_time =millis();
  while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  length = myCAM.read_fifo_length();
  if( length < 0x3FFFFF){
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
    Serial.println(F("CAM Capture Done."));
    total_time = millis() - total_time;
  }else{
    Serial.println(F("CAM Capture Done."));
    total_time = millis() - total_time;
  }
  total_time = millis();
  read_fifo_burst();
  total_time = millis() - total_time;
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  isCaptureFlag = false;
}
}
uint8_t read_fifo_burst()
{  
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
static int i = 0;
static int k = 0;
unsigned long position = 0;
uint16_t frame_cnt = 0;
uint8_t remnant = 0;
File outFile;
char str[8];
byte buf[256]; 
length = myCAM.read_fifo_length();
Serial.print(F("The fifo length is :"));
Serial.println(length, DEC);
// Serial.println("writting the data to the SD !");
if (length >= MAX_FIFO_SIZE) //8M
{
  Serial.println(F("Over size."));
  return 0;
}
if (length == 0 ) //0 kb
{
  Serial.println(F("Size is 0."));
  return 0;
}
movi_size = 0;
//Create a avi file
k = k + 1;
itoa(k, str, 10);
strcat(str, ".avi");
//Open the new file
outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
if (! outFile)
{
  Serial.println(F("open file failed"));
  while (1);
}
//Write AVI Header
for ( i = 0; i < AVIOFFSET; i++)
{
  char ch = pgm_read_byte(&avi_header[i]);
  buf[i] = ch;
}
outFile.write(buf, AVIOFFSET); 
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
    buf[i++] = temp; //save the last 0XD9
    //Write the remain bytes in the buffer
    myCAM.CS_HIGH();
    outFile.write(buf, i);
    jpeg_size += i;
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
    is_header = false;
    frame_cnt++;
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
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
  else if ((temp == 0xD8) && (temp_last == 0xFF))
  {
    is_header = true;
    myCAM.CS_HIGH();
    outFile.write("00dc");
    outFile.write(zero_buf, 4);
    i = 0;
    jpeg_size = 0;
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();   
    buf[i++] = temp_last;
    buf[i++] = temp;   
  }
}
myCAM.CS_HIGH();
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
//Close the file
outFile.close();
is_header = false;
Serial.println(F("Movie save OK"));
return 1;
}
