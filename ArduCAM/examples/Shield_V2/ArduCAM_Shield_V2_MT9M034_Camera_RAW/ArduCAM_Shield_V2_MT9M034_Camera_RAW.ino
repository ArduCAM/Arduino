// ArduCAM demo (C)2018 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules.
// This demo was made for MT9M034 sensor.
// It will turn the ArduCAM into a real digital camera with capture and playback functions.
// 1.Continuous shooting and store the image to Micro SD/TF card with RAW format.
//IF the FRAMES_NUM is 0X00, take one photos
//IF the FRAMES_NUM is 0X01, take two photos
//IF the FRAMES_NUM is 0X02, take three photos
//IF the FRAMES_NUM is 0X03, take four photos
//IF the FRAMES_NUM is 0X04, take five photos
//IF the FRAMES_NUM is 0X05, take six photos
//IF the FRAMES_NUM is 0X06, take seven photos
//IF the FRAMES_NUM is 0X07, continue shooting until the FIFO is full
// This program requires the ArduCAM V4.0.0 (or above) library and ArduCAM shield V2
// and use Arduino IDE 1.6.8 compiler or above

#define   FRAMES_NUM    0x07
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>

#if !(defined (MT9M034_CAM))
#error This demo can only support  MT9M034_CAM.
#endif
#if !(defined ARDUCAM_SHIELD_V2 && defined MT9M034_CAM )
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

#define SD_CS 9
const int SPI_CS = 10;

ArduCAM myCAM(MT9M034, SPI_CS);
uint32_t length = 0;
char str[8];
void setup()
{
  uint16_t vid;
  uint8_t temp = 0;
  Wire.begin();
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
  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  //Initialize SD Card
  while (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error")); delay(1000);
  }
  Serial.println(F("SD Card detected!"));
}
void loop()
{
  GrabImage(str);
}
void GrabImage(char* str)
{
  File outFile;
  char VL;
  byte buf[256];
  static int k = 0;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Start Capture"));
  //Polling the capture done flag
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  int PIC_CNT = length / 1233920; //1233920 = 1280*960
  Serial.println(PIC_CNT, DEC);
  Serial.println(F("Capture Done."));
  while (PIC_CNT--) {
    k = k + 1;
    itoa(k, str, 10);
    strcat(str, ".raw");      //Generate file name
    static int k = 0;
    int i, j = 0;
    outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
    if (! outFile)
    {
      Serial.println(F("File open error"));
      return;
    }
    Serial.println("Writting the image data in RAW format...");
    k = 0;
    //Read 1280x960 byte from FIFO
    //Save as RAW format
    for (i = 0; i < 1280; i++)
      for (j = 0; j < 964; j++)
      {
        VL = myCAM.read_fifo();
        buf[k++] = VL;
        //Write image data to bufer if not full
        if (k >= 256)
        {
          //Write 256 bytes image data to file from buffer
          outFile.write(buf, 256);
          k = 0;
        }
      }
    //Close the file
    outFile.close();
  }
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  Serial.println("Image save OK!");
  return;
}

