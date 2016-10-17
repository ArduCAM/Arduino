// ArduCAM demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules, and can run on any Arduino platform.
//
// This demo was made for ARDUCAM_SHIELD_REVC.
// It will run the ArduCAM 2MP/5MP as a real 2MP/5MP digital camera, provide both JPEG capture.
// The demo sketch will do the following tasks:
// 1. Set the sensor to JPEG mode.
// 2. Capture and buffer the image to FIFO every 5 seconds 
// 3. Store the image to Micro SD/TF card with JPEG format in sequential.
// 4. Resolution can be changed by myCAM.set_JPEG_size() function.
// This program requires the ArduCAM V4.0.0 (or later) library and ARDUCAM_SHIELD_REVC
// and use Arduino IDE 1.5.2 compiler or above
#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
//This demo can only work on ARDUCAM_SHIELD_REVC  platform.
#if !(defined (ARDUCAM_SHIELD_REVC)&&(defined (OV5642_CAM)||defined (OV2640_CAM)||defined (OV5640_CAM)))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#if defined(ESP8266)
 #define SD_CS 0 
 const int SPI_CS = 16;
#else 
 #define SD_CS 9 
 const int SPI_CS =10;
#endif

#if defined (OV2640_CAM)
ArduCAM myCAM( OV2640, SPI_CS );
#elif defined (OV5640_CAM)
ArduCAM myCAM( OV5640, SPI_CS );
#elif defined (OV5642_CAM)
ArduCAM myCAM( OV5642, SPI_CS );
#endif

void myCAMSaveToSDFile(){
  char str[8];
  byte buf[256];
  static int i = 0;
  static int k = 0;
  uint8_t temp = 0, temp_last = 0;
  File file;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println("star Capture");
 while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
 Serial.println("Capture Done!");  

 //Construct a file name
 k = k + 1;
 itoa(k, str, 10);
 strcat(str, ".jpg");
 //Open the new file
 file = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
 if(! file){
  Serial.println("open file faild");
  return;
 }
 i = 0;
 myCAM.CS_LOW();
 myCAM.set_fifo_burst();
 temp=SPI.transfer(0x00);
 //Read JPEG data from FIFO
 while ( (temp !=0xD9) | (temp_last !=0xFF)){
  temp_last = temp;
  temp = SPI.transfer(0x00);
  //Write image data to buffer if not full
  if( i < 256)
   buf[i++] = temp;
   else{
    //Write 256 bytes image data to file
    myCAM.CS_HIGH();
    file.write(buf ,256);
    i = 0;
    buf[i++] = temp;
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
   }
   delay(0);  
 }
 
 //Write the remain bytes in the buffer
 if(i > 0){
  myCAM.CS_HIGH();
  file.write(buf,i);
 }
 //Close the file
 file.close();
  Serial.println("CAM Save Done!");
}

void setup(){
  uint8_t vid = 0, pid = 0;
  uint8_t temp = 0;
  Wire.begin();
  Serial.begin(115200);
  Serial.println("ArduCAM Start!");

  //set the CS as an output:
  pinMode(SPI_CS,OUTPUT);

  // initialize SPI:
  SPI.begin();
  delay(1000);
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
   
  if (temp != 0x55){
    Serial.println("SPI1 interface Error!");
    //while(1);
  }
    //Initialize SD Card
  if(!SD.begin(SD_CS)){
    Serial.println("SD Card Error");
  }
  else
  Serial.println("SD Card detected!");
   
 #if defined (OV2640_CAM)
     //Check if the camera module type is OV2640
     myCAM.wrSensorReg8_8(0xff, 0x01);  
     myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
     myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
     if ((vid != 0x26) || (pid != 0x42))
      Serial.println("Can't find OV2640 module!");
     else
      Serial.println("OV2640 detected.");
 #elif defined (OV5640_CAM)
        //Check if the camera module type is OV5640
      myCAM.wrSensorReg16_8(0xff, 0x01);
      myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
      myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
       if((vid != 0x56) || (pid != 0x40))
       Serial.println("Can't find OV5640 module!");
       else
       Serial.println("OV5640 detected.");
  #elif defined (OV5642_CAM)
     //Check if the camera module type is OV5642
      myCAM.wrSensorReg16_8(0xff, 0x01);
      myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
      myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
       if((vid != 0x56) || (pid != 0x42))
       Serial.println("Can't find OV5642 module!");
       else
       Serial.println("OV5642 detected.");
  #endif
     myCAM.set_format(JPEG);
     myCAM.InitCAM();
 #if defined (OV2640_CAM)
   myCAM.OV2640_set_JPEG_size(OV2640_320x240);delay(1000);
 #elif defined (OV5640_CAM)
   myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
   myCAM.OV5640_set_JPEG_size(OV5640_320x240);delay(1000);
  #elif defined (OV5642_CAM)
   myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
   myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
 #endif
  delay(1000);
}

void loop(){
  myCAMSaveToSDFile();
  delay(5000);
   
  
  
}


