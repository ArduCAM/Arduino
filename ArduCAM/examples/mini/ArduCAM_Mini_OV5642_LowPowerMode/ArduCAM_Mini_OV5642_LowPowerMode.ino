#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

// set pin 4 as the slave select for the digital pot:
const int CS = 4;

ArduCAM myCAM(OV5642,CS);

void setup() {
  uint8_t vid,pid;
  uint8_t temp;
#if defined (__AVR__)
  Wire.begin(); 
#endif
#if defined(__arm__)
  Wire1.begin();
#endif 
  Serial.begin(921600);
  Serial.println("ArduCAM Start!"); 

  // set the SPI_CS as an output:
  pinMode(CS, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
    Serial.println("SPI1 interface Error!");
    //while(1);
  }

  //Check if the camera module type is OV5642
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x42))
    Serial.println("Can't find OV5642 module!");
  else
    Serial.println("OV5642 detected.");
  
  //Change to JPEG capture mode and initialize the OV5642 module	
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//enable low power
}

void loop() {
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;

  temp = Serial.read();
  switch(temp)
  {
    case 0:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV5642_set_JPEG_size(OV5642_320x240);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 1:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV5642_set_JPEG_size(OV5642_640x480);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 2:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV5642_set_JPEG_size(OV5642_1280x720);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 3:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV5642_set_JPEG_size(OV5642_1920x1080);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 4:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV5642_set_JPEG_size(OV5642_2048x1563);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 5:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 0x10:
      start_capture = 1;  
      Serial.println("CAM1 start single shot.");
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//disable low power
      
      delay(800);
      break;
    default:
      break;
  }
  if(start_capture == 1)
  {
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();	 
    //Start capture
    myCAM.start_capture();
    start_capture = 0;
  }
  if(myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
    Serial.println("CAM1 Capture Done!");
    temp = 0;
    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
      temp_last = temp;
      temp = myCAM.read_fifo();
      Serial.write(temp);
      delayMicroseconds(10);
    }
    //Clear the capture done flag 
    myCAM.clear_fifo_flag(); 
  }
}
