// ArduCAM Shield_V2 TOUCH demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use the touch function.
// This demo was made for Shield_V2 and TFT.
// The demo sketch will do the following tasks:
// 1. After power on, please  calibrate the screen;
// 2.Touch screen prompts four calibration point accurately;
// 3. After successful calibration, the screen prompt Adjust OK;
// 4. Then you can touch the screen to draw, touch the RST to clear the screen.
#include <ArduCAM_Touch.h>
#include <UTFT_SPI.h>
#include <SPI.h>
// Declare which fonts we will be using
extern uint8_t SmallFont[];
// Declare the TFT CS
const int SPI_CS =10;
UTFT myGLCD(SPI_CS);
//Initialize the touch  CS INT
ArduCAM_Touch  myTouch(8, 2);
void rtp_test(void)
{  
  while(1)
  {
    myTouch.sta = myTouch.TP_Scan(0);      
    if(myTouch.sta&TP_PRES_DOWN)     
    { 
      if(myTouch.x[0]<myTouch.width&&myTouch.y[0]<myTouch.height)
      { 
        if(myTouch.x[0]>(myTouch.width-30)&&myTouch.y[0]<20) {
            myGLCD.clrScr(); myTouch.Load_Drow_Dialog();
          }
        else  myTouch.TP_Draw_Big_Point(myTouch.x[0],myTouch.y[0],VGA_YELLOW);              
      }
    }else delay(20);      
   
  }
}
void setup()
{
  Serial.begin(115200);
	// set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM); 
  myTouch.TP_Adjust();
  myTouch.Load_Drow_Dialog();
}

void loop()
{
  rtp_test();
}
