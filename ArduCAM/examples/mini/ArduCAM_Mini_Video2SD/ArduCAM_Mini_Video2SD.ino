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

// FC aka XFer: last edit 20181108

// TODO:
// Preallocate output file (create at max length, fill with 0, rewind, then truncate to actual bytes written after capture finished)
// This should improve writing speed and latency a bit

#include <stdint.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"

// DEFINES
//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED|| defined OV2640_MINI_2MP_PLUS || defined OV2640_MINI_2MP|| defined OV3640_MINI_3MP)
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

//#define FISHINO_UNO // Nice UNO board with integrated RTC, microSD, WiFi

#define SERIAL_SPEED 115200
#define BUFFSIZE 512  // 512 is a good buffer size for SD writing. 4096 would be better, on boards with enough RAM (not Arduino Uno of course)
#define FRAME_SIZE OV2640_320x240
#define WIDTH_1 0x40 // Video width in pixel, hex. Here we set 320 (Big Endian: 320 = 0x01 0x40 -> 0x40 0x01). For 640: 0x80
#define WIDTH_2 0x01 // For 640: 0x02  
#define HEIGHT_1 0xF0 // 240 pixels height (0x00 0xF0 -> 0xF0 0x00). For 480: 0xE0
#define HEIGHT_2 0x00 // For 480: 0x01 
#define FPS 0x0F // 15 FPS. Placeholder: will be overwritten at runtime based upon real FPS attained
#define TOTAL_FRAMES 200	// Number of frames to be recorded. If < 256, easier to recognize in header (for manual hex debug)
//set pin 7 as the slave select for SPI:
#define SPI_CS  7
// SD card Select pin:
#define SD_CS 9	// 9 on Arducam adapter Uno and SD shields
#ifdef FISHINO_UNO
#define SD_AUX 10	// Needs to be Output on Fishino Uno for the integrated SD card to work
#endif
#define AVIOFFSET 240 // AVI main header length

//#define DISABLE_SD

// GLOBALS
unsigned long movi_size = 0;
unsigned long jpeg_size = 0;
const char zero_buf[4] = {0x00, 0x00, 0x00, 0x00};
const int avi_header[AVIOFFSET] PROGMEM = {
  0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  WIDTH_1, WIDTH_2, 0x00, 0x00, HEIGHT_1, HEIGHT_2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, FPS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, WIDTH_1, WIDTH_2, 0x00, 0x00, HEIGHT_1, HEIGHT_2, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54,
  0x10, 0x00, 0x00, 0x00, 0x6F, 0x64, 0x6D, 0x6C, 0x64, 0x6D, 0x6C, 0x68, 0x04, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};
#if defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS)
ArduCAM myCAM( OV2640, SPI_CS );
#elif defined (OV3640_MINI_3MP)
ArduCAM myCAM( OV3640, SPI_CS );
#else
ArduCAM myCAM( OV5642, SPI_CS );
#endif
// END GLOBALS


static void inline print_quartet(unsigned long i, File fd)
{ // Writes an uint32_t in Big Endian at current file position
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);
}

static void Video2SD()
{ // We don't enforce FPS: we just record and save frames as fast as possible
  // Then we compute the attained FPS and update the AVI header accordingly
  char str[8];
  uint16_t n;
  File outFile;
  byte buf[BUFFSIZE];
  static int i = 0;
  uint8_t temp = 0, temp_last = 0;
  unsigned long fileposition = 0;
  uint16_t frame_cnt = 0;
  uint16_t remnant = 0;
  uint32_t length = 0;
  uint32_t startms;
  uint32_t elapsedms;
  uint32_t uVideoLen = 0;
  bool is_header = false;

#ifndef DISABLE_SD
  // Create a avi file. Name should be unique-ish, but short

  digitalWrite(SD_CS, HIGH);
  randomSeed(analogRead(0) * millis());
  n = (random(2, 999));   // Don't use 1.avi: was the default in old code, we don't want to overwrite old recordings
  itoa(n, str, 10);
  strcat(str, ".avi");
  Serial.print("\nFile name will be "); Serial.println(str);

  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if (! outFile)
  {
    Serial.println(F("File open failed"));
    while (1);
    return;
  }
#endif

  //Write AVI Main Header
  // Some entries will be overwritten later
  for ( i = 0; i < AVIOFFSET; i++)
  {
    char ch = pgm_read_byte(&avi_header[i]);
    buf[i] = ch;
  }
#ifndef DISABLE_SD
  outFile.write(buf, AVIOFFSET);
#endif

  Serial.print(F("\nRecording "));
  Serial.print(TOTAL_FRAMES);
  Serial.println(F(" video frames: please wait...\n"));

  startms = millis();

  //Write video data, frame by frame
  for ( frame_cnt = 0; frame_cnt < TOTAL_FRAMES; frame_cnt++)
  {
#if defined (ESP8266)
    yield();
#endif
    temp_last = 0; temp = 0;
    //Capture a frame
    //Flush the FIFO
    myCAM.flush_fifo();
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    // Wait for frame ready
    while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
    length = myCAM.read_fifo_length();	// Length of FIFO buffer. In general, it contains more than 1 JPEG frame;
    // so we'll have to check JPEG markers to save a single JPEG frame
#if defined(SPI_HAS_TRANSACTION)
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
#endif

#ifndef DISABLE_SD
    // Write segment. We store 1 frame for each segment (video chunk)
    outFile.write("00dc");	// "start of video data chunk" (00 = data stream #0, d = video, c = "compressed")
    outFile.write(zero_buf, 4);	// Placeholder for actual JPEG frame size, to be overwritten later
#endif
    i = 0;
    jpeg_size = 0;

    // Deassert camera Chip Select to start SPI transfer
    myCAM.CS_LOW();
    // Set FIFO to burst read mode
    myCAM.set_fifo_burst();
    // Transfer data, a byte at a time
    while ( length-- )
    { // For every byte in the FIFO...
#if defined (ESP8266)
      yield();
#endif
      // We always need the last 2 bytes, to check for JPEG begin/end markers
      temp_last = temp; // Save current temp value
      temp =  SPI.transfer(0x00); // Overwrite temp with 1 byte from FIFO (0x00 is dummy byte for the slave: we are reading, the slave will ignore it)
#if defined(SPI_HAS_TRANSACTION)
      SPI.endTransaction();
#endif
      // a JPEG ends with the two bytes 0xFF, 0xD9
      if ( (temp == 0xD9) && (temp_last == 0xFF) ) //	End of the image
      {
        buf[i++] = temp;  // Add this last byte to the buffer
        myCAM.CS_HIGH();  // End of transfer: re-assert Slave Select
#ifndef DISABLE_SD
        // Write the buffer to file
        outFile.write(buf, i);
#endif
        is_header = false;	// We are at the last byte of the JPEG: sure is not the header :)
        jpeg_size += i;		// Update total jpeg size with this last buffer size
        i = 0;				// Reset byte counter (restart writing from the first element of the buffer)
      }
      if (is_header == true)  // Not at end of JPEG, yet
      {
        //Write image data to buffer if not full
        if (i < BUFFSIZE)
          buf[i++] = temp;
        else
        { // Buffer is full: transfer to file
          myCAM.CS_HIGH();  // End SPI transfer

#ifndef DISABLE_SD
          //Write BUFFSIZE bytes image data to file
          outFile.write(buf, BUFFSIZE);
#endif
          i = 0;	// Restart writing from the first element
          buf[i++] = temp;	// Save current byte as first in "new" buffer
          myCAM.CS_LOW();   // Re-enable SPI transfer
          myCAM.set_fifo_burst();	// Set FIFO to burst read mode
          jpeg_size += BUFFSIZE;
        }
      }
      else if ((temp == 0xD8) & (temp_last == 0xFF))
      { // A JPEG starts with the two bytes 0xFF, 0XD8; so here we are at the beginning of the JPEG
        is_header = true;
        buf[i++] = temp_last;	// Save the first two bytes (off-cycle)
        buf[i++] = temp;
      }
    }	// end loop over each byte in the FIFO: JPEG is complete

    // Padding
    remnant = jpeg_size & 0x00000001;	// Align to 16 bit: add 0 or 1 "0x00" bytes
#ifndef DISABLE_SD
    if (remnant > 0)
    {
      outFile.write(zero_buf, remnant); // see https://docs.microsoft.com/en-us/windows/desktop/directshow/avi-riff-file-reference
    }
#endif
    movi_size += jpeg_size;	// Update totals
    uVideoLen += jpeg_size;   // <- This is for statistics only

    // Now we have the real frame size in bytes. Time to overwrite the placeholder

#ifndef DISABLE_SD
    fileposition = outFile.position();  // Here, we are at end of chunk (after padding)
    outFile.seek(fileposition - jpeg_size - remnant - 4); // Here we are the the 4-bytes blank placeholder
    print_quartet(jpeg_size, outFile);    // Overwrite placeholder with actual frame size (without padding)
    outFile.seek(fileposition - jpeg_size - remnant + 2); // Here is the FOURCC "JFIF" (JPEG header)
    outFile.write("AVI1", 4);         // Overwrite "JFIF" (still images) with more appropriate "AVI1"

    // Return to end of JPEG, ready for next chunk
    outFile.seek(fileposition);
#endif
  }	// End cycle for all frames
  // END CAPTURE

  // Compute statistics
  elapsedms = millis() - startms;
  float fRealFPS = (1000.0f * (float)frame_cnt) / ((float)elapsedms);
  float fmicroseconds_per_frame = 1000000.0f / fRealFPS;
  uint8_t iAttainedFPS = round(fRealFPS); // Will overwrite AVI header placeholder
  uint32_t us_per_frame = round(fmicroseconds_per_frame); // Will overwrite AVI header placeholder

#ifndef DISABLE_SD
  //Modify the MJPEG header from the beginning of the file, overwriting various placeholders
  outFile.seek(4);
  print_quartet(movi_size + 12 * frame_cnt + 4, outFile); //    riff file size
  //overwrite hdrl
  //hdrl.avih.us_per_frame:
  outFile.seek(0x20);
  print_quartet(us_per_frame, outFile);
  unsigned long max_bytes_per_sec = movi_size * iAttainedFPS / frame_cnt; //hdrl.avih.max_bytes_per_sec
  outFile.seek(0x24);
  print_quartet(max_bytes_per_sec, outFile);
  //hdrl.avih.tot_frames
  outFile.seek(0x30);
  print_quartet(frame_cnt, outFile);
  outFile.seek(0x84);
  print_quartet((int)iAttainedFPS, outFile);
  //hdrl.strl.list_odml.frames
  outFile.seek(0xe0);
  print_quartet(frame_cnt, outFile);
  outFile.seek(0xe8);
  print_quartet(movi_size, outFile);// size again
  myCAM.CS_HIGH();
  //Close the file
  outFile.close();
#endif

  Serial.println(F("\n*** Video recorded and saved ***\n"));
  Serial.print(F("Recorded "));
  Serial.print(elapsedms / 1000);
  Serial.print(F("s in "));
  Serial.print(frame_cnt);
  Serial.print(F(" frames\nFile size is "));
  Serial.print(movi_size + 12 * frame_cnt + 4);
  Serial.print(F(" bytes\nActual FPS is "));
  Serial.print(fRealFPS, 2);
  Serial.print(F("\nMax data rate is "));
  Serial.print(max_bytes_per_sec);
  Serial.print(F(" byte/s\nFrame duration is "));
  Serial.print(us_per_frame);
  Serial.println(F(" us"));
  Serial.print(F("Average frame length is "));
  Serial.print(uVideoLen / TOTAL_FRAMES);
  Serial.println(F(" bytes"));
}

////////////
//
//  SETUP
//
////////////

void setup()
{
  uint8_t vid, pid;
  uint8_t temp;

  Wire.begin();
  Serial.begin(SERIAL_SPEED);
  while (!Serial);

  delay(200);
  Serial.println(F("ArduCAM Start!\n"));
  delay(5000);	// Gain time to start logic analyzer

#ifndef DISABLE_SD
  // set the SPI_CS as an output:
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
#ifdef FISHINO_UNO
  pinMode(SD_AUX, OUTPUT);
#endif
#endif

  delay(1000);

  // initialize SPI:
  SPI.begin();

#ifndef DISABLE_SD
  //Initialize SD Card
  while (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error!")); delay(1000);
  }
  Serial.println(F("SD Card detected."));
  delay(200);
#endif


  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(200);

  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI interface Error!"));
      delay(1000); continue;
    } else {
      Serial.println(F("SPI interface OK.")); break;
    }
  }
  delay(100);
#if defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS)
  while (1)
  {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    {
      Serial.println(F("Can't find OV2640 module!"));
      delay(1000); continue;
    }
    else {
      Serial.println(F("OV2640 detected.")); break;
    }
  }
#elif defined (OV3640_MINI_3MP)
  while (1) {
    //Check if the camera module type is OV3640
    myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
    if ((vid != 0x36) || (pid != 0x4C)) {
      Serial.println(F("Can't find OV3640 module!"));
      delay(1000); continue;
    } else {
      Serial.println(F("OV3640 detected.")); break;
    }
  }
#else
  while (1) {
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)) {
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000); continue;
    }
    else {
      Serial.println(F("OV5642 detected.")); break;
    }
  }
#endif
  delay(1000);
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
#if defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS)
  myCAM.OV2640_set_JPEG_size(FRAME_SIZE);
#elif defined (OV3640_MINI_3MP)
  myCAM.OV3640_set_JPEG_size(OV3640_320x240);
#else
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
#endif

 

  Video2SD(); 
  delay(30000);
}

void loop() {
  delay(500000);
}
