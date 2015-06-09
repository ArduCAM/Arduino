#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

// set pin 4 as the slave select for the digital pot:
const int CS = 4;

ArduCAM myCAM(OV2640,CS);

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

  if(temp != 0x55)
  {
  	Serial.println("SPI1 interface Error!");
  	//while(1);
  }
  
  myCAM.set_mode(MCU2LCD_MODE);

  //Check if the camera module type is OV2640
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);

  if((vid != 0x26) || (pid != 0x42))
  	Serial.println("Can't find OV2640 module!");
  else
  	Serial.println("OV2640 detected.");
  
  //Change to JPEG capture mode and initialize the OV5642 module	
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//enable low power
}

void loop() {
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;
  bool is_header = false;

  temp = Serial.read();
  switch(temp)
  {
    case 0:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_160x120);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 1:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_176x144);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 2:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_320x240);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 3:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_352x288);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 4:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_640x480);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 5:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_800x600);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 6:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_1024x768);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 7:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);
      myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      break;
    case 8:
      myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);
      myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
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
    Serial.println("CAM Capture Done!");
    
    uint32_t length = 0;
    length = myCAM.read_fifo_length();
    Serial.println(length);
    if(length >= 393216 )  // 384kb
    {
      Serial.println("Over size.");
      myCAM.clear_fifo_flag();
      return;
    }
    
    if(length == 0 )  //0 kb
    {
      Serial.println("Size is 0.");
      myCAM.clear_fifo_flag();
      return;
    }
    
    temp = 0;
    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
      temp_last = temp;
  	temp = myCAM.read_fifo();
  	if(is_header == true)
        {
  	  Serial.write(temp);
        }
        else if((temp == 0xD8) & (temp_last == 0xFF))
        {
          is_header = true;
          Serial.write(temp_last);
          Serial.write(temp);
        }
        delayMicroseconds(8);
    }
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
    
    is_header = false;
  }
}
