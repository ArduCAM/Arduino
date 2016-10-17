// ArduCAM Mini demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 5MP camera, and can run on any Arduino platform.
// This demo was made for ArduCAM_Mini_5MP_Plus.
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
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini_5MP_Plus
// and use Arduino IDE 1.5.2 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5640_MINI_5MP_PLUS)||defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define BMPIMAGEOFFSET 66

// set pin 7 as the slave select for the digital pot:
const int CS = 7;
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;

#if defined (OV5640_MINI_5MP_PLUS)
ArduCAM myCAM(OV5640, CS);
#else
ArduCAM myCAM(OV5642, CS);
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

  Serial.println("ACK CMD ArduCAM Start!");

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
    Serial.println("ACK CMD SPI interface Error!");  
    while(1);
  }

   #if defined (OV5640_MINI_5MP_PLUS)
    //Check if the camera module type is OV5640
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40))
      Serial.println("ACK CMD Can't find OV5640 module!");
    else
      Serial.println("ACK CMD OV5640 detected.");
  #else
    //Check if the camera module type is OV5642
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42))
      Serial.println("ACK CMD Can't find OV5642 module!");
    else
      Serial.println("ACK CMD OV5642 detected.");
  #endif

  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);

  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);

}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t temp= 0xff,temp_last =0;
  bool is_header = false;
  if (Serial.available())
  {
    temp = Serial.read();
    switch (temp)
    {
      case 0:
      temp = 0xff;
      #if defined (OV5640_MINI_5MP_PLUS)
      myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
      Serial.println("ACK CMD switch to OV5640_320x240");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
        Serial.println("ACK CMD switch to OV5642_320x240");
       #endif
        break;
      case 1:
       temp = 0xff;
       #if defined (OV5640_MINI_5MP_PLUS)
       myCAM.OV5640_set_JPEG_size(OV5640_352x288);delay(1000);
       Serial.println("ACK CMD switch to OV5640_352x288");
      #else
       myCAM.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
       Serial.println("ACK CMD switch to OV5642_640x480");
      #endif
        break;
      case 2:
       temp = 0xff;
       #if defined (OV5640_MINI_5MP_PLUS)
       myCAM.OV5640_set_JPEG_size(OV5640_640x480);delay(1000);
       Serial.println("ACK CMD switch to OV5640_640x480");
       #else
        myCAM.OV5642_set_JPEG_size(OV5642_1280x960);delay(1000);
        Serial.println("ACK CMD switch to OV5642_1280x960");
       #endif
        break;
      case 3:
       temp = 0xff;
      #if defined (OV5640_MINI_5MP_PLUS)
       myCAM.OV5640_set_JPEG_size(OV5640_800x480);delay(1000);
       Serial.println("ACK CMD switch to OV5640_800x480");
       #else
        myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);
        Serial.println("ACK CMD switch to OV5642_1600x1200");
       #endif
        break;
      case 4:
       temp = 0xff;
      #if defined (OV5640_MINI_5MP_PLUS)
       myCAM.OV5640_set_JPEG_size(OV5640_1024x768);delay(1000);
       Serial.println("ACK CMD switch to OV5640_1024x768");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);
        Serial.println("ACK CMD switch to OV5642_2048x1536");
      #endif
       break;
      case 5:
       temp = 0xff;
      #if defined (OV5640_MINI_5MP_PLUS)
       myCAM.OV5640_set_JPEG_size(OV5640_1280x960);delay(1000);
       Serial.println("ACK CMD switch to OV5640_1280x960");
      #else
        myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
        Serial.println("ACK CMD switch to OV5642_2592x1944");
        #endif
        break;
       #if defined (OV5640_MINI_5MP_PLUS)
        case 6:
         temp = 0xff;
        myCAM.OV5640_set_JPEG_size(OV5640_1600x1200);delay(1000);
        Serial.println("ACK CMD switch to OV5640_1600x1200");
        break;
      case 7:
       temp = 0xff;
        myCAM.OV5640_set_JPEG_size(OV5640_2048x1536);delay(1000);
        Serial.println("ACK CMD switch to OV5640_2048x1536");
        break;
      case 8:
       temp = 0xff;
        myCAM.OV5640_set_JPEG_size(OV5640_2592x1944);delay(1000);
          Serial.println("ACK CMD switch to OV5640_2592x1944");
        break;
      #endif
      case 0x10:
        mode = 1;
        start_capture = 1;
        Serial.println("ACK CMD CAM start single shoot.");
        break;
      case 0x11:
        myCAM.set_format(JPEG);
        myCAM.InitCAM();
        myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
        break;
      case 0x20:
        mode = 2;
        start_capture = 2;
        Serial.println("ACK CMD CAM start video streaming.");
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
        Serial.println("ACK CMD CAM stop video streaming!");
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
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0; 
  length = myCAM.read_fifo_length();
  Serial.println(length, DEC);

  if (length >= MAX_FIFO_SIZE) //512 kb
  {
    Serial.println("ACK CMD Over size.");
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println("ACK CMD Size is 0.");
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
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
       Serial.println("ACK IMG");
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
