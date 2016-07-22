// ArduCAM Mini demo (C)2015 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 5MP camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini OV5642 5MP Camera.
// It needs to be used in combination with PC software.
// It can take photo continuously as video streaming.
//You can change the FRAMES_NUM count to change the number of the picture.
//IF the FRAMES_NUM is 0X00, take one photos
//IF the FRAMES_NUM is 0X02, take two photos
//IF the FRAMES_NUM is 0X03, take three photos
//IF the FRAMES_NUM is 0X04, take four photos
//IF the FRAMES_NUM is 0X05, take five photos
//IF the FRAMES_NUM is 0X06, take six photos
//IF the FRAMES_NUM is 0X07, continue shooting until the FIFO is full
// The demo sketch will do the following tasks:
// 1. Set the camera to JEPG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x10,camera will capture a JPEG photo and buffer the image to FIFO.Then write datas to Serial port.
// 5. If receive 0x20,camera will capture JPEG photo and write datas continuously.Stop when receive 0x21.
// This program requires the ArduCAM V3.4.1 (or later) library and ArduCAM Mini 5MP camera
// and use Arduino IDE 1.5.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#define   FIFO_SIZE     0x07FFFFF
#define   FRAMES_NUM    0x06
// set pin 10 as the slave select for the digital pot:
const int CS = 7;
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;

ArduCAM myCAM(OV5640, CS);
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
  Serial.begin(921600);
  Serial.println("ArduCAM Start!");

  // set the CS as an output:
  pinMode(CS, OUTPUT);

  // initialize SPI:
  SPI.begin();
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  
 // Serial.println(temp,HEX);
  if (temp != 0x55)
  {
    Serial.println("SPI interface Error!");
    //while(1);
  }

  //Check if the camera module type is OV5640
  myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
  if ((vid != 0x56) || (pid != 0x40))
    Serial.println("Can't find OV5640 module!");
  else
    Serial.println("OV5640 detected.");

  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);

  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);

}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp, temp_last;
  bool is_header = false;
  if (Serial.available())
  {
    temp = Serial.read();
    switch (temp)
    {
      
     case 0:
        myCAM.OV5640_set_JPEG_size(OV5640_320x240);
        break;
      case 1:
        myCAM.OV5640_set_JPEG_size(OV5640_352x288);
        break;
      case 2:
        myCAM.OV5640_set_JPEG_size(OV5640_640x480);
        break;
      case 3:
        myCAM.OV5640_set_JPEG_size(OV5640_800x480);
        break;
      case 4:
        myCAM.OV5640_set_JPEG_size(OV5640_1024x768);
        break;
      case 5:
        myCAM.OV5640_set_JPEG_size(OV5640_1280x960);
        break;
      case 6:
        myCAM.OV5640_set_JPEG_size(OV5640_1600x1200);
        break;
      case 7:
        myCAM.OV5640_set_JPEG_size(OV5640_2048x1536);
        break;
      case 8:
        myCAM.OV5640_set_JPEG_size(OV5640_2592x1944);
        break;
      case 0x10:
        mode = 1;
        start_capture = 1;
        Serial.println("CAM start single shoot.");
        break;
      case 0x11:
        myCAM.set_format(JPEG);
        myCAM.InitCAM();
        myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
        break;
      case 0x20:
        mode = 2;
        start_capture = 2;
        Serial.println("CAM start contrues shoots.");
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
      // Serial.println("OK!");
      //Start capture
      myCAM.start_capture();
      start_capture = 0;
    }
    if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    {
      Serial.println("CAM Capture Done!");
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
        Serial.println("CAM stop continuous shoots!");
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
        if ((length >= 524288) | (length == 0))
        {
          myCAM.clear_fifo_flag();
          start_capture = 2;
          continue;
        }
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();//Set fifo burst mode
        //SPI.transfer(0x00);
        length--;
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
            Serial.write(temp_last);
            Serial.write(temp);
          }
          if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
            break;
          delayMicroseconds(12);
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
  if (length >= 0x07fffff) //1M
  {
    Serial.println("Over size.");
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println("Size is 0.");
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  SPI.transfer(0x00);//First byte is 0x00 ,not 0xff
  length--;
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
      Serial.write(temp_last);
      Serial.write(temp);   
    }
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      delay(1000);
    delayMicroseconds(12);
  }
  myCAM.CS_HIGH();
  is_header = false;
}
