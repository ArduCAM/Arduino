// ArduCAM Mini demo (C)2015 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 2MP camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini OV2640 2MP Camera.
// It needs to be used in combination with PC software.
// It can take photo continuously as video streaming.
//
// The demo sketch will do the following tasks:
// 1. Set the camera to JEPG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x10,camera will capture a JPEG photo and buffer the image to FIFO.Then write datas to Serial port.
// 5. If receive 0x20,camera will capture JPEG photo and write datas continuously.Stop when receive 0x21.
// 6. If receive 0x30,camera will capture a BMP  photo and buffer the image to FIFO.Then write datas to Serial port.
// 7. If receive 0x11 ,set camera to JPEG output mode.
// 8. If receive 0x31 ,set camera to BMP  output mode.
// This program requires the ArduCAM V3.4.1 (or later) library and ArduCAM Mini 2MP camera
// and use Arduino IDE 1.5.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#define BMPIMAGEOFFSET 66


#if defined(ESP8266)
// set GPIO15 as the slave select for the digital pot:
const int CS = 16;
#else
// set pin 10 as the slave select for the digital pot:
const int CS = 10;
#endif
int mode = 0;

const char bmp_header[BMPIMAGEOFFSET] PROGMEM =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};

ArduCAM myCAM(OV2640, CS);
void read_fifo_burst(ArduCAM myCAM);

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
  //Serial.println(temp);
  if (temp != 0x55)
  {
    Serial.println("SPI1 interface Error!");
    //while(1);
  }

  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);  
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26) || (pid != 0x42))
    Serial.println("Can't find OV2640 module!");
  else
    Serial.println("OV2640 detected.");

  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp, temp_last;
  uint8_t start_capture = 0;
  bool is_header = false;
  if (Serial.available())
  {
    temp = Serial.read();
    switch (temp)
    {
      case 0:
        myCAM.OV2640_set_JPEG_size(OV2640_160x120);
        break;
      case 1:
        myCAM.OV2640_set_JPEG_size(OV2640_176x144);
        break;
      case 2:
        myCAM.OV2640_set_JPEG_size(OV2640_320x240);
        break;
      case 3:
        myCAM.OV2640_set_JPEG_size(OV2640_352x288);
        break;
      case 4:
        myCAM.OV2640_set_JPEG_size(OV2640_640x480);
        break;
      case 5:
        myCAM.OV2640_set_JPEG_size(OV2640_800x600);
        break;
      case 6:
        myCAM.OV2640_set_JPEG_size(OV2640_1024x768);
        break;
      case 7:
        myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);
        break;
      case 8:
        myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
        break;

      case 0x10:
        mode = 1;
        start_capture = 1;
        Serial.println("CAM start single shoot.");
        break;
      case 0x11:
        myCAM.set_format(JPEG);
        myCAM.InitCAM();
        break;
      case 0x20:
        mode = 2;
        start_capture = 2;
        Serial.println("CAM start contrues shoots.");
        break;
      case 0x30:
        mode = 3;
        start_capture = 3;
        Serial.println("CAM start single shoot.");
        break;
      case 0x31:
        myCAM.set_format(BMP);
        myCAM.InitCAM();
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
        uint32_t length = myCAM.read_fifo_length();
        if ((length >= 393216) | (length == 0))
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
          Serial.write(temp);
          if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
            break;
          delayMicroseconds(12);
        }
        myCAM.CS_HIGH();
        myCAM.clear_fifo_flag();
        start_capture = 2;
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
      Serial.println("CAM Capture Done!");

      uint8_t temp, temp_last;
      uint32_t length = 0;
      length = myCAM.read_fifo_length();
      if (length >= 393216 ) // 384kb
      {
        Serial.println("Over size.");
        myCAM.clear_fifo_flag();
        return;
      }

      if (length == 0 ) //0 kb
      {
        Serial.println("Size is 0.");
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

void read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp, temp_last;
  uint32_t length = 0;
  length = myCAM.read_fifo_length();
  if (length >= 393216 ) // 384kb
  {
    Serial.println("Over size.");
    return;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println("Size is 0.");
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  //SPI.transfer(0x00);
  length--;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);//read a byte from spi
    Serial.write(temp);
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      break;
    delayMicroseconds(12);
  }
  myCAM.CS_HIGH();
}