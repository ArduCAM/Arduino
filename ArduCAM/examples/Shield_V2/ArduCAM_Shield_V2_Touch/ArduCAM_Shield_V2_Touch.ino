// ArduCAM demo (C)2017 Lee
// Web: http://www.arducam.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules, and can run on any Arduino platform.
// When you touch the 'Start Capture',It will preview the live video on LCD Screen.
//When you touch the ‘touch paint’，It will begin test the touch。
// This program requires the ArduCAM V4.0.0 (or later)、ArduCAM_Touch and UTFT4ArduCAM_SPI library and ARDUCAM_SHIELD_REVC
// and use Arduino IDE 1.6.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <ArduCAM_Touch.h>
#include <EEPROM.h>
#include <SPI.h>
#include <UTFT_SPI.h>
#include "memorysaver.h"
//This demo can only work on ARDUCAM_SHIELD_V2  platform.
#if !(defined (ARDUCAM_SHIELD_V2)&&(defined (OV5640_CAM) ||defined (OV5642_CAM)||defined (OV2640_CAM) || defined (OV3640_CAM)))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define  SPI_CS  10
int mode = 0;
boolean isShowFlag = false;
boolean TouchtestFlag = false;
uint32_t previous_time;
char currentPage = 0;
UTFT myGLCD(SPI_CS);
#if defined (OV2640_CAM)
  ArduCAM myCAM(OV2640, SPI_CS);
#elif defined (OV5640_CAM)
  ArduCAM myCAM(OV5640, SPI_CS);
#elif defined (OV3640_CAM)
  ArduCAM myCAM(OV3640, SPI_CS);
#elif defined (OV5642_CAM)
  ArduCAM myCAM(OV5642, SPI_CS);
#endif
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
ArduCAM_Touch  myTouch(8, 2);
void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp;
Wire.begin();
Serial.begin(921600);
Serial.println(F("ACK CMD ArduCAM Start!"));
// set the CS as an output:
pinMode(SPI_CS, OUTPUT);
digitalWrite(SPI_CS, HIGH);
// initialize SPI:
SPI.begin();
//Reset the CPLD
myCAM.write_reg(0x07, 0x80);
delay(100);
myCAM.write_reg(0x07, 0x00);
delay(100);
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55){
  Serial.println(F("SPI interface Error!"));
  delay(1000);continue;
  }else{
  Serial.println(F("SPI interface OK!"));break;
  }
}
//Change MCU mode
myCAM.set_mode(MCU2LCD_MODE);
myGLCD.InitLCD();
myTouch.InitTouch(); 
temp = EEPROM.read(16);
if (temp!= 0x0A)
  myTouch.TP_Adjust();
else{
  myTouch.TP_Get_Adjdata();
}
#if defined (OV2640_CAM)
  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println(F("Can't find OV2640 module!"));
      delay(1000);continue;
    }else{
      Serial.println(F("OV2640 detected."));break;
    }
  } 
#elif defined (OV3640_CAM)
while(1){
  //Check if the camera module type is OV3640
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV3640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV3640_CHIPID_LOW, &pid);
  if((vid != 0x36) || (pid != 0x4C)){
    Serial.println(F("Can't find OV3640 module!"));
    delay(1000);continue;
  }else{
    Serial.println(F("OV3640 detected."));break;
  } 
 } 
#elif defined (OV5640_CAM)
while(1){
  //Check if the camera module type is OV5642
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x40)){
    Serial.println(F("Can't find OV5640 module!"));
    delay(1000);continue;
  }else{
    Serial.println(F("OV5640 detected."));break;
  } 
}
#elif defined (OV5642_CAM)
while(1){
  //Check if the camera module type is OV5642
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x42)){
    Serial.println(F("Can't find OV5642 module!"));
    delay(1000);continue;
  } else{
    Serial.println(F("OV5642 detected.")); break;
  }
}
#endif
drawHomeScreen();
}
void loop() {
if (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
{
  previous_time = millis();
  while (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
  {
    if ((millis() - previous_time) > 30)
    {
      myGLCD.clrScr();
      myTouch.TP_Adjust();
      if(TouchtestFlag){
        myTouch.Drow_menu();
        myGLCD.setColor(100, 155, 203);
        myGLCD.fillRoundRect(10, 10, 60, 36);
        myGLCD.setColor(255, 255, 255);
        myGLCD.drawRoundRect(10, 10, 60, 36);
        myGLCD.setFont(BigFont);
        myGLCD.setBackColor(100, 155, 203);
        myGLCD.print("<-", 18, 15);     
      }else{
        drawHomeScreen();
      }
    }
  }     
}

// put your main code here, to run repeatedly:
if (currentPage == 0){
  myTouch.sta = myTouch.TP_Scan(0);
  if(myTouch.sta&TP_PRES_DOWN){
    if ((myTouch.x[0]>=35) && (myTouch.x[0]<=285) && (myTouch.y[0]>=90) && (myTouch.y[0]<=130)) {
      drawFrame(35, 90, 285, 130); // Custom Function -Highlighs the buttons when it's pressed
      currentPage = 1;           // Indicates that we are the first example
      isShowFlag = false;
      TouchtestFlag = 1;
      myGLCD.clrScr();             // Clears the screen
      myTouch.Drow_menu();
      myGLCD.setColor(100, 155, 203);
      myGLCD.fillRoundRect(10, 10, 60, 36);
      myGLCD.setColor(255, 255, 255);
      myGLCD.drawRoundRect(10, 10, 60, 36);
      myGLCD.setFont(BigFont);
      myGLCD.setBackColor(100, 155, 203);
      myGLCD.print("<-", 18, 15);     
    }
    if ((myTouch.x[0]>=35) && (myTouch.x[0]<=285) && (myTouch.y[0]>=190) && (myTouch.y[0]<=230)) {
      drawFrame(35, 190, 285, 230);
      currentPage = 2;
      myGLCD.clrScr();
      isShowFlag = true;
      TouchtestFlag  = 0;
      myGLCD.setColor(100, 155, 203);
      myGLCD.fillRoundRect(10, 10, 60, 36);
      myGLCD.setColor(255, 255, 255);
      myGLCD.drawRoundRect(10, 10, 60, 36);
      myGLCD.setFont(BigFont);
      myGLCD.setBackColor(100, 155, 203);
      myGLCD.print("<-", 18, 15);
    }    
  }    
}
if(currentPage == 1){
  if(TouchtestFlag){
    myTouch.sta = myTouch.TP_Scan(0);     
    if(myTouch.sta&TP_PRES_DOWN) {
      if ((myTouch.x[0]>=10) && (myTouch.x[0]<=60) &&(myTouch.y[0]>=10) && (myTouch.y[0]<=36)) {
        TouchtestFlag = false;
        drawFrame(10, 10, 60, 36);
        currentPage = 0;
        myGLCD.clrScr();
        drawHomeScreen();
      } 
      if(myTouch.x[0]>275&&(myTouch.y[0]>=0&&myTouch.y[0]<=BOXSIZE))
      { 
        myTouch.TP_fillRect(280, BOXSIZE*0, 320, BOXSIZE*1, VGA_RED);         
      }else if(myTouch.x[0]>275&&(myTouch.y[0]>=BOXSIZE&&myTouch.y[0]<=BOXSIZE*2)){
        myTouch.TP_fillRect(280, BOXSIZE*1, 320, BOXSIZE*2, VGA_YELLOW);
      }else if(myTouch.x[0]>275&&(myTouch.y[0]>=BOXSIZE*2&&myTouch.y[0]<=BOXSIZE*3)){
        myTouch.TP_fillRect(280, BOXSIZE*2, 320, BOXSIZE*3, VGA_GREEN);
      }else if(myTouch.x[0]>275&&(myTouch.y[0]>=BOXSIZE*3&&myTouch.y[0]<=BOXSIZE*4)){
        myTouch.TP_fillRect(280, BOXSIZE*3, 320, BOXSIZE*4, VGA_MAROON);
      }else if(myTouch.x[0]>275&&(myTouch.y[0]>=BOXSIZE*4&&myTouch.y[0]<=BOXSIZE*5)){
        myTouch.TP_fillRect(280, BOXSIZE*4, 320, BOXSIZE*5, VGA_BLUE);
      }else if(myTouch.x[0]>275&&(myTouch.y[0]>=BOXSIZE*5&&myTouch.y[0]<=BOXSIZE*6)){
        myGLCD.clrScr();
        myTouch.Drow_menu();
        myGLCD.setColor(100, 155, 203);
        myGLCD.fillRoundRect(10, 10, 60, 36);
        myGLCD.setColor(255, 255, 255);
        myGLCD.drawRoundRect(10, 10, 60, 36);
        myGLCD.setFont(BigFont);
        myGLCD.setBackColor(100, 155, 203);
        myGLCD.print("<-", 18, 15);
      } else{
        myTouch.TP_Draw_Big_Point(myTouch.x[0],myTouch.y[0]); 
      }  
    }
  }
}
if (currentPage == 2) {
  if(isShowFlag){
    myCAM.set_mode(MCU2LCD_MODE);
    myCAM.set_format(BMP);
    myCAM.InitCAM();
    #if !(defined (OV2640_CAM)||defined (OV3640_CAM))
    myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);    //VSYNC is active HIGH
    #endif
    while(1){
      myTouch.sta = myTouch.TP_Scan(0);
      if(myTouch.sta&TP_PRES_DOWN) {
        isShowFlag = false;
        currentPage = 0;
        isShowFlag=false;
        myCAM.set_mode(MCU2LCD_MODE);
        myGLCD.clrScr();
        drawHomeScreen();
        break;
      }   
      if(!myCAM.get_bit(ARDUCHIP_TRIG,VSYNC_MASK))              //New Frame is coming
      {     
        myCAM.set_mode(MCU2LCD_MODE);      //Switch to MCU
        myGLCD.resetXY();
        myCAM.set_mode(CAM2LCD_MODE);        //Switch to CAM
        while(!myCAM.get_bit(ARDUCHIP_TRIG,VSYNC_MASK));   //Wait for VSYNC is gone  
      }
    }
  }  
}
}

// Highlights the button when pressed
void drawFrame(int x1, int y1, int x2, int y2) {
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
  myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}

void drawHomeScreen() {
  // Title
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("ArduCAM", CENTER, 10); // Prints the string on the screen
  myGLCD.setColor(255, 255, 0); // Sets color to red
  myGLCD.drawLine(0,32,319,32); // Draws the red line CHANGED TO LIME GREEN
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets the font to small
  myGLCD.print("Shield V2 Demo", CENTER, 41); // Prints the string
  // Button - Rear View Camera
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (35, 90, 285, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (35, 90, 285, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("Touch Paint", CENTER, 102); // Prints the string
  // Show camera
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 190, 285, 230);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 190, 285, 230);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Start Capture", CENTER, 202);
}


