// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 5MP camera, and can run on any Arduino platform.
// This demo was made for ARDUCAM_SHIELD_REVC.
// It needs to be used in combination with PC software.
// It can take photo continuously as video streaming.
//
// The demo sketch will do the following tasks:
// 1. Set the camera to JPEG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x10,camera will capture a JPEG photo and buffer the image to FIFO.Then write datas to Serial port.
// 5. If receive 0x20,camera will capture JPEG photo and write datas continuously.Stop when receive 0x21.
// 6. If receive 0x30,camera will capture a BMP  photo and buffer the image to FIFO.Then write datas to Serial port.
// 7. If receive 0x11 ,set camera to JPEG output mode.
// 8. If receive 0x31 ,set camera to BMP  output mode.
// This program requires the ArduCAM V4.0.0 (or later) library and ARDUCAM_SHIELD_REVC
// and use Arduino IDE 1.6.8 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo can only work on ARDUCAM_SHIELD_REVC  platform.
#if !(defined (ARDUCAM_SHIELD_REVC)&&(defined (OV5642_CAM)||defined (OV2640_CAM)||defined (OV5640_CAM)))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define BMPIMAGEOFFSET 66

#if defined(ESP8266)
 const int SPI_CS = 16;
#else 
 const int SPI_CS =10;
#endif
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;
#if defined (OV2640_CAM)
  ArduCAM myCAM(OV2640, SPI_CS);
#elif defined (OV5640_CAM)
  ArduCAM myCAM(OV5640, SPI_CS);
#elif defined (OV5642_CAM)
  ArduCAM myCAM(OV5642, SPI_CS);
#endif
uint8_t read_fifo_burst(ArduCAM myCAM);
void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
  Serial.begin(115200);
#else
  Wire.begin();
  Serial.begin(921600);
#endif
Serial.println(F("ACK CMD ArduCAM Start!"));
// set the CS as an output:
pinMode(SPI_CS, OUTPUT);
// initialize SPI:
SPI.begin();
//Check if the ArduCAM SPI bus is OK
while(1){
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55)
  {
    Serial.println(F("ACK CMD SPI interface Error!"));
    delay(1000);continue;    
  }else{
     Serial.println(F("ACK CMD SPI interface OK.")); break;
    }
}
#if defined (OV2640_CAM)
  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println(F("ACK CMD Can't find OV2640 module!"));
      delay(1000);continue;
    }else{
        Serial.println(F("ACK CMD OV2640 detected."));break;
    }
  }
#elif defined (OV5640_CAM)
  while(1){
    //Check if the camera module type is OV5640
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40)){
      Serial.println(F("ACK CMD Can't find OV5640 module!"));
      delay(1000);continue;
    }else{
      Serial.println(F("ACK CMD OV5640 detected."));break;
    }  
  }
#elif defined (OV5642_CAM)
  while(1){
    //Check if the camera module type is OV5642
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("ACK CMD Can't find OV5642 module!"));
      delay(1000); continue;
    }else{
      Serial.println(F("ACK CMD OV5642 detected."));break;
    } 
  }
#endif
//Change to JPEG capture mode and initialize the OV5642 module
myCAM.set_format(JPEG);
myCAM.InitCAM();
#if defined (OV5642_CAM)
  yCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
#endif
#if defined (OV5640_CAM)
  myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
#endif
myCAM.clear_fifo_flag();
myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);
}
void loop() {
// put your main code here, to run repeatedly:
uint8_t temp =0xff, temp_last =0x00;
bool is_header = false;
if (Serial.available())
{
  #if defined(ESP8266)
    yield();
  #endif
  temp = Serial.read();
  switch (temp)
  {
    case 0:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_160x120);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_160x120"));
    #elif defined (OV5640_CAM)
      myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_320x240"));
    #elif defined (OV5642_CAM)
      myCAM.OV5642_set_JPEG_size(OV5642_320x240);
      Serial.println(F("ACK CMD switch to OV5642_320x240"));
    #endif
    break;
    case 1:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_176x144);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_176x144"));
    #elif defined (OV5640_CAM)
      myCAM.OV5640_set_JPEG_size(OV5640_352x288);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_352x288"));
    #elif defined (OV5642_CAM)
      myCAM.OV5642_set_JPEG_size(OV5642_640x480);
      Serial.println(F("ACK CMD switch to OV5642_640x480"));
    #endif
    break;
    case 2:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_320x240"));
    #elif defined (OV5640_CAM)
      myCAM.OV5640_set_JPEG_size(OV5640_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_640x480"));
    #elif defined (OV5642_CAM)
      myCAM.OV5642_set_JPEG_size(OV5642_1280x960);
      Serial.println(F("ACK CMD switch to OV5642_1280x960"));
    #endif
    break;
    case 3:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_352x288);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_352x288"));
    #elif defined (OV5640_CAM)
      myCAM.OV5640_set_JPEG_size(OV5640_800x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_800x480"));
    #elif defined (OV5642_CAM)
      myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);
      Serial.println(F("ACK CMD switch to OV5642_1600x1200"));
    #endif
    break;
    case 4:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_640x480"));
    #elif defined (OV5640_CAM)
      myCAM.OV5640_set_JPEG_size(OV5640_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_1024x768"));
    #elif defined (OV5642_CAM)
      myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);
      Serial.println(F("ACK CMD switch to OV5642_2048x1536"));
    #endif
    break;
    case 5:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_800x600);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_800x600"));
    #elif defined (OV5640_CAM)
      myCAM.OV5640_set_JPEG_size(OV5640_1280x960);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_1280x960"));
    #elif defined (OV5642_CAM)
      myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_2592x1944"));
    #endif
    break;
    #if (defined (OV5640_CAM)||defined (OV2640_CAM))
    case 6:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_1024x768"));
    #else
      myCAM.OV5640_set_JPEG_size(OV5640_1600x1200);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_1600x1200"));
    #endif
    break;
    case 7:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_1280x1024"));
    #else
      myCAM.OV5640_set_JPEG_size(OV5640_2048x1536);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_2048x1536"))
    #endif
    break;
    case 8:
    #if defined (OV2640_CAM)
      myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_1600x1200"));
    #else
      myCAM.OV5640_set_JPEG_size(OV5640_2592x1944);delay(1000);
      Serial.println(F("ACK CMD switch to OV5640_2592x1944"))
    #endif 
    break;
    #endif
    case 0x10:
    mode = 1;
    start_capture = 1;
    Serial.println(F("ACK CMD CAM start single shoot."));
    break;
    case 0x11:
    myCAM.set_format(JPEG);
    myCAM.InitCAM();
    myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
    break;
    case 0x20:
    mode = 2;
    start_capture = 2;
    Serial.println(F("ACK CMD CAM start video streaming."));
    break;
    case 0x30:
    mode = 3;
    start_capture = 3;
    Serial.println(F("ACK CMD CAM start single shoot."));
    break;
    default:
    break;
  }
}
if (mode == 1)
{
  if (start_capture == 1)
  {
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    start_capture = 0;
  }
  if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println(F("ACK CMD CAM Capture Done!"));
    read_fifo_burst(myCAM);
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
  }
}
else if (mode == 2)
{
  while (1)
  {
    #if defined(ESP8266)
      yield();
    #endif
    temp = Serial.read();
    if (temp == 0x21)
    {
      start_capture = 0;
      mode = 0;
      Serial.println(F("ACK CMD CAM stop video streaming!"));
      break;
    }
    if (start_capture == 2)
    {
      myCAM.flush_fifo();
      myCAM.clear_fifo_flag();
      //Start capture
      myCAM.start_capture();
      start_capture = 0;
    }
    if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    {
      uint32_t length = 0;
      length = myCAM.read_fifo_length();
      if ((length >= MAX_FIFO_SIZE) | (length == 0))
      {
      myCAM.clear_fifo_flag();
      start_capture = 2;
      continue;
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();//Set fifo burst mode
      temp =  SPI.transfer(0x00);
      length --;
      while ( length-- )
      {
        #if defined(ESP8266)
        yield();
        #endif
        temp_last = temp;
        temp =  SPI.transfer(0x00);
        if (is_header == true)
        {
          Serial.write(temp);
        }
        else if ((temp == 0xD8) & (temp_last == 0xFF))
        {
          is_header = true;
          Serial.write(temp_last);
          Serial.write(temp);
        }
        if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
        break;
        delayMicroseconds(15);
      }
      myCAM.CS_HIGH();
      myCAM.clear_fifo_flag();
      start_capture = 2;
      is_header = false;
    }
  }
}
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
uint8_t temp, temp_last;
uint32_t length = 0;
length = myCAM.read_fifo_length();
Serial.println(length, DEC);
if (length >= MAX_FIFO_SIZE) //512 kb
{
  Serial.println(F("Over size."));
  return 0;
}
if (length == 0 ) //0 kb
{
  Serial.println(F("Size is 0."));
  return 0;
}
myCAM.CS_LOW();
myCAM.set_fifo_burst();//Set fifo burst mode
temp =  SPI.transfer(0x00);
length --;
while ( length-- )
{
  #if defined(ESP8266)
  yield();
  #endif
  temp_last = temp;
  temp =  SPI.transfer(0x00);
  if (is_header == true)
  {
   Serial.write(temp);
  }
  else if ((temp == 0xD8) & (temp_last == 0xFF))
  {
    is_header = true;
    Serial.println(F("ACK IMG"));
    Serial.write(temp_last);
    Serial.write(temp);
  }
  if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    break;
  delayMicroseconds(15);
}
myCAM.CS_HIGH();
is_header = false;
return 1;
}
