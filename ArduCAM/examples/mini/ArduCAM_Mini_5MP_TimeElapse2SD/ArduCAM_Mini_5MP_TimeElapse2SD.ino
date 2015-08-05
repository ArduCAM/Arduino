// ArduCAM demo (C)2015 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules, and can run on any Arduino platform.
//
// This demo was made for Omnivision OV5642 5MP sensor.
// It will run the ArduCAM Mini 5MP as a real 5MP digital camera, provide JPEG capture.
// The demo sketch will do the following tasks:
// 1. Set the sensor to JPEG mode.
// 2. Capture and buffer the image to FIFO every 5 seconds
// 3. Store the image to Micro SD/TF card with JPEG format in sequential.
// 4. Resolution can be changed by myCAM.OV5642_set_JPEG_size() function.
// This program requires the ArduCAM V3.4.0 (or later) library and ArduCAM Mini 5MP shield
// and use Arduino IDE 1.5.2 compiler or above


#include <UTFT_SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#if defined(__arm__)
#include <itoa.h>
#endif

#define SD_CS 9
// set pin 10 as the slave select for SPI:
const int SPI_CS = 4;

ArduCAM myCAM(OV5642, SPI_CS);
UTFT myGLCD(SPI_CS);
boolean isShowFlag = true;

void setup()
{
  uint8_t vid, pid;
  uint8_t temp;
#if defined (__AVR__)
  Wire.begin();
#endif
#if defined(__arm__)
  Wire1.begin();
#endif
  Serial.begin(115200);
  Serial.println("ArduCAM Start!");
  // set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);

  // initialize SPI:
  SPI.begin();
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55)
  {
    Serial.println("SPI interface Error!");
    while (1);
  }


  //Check if the camera module type is OV5642
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if ((vid != 0x56) || (pid != 0x42))
    Serial.println("Can't find OV5642 module!");
  else
    Serial.println("OV5642 detected");


  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);

  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
  //       myCAM.OV5642_set_JPEG_size(OV5642_640x480);
  //       myCAM.OV5642_set_JPEG_size(OV5642_1280x720);
  //       myCAM.OV5642_set_JPEG_size(OV5642_1920x1080);
  //       myCAM.OV5642_set_JPEG_size(OV5642_2048x1563);
  //       myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);
  //Initialize SD Card
  if (!SD.begin(SD_CS))
  {
    //while (1);		//If failed, stop here
    Serial.println("SD Card Error");
  }
  else
    Serial.println("SD Card detected!");
}

void loop()
{
  char str[8];
  File outFile;
  byte buf[256];
  static int i = 0;
  static int k = 0;
  static int n = 0;
  uint8_t temp, temp_last;
  uint8_t start_capture = 0;
  int total_time = 0;

  start_capture = 1;
  delay(5000);

  if (start_capture)
  {
    //Flush the FIFO
    myCAM.flush_fifo();
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    Serial.println("Start Capture");
  }

  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));

  Serial.println("Capture Done!");
  //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if (! outFile)
  {
    Serial.println("open file failed");
    return;
  }
  total_time = millis();

  i = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  temp = SPI.transfer(0x00);

  //Read JPEG data from FIFO
  while ( (temp != 0xD9) | (temp_last != 0xFF) )
  {
    temp_last = temp;
    temp = SPI.transfer(0x00);;
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
  //Write the remain bytes in the buffer
  if (i > 0)
  {
    myCAM.CS_HIGH();
    outFile.write(buf, i);
  }
  //Close the file
  outFile.close();
  total_time = millis() - total_time;
  Serial.print("Total time used:");
  Serial.print(total_time, DEC);
  Serial.println(" millisecond");
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Clear the start capture flag
  start_capture = 0;

}



