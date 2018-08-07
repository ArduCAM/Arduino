// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini camera, and can run on any Arduino platform.
// This demo was made for ArduCAM_Mini_5MP_Plus.
// It needs to be used in combination with PC software.
// It can test OV3640 functions 
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini_5MP_Plus
// and use Arduino IDE 1.6.8 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo can only work on OV3640_MINI_3MP platform.
#if !(defined OV3640_MINI_3MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define BMPIMAGEOFFSET 66
const char bmp_header[BMPIMAGEOFFSET] PROGMEM =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};
// set pin 7 as the slave select for the digital pot:
const int CS = 7;
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;
ArduCAM myCAM( OV3640, CS );
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
pinMode(CS, OUTPUT);
digitalWrite(CS, HIGH);
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
    Serial.println(F("ACK CMD SPI interface Error!"));
    delay(1000);continue;
  }else{
    Serial.println(F("ACK CMD SPI interface OK."));break;
  }
}
  while(1){
    //Check if the camera module type is OV3640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
    if ((vid != 0x36 ) && ( pid != 0x4C )){
      Serial.println(F("ACK CMD Can't find OV3640 module!"));
      delay(1000);continue;
    }
    else{
      Serial.println(F("ACK CMD OV3640 detected."));break;
    } 
  }
//Change to JPEG capture mode and initialize the OV5642 module
myCAM.set_format(JPEG);
myCAM.InitCAM();
myCAM.OV3640_set_JPEG_size(OV3640_320x240);
delay(1000);
myCAM.clear_fifo_flag();
}
void loop() {
// put your main code here, to run repeatedly:
uint8_t temp = 0xff, temp_last = 0;
bool is_header = false;
if (Serial.available())
{
  temp = Serial.read();
  switch (temp)
  {
     case 0:
      myCAM.OV3640_set_JPEG_size(OV3640_176x144);delay(1000);
      Serial.println(F("ACK CMD switch to OV2640_160x120"));
      temp = 0xff;
    break;
    case 1:
      myCAM.OV3640_set_JPEG_size(OV3640_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_320x240"));
    temp = 0xff;
    break;
    case 2: 
      myCAM.OV3640_set_JPEG_size(OV3640_352x288);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_352x288"));
    temp = 0xff;
    break;
    case 3:
    temp = 0xff;
      myCAM.OV3640_set_JPEG_size(OV3640_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_640x480"));
    break;
    case 4:
    temp = 0xff;
      myCAM.OV3640_set_JPEG_size(OV3640_800x600);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_800x600"));
    break;
    case 5:
    temp = 0xff;
   
      myCAM.OV3640_set_JPEG_size(OV3640_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1024x768"));
    break;
    case 6:
    temp = 0xff;
      myCAM.OV3640_set_JPEG_size(OV3640_1280x960);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1280x960"));
    break;
    case 7:
    temp = 0xff;
      myCAM.OV3640_set_JPEG_size(OV3640_1600x1200);delay(1000);
      Serial.println(F("ACK CMD switch to OV3640_1600x1200"));
    break;
    case 8:
    temp = 0xff;
     myCAM.OV3640_set_JPEG_size(OV3640_2048x1536);delay(1000);
     Serial.println(F("ACK CMD switch to OV3640_2048x1536"));
    break;
    case 0x10:
    mode = 1;
    temp = 0xff;
    start_capture = 1;
    Serial.println(F("ACK CMD CAM start single shoot."));
    break;
    case 0x11: 
    temp = 0xff;
    myCAM.set_format(JPEG);
    myCAM.InitCAM();
    break;
    case 0x20:
    mode = 2;
    temp = 0xff;
    start_capture = 2;
    Serial.println(F("ACK CMD CAM start video streaming."));
    break;
    case 0x30:
    mode = 3;
    temp = 0xff;
    start_capture = 3;
    Serial.println(F("CAM start single shoot."));
    break;
    case 0x31:
    temp = 0xff;
    myCAM.set_format(BMP);
    myCAM.InitCAM();
    myCAM.wrSensorReg16_8(0x3818, 0x81);
    myCAM.wrSensorReg16_8(0x3621, 0xA7);
    break;
    case 0x40:
    myCAM.OV3640_set_Light_Mode(Auto);temp = 0xff;
    Serial.println(F("ACK CMD Set to Auto"));break;
     case 0x41:
    myCAM.OV3640_set_Light_Mode(Sunny);temp = 0xff;
    Serial.println(F("ACK CMD Set to Sunny"));break;
     case 0x42:
    myCAM.OV3640_set_Light_Mode(Cloudy);temp = 0xff;
    Serial.println(F("ACK CMD Set to Cloudy"));break;
     case 0x43:
    myCAM.OV3640_set_Light_Mode(Office);temp = 0xff;
    Serial.println(F("ACK CMD Set to Office"));break;
   case 0x44:
    myCAM.OV3640_set_Light_Mode(Home);   temp = 0xff;
   Serial.println(F("ACK CMD Set to Home"));break;
   case 0x50:
    myCAM.OV3640_set_Color_Saturation(Saturation2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+2"));break;
   case 0x51:
     myCAM.OV3640_set_Color_Saturation(Saturation1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+1"));break;
   case 0x52:
    myCAM.OV3640_set_Color_Saturation(Saturation0); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+0"));break;
    case 0x53:
    myCAM. OV3640_set_Color_Saturation(Saturation_1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation-1"));break;
    case 0x54:
     myCAM.OV3640_set_Color_Saturation(Saturation_2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation-2"));break; 
   case 0x60:
    myCAM.OV3640_set_Brightness(Brightness2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness+2"));break;
   case 0x61:
     myCAM.OV3640_set_Brightness(Brightness1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness+1"));break;
   case 0x62:
    myCAM.OV3640_set_Brightness(Brightness0); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness+0"));break;
    case 0x63:
    myCAM. OV3640_set_Brightness(Brightness_1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness-1"));break;
    case 0x64:
     myCAM.OV3640_set_Brightness(Brightness_2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness-2"));break; 
    case 0x70:
      myCAM.OV3640_set_Contrast(Contrast2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast+2"));break; 
    case 0x71:
      myCAM.OV3640_set_Contrast(Contrast1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast+1"));break;
     case 0x72:
      myCAM.OV3640_set_Contrast(Contrast0);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast+0"));break;
    case 0x73:
      myCAM.OV3640_set_Contrast(Contrast_1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast-1"));break;
   case 0x74:
      myCAM.OV3640_set_Contrast(Contrast_2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast-2"));break;
   case 0x80:
    myCAM.OV3640_set_Special_effects(Antique);temp = 0xff;
    Serial.println(F("ACK CMD Set to Antique"));break;
   case 0x81:
    myCAM.OV3640_set_Special_effects(Bluish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Bluish"));break;
   case 0x82:
    myCAM.OV3640_set_Special_effects(Greenish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Greenish"));break;  
   case 0x83:
    myCAM.OV3640_set_Special_effects(Reddish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Reddish"));break;  
   case 0x84:
    myCAM.OV3640_set_Special_effects(Yellowish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Yellowish"));break;
    case 0x85:
    myCAM.OV3640_set_Special_effects(BW);temp = 0xff;
    Serial.println(F("ACK CMD Set to BW"));break; 
  case 0x86:
    myCAM.OV3640_set_Special_effects(Negative);temp = 0xff;
    Serial.println(F("ACK CMD Set to Negative"));break; 
  case 0x87:
    myCAM.OV3640_set_Special_effects(Normal);temp = 0xff;
    Serial.println(F("ACK CMD Set to Normal"));break;      
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
    Serial.println(F("ACK CMD CAM Capture Done."));
    read_fifo_burst(myCAM);
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
  }
}
else if (mode == 2)
{
  while (1)
  {
    temp = Serial.read();
    if (temp == 0x21)
    {
      start_capture = 0;
      mode = 0;
      Serial.println(F("ACK CMD CAM stop video streaming."));
      break;
    }
    switch (temp)
    {
       case 0x40:
    myCAM.OV3640_set_Light_Mode(Auto);temp = 0xff;
    Serial.println(F("ACK CMD Set to Auto"));break;
     case 0x41:
    myCAM.OV3640_set_Light_Mode(Sunny);temp = 0xff;
    Serial.println(F("ACK CMD Set to Sunny"));break;
     case 0x42:
    myCAM.OV3640_set_Light_Mode(Cloudy);temp = 0xff;
    Serial.println(F("ACK CMD Set to Cloudy"));break;
     case 0x43:
    myCAM.OV3640_set_Light_Mode(Office);temp = 0xff;
    Serial.println(F("ACK CMD Set to Office"));break;
   case 0x44:
    myCAM.OV3640_set_Light_Mode(Home);   temp = 0xff;
   Serial.println(F("ACK CMD Set to Home"));break;
   case 0x50:
    myCAM.OV3640_set_Color_Saturation(Saturation2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+2"));break;
   case 0x51:
     myCAM.OV3640_set_Color_Saturation(Saturation1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+1"));break;
   case 0x52:
    myCAM.OV3640_set_Color_Saturation(Saturation0); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+0"));break;
    case 0x53:
    myCAM. OV3640_set_Color_Saturation(Saturation_1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation-1"));break;
    case 0x54:
     myCAM.OV3640_set_Color_Saturation(Saturation_2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation-2"));break; 
   case 0x60:
    myCAM.OV3640_set_Brightness(Brightness2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness+2"));break;
   case 0x61:
     myCAM.OV3640_set_Brightness(Brightness1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness+1"));break;
   case 0x62:
    myCAM.OV3640_set_Brightness(Brightness0); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness+0"));break;
    case 0x63:
    myCAM. OV3640_set_Brightness(Brightness_1); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness-1"));break;
    case 0x64:
     myCAM.OV3640_set_Brightness(Brightness_2); temp = 0xff;
     Serial.println(F("ACK CMD Set to Brightness-2"));break; 
    case 0x70:
      myCAM.OV3640_set_Contrast(Contrast2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast+2"));break; 
    case 0x71:
      myCAM.OV3640_set_Contrast(Contrast1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast+1"));break;
     case 0x72:
      myCAM.OV3640_set_Contrast(Contrast0);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast+0"));break;
    case 0x73:
      myCAM.OV3640_set_Contrast(Contrast_1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast-1"));break;
   case 0x74:
      myCAM.OV3640_set_Contrast(Contrast_2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Contrast-2"));break;
   case 0x80:
    myCAM.OV3640_set_Special_effects(Antique);temp = 0xff;
    Serial.println(F("ACK CMD Set to Antique"));break;
   case 0x81:
    myCAM.OV3640_set_Special_effects(Bluish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Bluish"));break;
   case 0x82:
    myCAM.OV3640_set_Special_effects(Greenish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Greenish"));break;  
   case 0x83:
    myCAM.OV3640_set_Special_effects(Reddish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Reddish"));break;  
   case 0x84:
    myCAM.OV3640_set_Special_effects(Yellowish);temp = 0xff;
    Serial.println(F("ACK CMD Set to Yellowish"));break;
    case 0x85:
    myCAM.OV3640_set_Special_effects(BW);temp = 0xff;
    Serial.println(F("ACK CMD Set to BW"));break; 
  case 0x86:
    myCAM.OV3640_set_Special_effects(Negative);temp = 0xff;
    Serial.println(F("ACK CMD Set to Negative"));break; 
  case 0x87:
    myCAM.OV3640_set_Special_effects(BWnegative);temp = 0xff;
    Serial.println(F("ACK CMD Set to BWnegative"));break;   
   case 0x88:
    myCAM.OV3640_set_Special_effects(Normal);temp = 0xff;
    Serial.println(F("ACK CMD Set to Normal"));break;    
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
      myCAM.clear_fifo_flag();
      start_capture = 2;
      is_header = false;
    }
  }
}
else if (mode == 3)
{
  if (start_capture == 3)
  {
    //Flush the FIFO
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    start_capture = 0;
  }
  if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println(F("ACK CMD CAM Capture Done."));
    uint8_t temp, temp_last;
    uint32_t length = 0;
    length = myCAM.read_fifo_length();
    if (length >= MAX_FIFO_SIZE ) 
    {
      Serial.println(F("ACK CMD Over size."));
      myCAM.clear_fifo_flag();
      return;
    }
    if (length == 0 ) //0 kb
    {
      Serial.println(F("ACK CMD Size is 0."));
      myCAM.clear_fifo_flag();
      return;
    }
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();//Set fifo burst mode
    
    Serial.write(0xFF);
    Serial.write(0xAA);
    for (temp = 0; temp < BMPIMAGEOFFSET; temp++)
    {
      Serial.write(pgm_read_byte(&bmp_header[temp]));
    }
    SPI.transfer(0x00);
    char VH, VL;
    int i = 0, j = 0;
    for (i = 0; i < 240; i++)
    {
      for (j = 0; j < 320; j++)
      {
        VH = SPI.transfer(0x00);;
        VL = SPI.transfer(0x00);;
        Serial.write(VL);
        delayMicroseconds(12);
        Serial.write(VH);
        delayMicroseconds(12);
      }
    }
    Serial.write(0xBB);
    Serial.write(0xCC);
    myCAM.CS_HIGH();
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
  }
}
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp = 0, temp_last = 0;
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
