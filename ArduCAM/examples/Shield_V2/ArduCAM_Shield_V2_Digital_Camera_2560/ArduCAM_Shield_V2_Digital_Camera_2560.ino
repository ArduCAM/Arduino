// ArduCAM demo (C)2018 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules//This demo can only work on ARDUCAM_SHIELD_V2 platform.
//This demo is compatible with ESP8266
// It will run the ArduCAM as a real 2MP/3MP/5MP digital camera, provide both preview and JPEG capture.
// The demo sketch will do the following tasks:
// 1. Set the sensor to BMP preview output mode.
// 2. Switch to JPEG mode when shutter buttom pressed.
// 3. Capture and buffer the image to FIFO.
// 4. Store the image to Micro SD/TF card with JPEG format.
// 5. Resolution can be changed by myCAM.OV5642_set_JPEG_size() function.
// 6.Record a short movie when shutter buttom hold on for 3 seconds.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM shield V2
// and use Arduino IDE 1.6.8 compiler or above

#include <UTFT_SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo was made for Omnivision MT9D111A/MT9D111B/MT9M112/MT9V111_CAM/
//                                  MT9M001/MT9T112/MT9D112/OV7670/OV7675/
//                                  OV7725/OV2640/OV5640/OV5642 sensor.
#if !(defined ARDUCAM_SHIELD_V2 && (defined MT9D111A_CAM|| defined MT9D111B_CAM || defined MT9M112_CAM \
                                 || defined MT9V111_CAM || defined MT9M001_CAM || defined MT9T112_CAM \
                                 || defined MT9D112_CAM || defined OV7670_CAM  || defined OV7675_CAM \
                                 || defined OV7725_CAM  || defined OV2640_CAM  || defined OV5640_CAM \
                                 || defined OV5642_CAM  || defined OV3640_CAM))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#if defined(__arm__)
#include <itoa.h>
#endif

#if defined(ESP8266)
#define SD_CS 0
const int SPI_CS = 16;
#else
#define SD_CS 9
const int SPI_CS = 10;
#endif

#define   FRAMES_NUM    0x07
#define   rate     0x0a
#define AVIOFFSET 240



boolean isShowFlag = true;
bool is_header = false;
unsigned long previous_time = 0;

char str[8];
File outFile;
File movieFile;
byte buf[256];
static int i = 0;
static int k = 0;
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
uint8_t start_capture = 0;
int total_time = 0;
unsigned long movi_size = 0;
unsigned long jpeg_size = 0;
const char zero_buf[4] = {0x00, 0x00, 0x00, 0x00};
const int avi_header[AVIOFFSET] PROGMEM = {
  0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x01, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, rate, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54,
  0x10, 0x00, 0x00, 0x00, 0x6F, 0x64, 0x6D, 0x6C, 0x64, 0x6D, 0x6C, 0x68, 0x04, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};
void print_quartet(unsigned long i, File fd) {
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);
}

#if defined (MT9D111A_CAM)
ArduCAM myCAM(MT9D111_A, SPI_CS);
#elif defined (MT9D111B_CAM)
ArduCAM myCAM(MT9D111_B, SPI_CS);
#elif defined (MT9M112_CAM)
ArduCAM myCAM(MT9M112, SPI_CS);
#elif defined (MT9V111_CAM)
ArduCAM myCAM(MT9V111, SPI_CS);
#elif defined (MT9M001_CAM)
ArduCAM myCAM(MT9M001, SPI_CS);
#elif defined (MT9T112_CAM)
ArduCAM myCAM(MT9T112, SPI_CS);
#elif defined (MT9D112_CAM)
ArduCAM myCAM(MT9D112, SPI_CS);
#elif defined (OV7670_CAM)
ArduCAM myCAM(OV7670, SPI_CS);
#elif defined (OV7675_CAM)
ArduCAM myCAM(OV7675, SPI_CS);
#elif defined (OV7725_CAM)
ArduCAM myCAM(OV7725, SPI_CS);
#elif defined (OV2640_CAM)
ArduCAM myCAM(OV2640, SPI_CS);
#elif defined (OV3640_CAM)
ArduCAM myCAM(OV3640, SPI_CS);
#elif defined (OV5640_CAM)
ArduCAM myCAM(OV5640, SPI_CS);
#elif defined (OV5642_CAM)
ArduCAM myCAM(OV5642, SPI_CS);
#endif
UTFT myGLCD(SPI_CS);

void setup()
{
  uint8_t vid, pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  // set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  // initialize SPI:
  SPI.begin();
  
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  
  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      Serial.println(F("SPI interface Error!"));
      delay(1000); continue;
    } else {
      Serial.println(F("SPI interface OK!")); break;
    }
  }
  //Change MCU mode
  myCAM.set_mode(MCU2LCD_MODE);
  myGLCD.InitLCD();
#if defined (OV2640_CAM)
  while (1) {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
      Serial.println(F("Can't find OV2640 module!"));
      delay(1000); continue;
    } else {
      Serial.println(F("OV2640 detected.")); break;
    }
  }
#elif defined (OV3640_CAM)
  while (1) {
    //Check if the camera module type is OV3640
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
    if ((vid != 0x36) || (pid != 0x4C)) {
      Serial.println(F("Can't find OV3640 module!"));
      delay(1000); continue;
    } else {
      Serial.println(F("OV3640 detected.")); break;
    }
  }
#elif defined (OV5640_CAM)
  while (1) {
    //Check if the camera module type is OV5640
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40)) {
      Serial.println(F("Can't find OV5640 module!"));
      delay(1000); continue;
    } else {
      Serial.println(F("OV5640 detected.")); break;
    }
  }
#elif defined (OV5642_CAM)
  while (1) {
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)) {
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000); continue;
    } else {
      Serial.println(F("OV5642 detected.")); break;
    }
  }
#endif
  //Initialize SD Card
  while (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error")); delay(1000);
  }
  Serial.println(F("SD Card detected!"));
  //Change to BMP capture mode and initialize the OV5642 module
  myCAM.set_format(BMP);
  myCAM.InitCAM();
}
void loop()
{
  while (1) {
#if defined(ESP8266)
    yield();
#endif
    if (!myCAM.get_bit(ARDUCHIP_TRIG, VSYNC_MASK))    //New Frame is coming
    {
      myCAM.set_mode(MCU2LCD_MODE);     //Switch to MCU
      myGLCD.resetXY();
      myCAM.set_mode(CAM2LCD_MODE);     //Switch to CAM
      while (!myCAM.get_bit(ARDUCHIP_TRIG, VSYNC_MASK));  //Wait for VSYNC is gone
    }
    else if (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
    {
      previous_time = millis();
      while (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
      {
        if ((millis() - previous_time) > 1500)
        {
          saveShortMovie();
          //Serial.println("Save video Streaming ");
        }
      }
      if ((millis() - previous_time) < 1500)
      {
        saveImageJPEG();
      }
    }
  }

}




void saveImageJPEG() {
  myCAM.set_mode(MCU2LCD_MODE);
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
#if defined (OV2640_CAM)
  myCAM.OV2640_set_JPEG_size(OV2640_640x480); delay(1000);
#elif defined (OV3640_CAM)
  myCAM.OV3640_set_JPEG_size(OV3640_320x240); delay(1000);
#elif defined (OV5640_CAM)
  myCAM.OV5640_set_JPEG_size(OV5640_320x240); delay(1000);
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);    //VSYNC is active HIGH
#elif defined (OV5642_CAM)
  myCAM.OV5642_set_JPEG_size(OV5642_320x240); delay(1000);
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);    //VSYNC is active HIGH
#endif
  myCAM.write_reg(ARDUCHIP_FRAMES, 0);
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Start Capture"));
  while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  Serial.println(F("Capture Done."));
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
    return ;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return;
  }
  //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if (! outFile)
  {
    Serial.println(F("File open failed"));
    return;
  }
  i = 0;
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
      //Close the file
      outFile.close();
      Serial.println(F("Image save OK."));
      is_header = false;
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
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Clear the start capture flag
  start_capture = 0;
  myCAM.set_format(BMP);
  myCAM.InitCAM();

}


uint8_t saveShortMovie()
{
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  static int i = 0;
  static int k = 0;
  unsigned long position = 0;
  uint16_t frame_cnt = 0;
  uint8_t remnant = 0;
  char str[8];
  byte buf[256];

 myCAM.set_mode(MCU2LCD_MODE);
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
#if defined (OV2640_CAM)
  myCAM.OV2640_set_JPEG_size(OV2640_640x480); delay(1000);
#elif defined (OV3640_CAM)
  myCAM.OV3640_set_JPEG_size(OV3640_320x240); delay(1000);
#elif defined (OV5640_CAM)
  myCAM.OV5640_set_JPEG_size(OV5640_320x240); delay(1000);
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);    //VSYNC is active HIGH
#elif defined (OV5642_CAM)
  myCAM.OV5642_set_JPEG_size(OV5642_320x240); delay(1000);
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);    //VSYNC is active HIGH
#endif
  //Flush the FIFO
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Start recording video..."));
  while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)){;}
  length = myCAM.read_fifo_length();
  if ( length < 0x3FFFFF) {
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
#if defined (ESP8266)
      yield();
#endif
    }
    Serial.println(F("Record video finish."));
    total_time = millis() - total_time;
  } else {
    Serial.println(F("Record video finish"));
    total_time = millis() - total_time;
  }
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  Serial.println("Writing the video data to the SD card...");
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
  movi_size = 0;
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
#if defined (ESP8266)
    yield();
#endif
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
  print_quartet(movi_size + 12 * frame_cnt + 4, outFile); //riff file size
  //overwrite hdrl
  unsigned long us_per_frame = 1000000 / rate; //(per_usec); //hdrl.avih.us_per_frame
  outFile.seek(0x20);
  print_quartet(us_per_frame, outFile);
  unsigned long max_bytes_per_sec = movi_size * rate / frame_cnt; //hdrl.avih.max_bytes_per_sec
  outFile.seek(0x24);
  print_quartet(max_bytes_per_sec, outFile);
  unsigned long tot_frames = frame_cnt;    //hdrl.avih.tot_frames
  outFile.seek(0x30);
  print_quartet(tot_frames, outFile);
  unsigned long frames = frame_cnt; // (TOTALFRAMES); //hdrl.strl.list_odml.frames
  outFile.seek(0xe0);
  print_quartet(frames, outFile);
  outFile.seek(0xe8);
  print_quartet(movi_size, outFile);// size again
  //Close the file
  outFile.close();
  is_header = false;
  Serial.println(F("Movie save OK."));
  myCAM.set_format(BMP);
  myCAM.InitCAM();
  return 1;
}


