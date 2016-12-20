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
#include <ArduCAM.h>
#include <EEPROM.h>
#include <UTFT_SPI.h>
#include <SPI.h>

// Declare which fonts we will be using
extern uint8_t SmallFont[];
// Declare the TFT CS
const int SPI_CS =10;
UTFT myGLCD(SPI_CS);
//Initialize the touch  CS INT
ArduCAM_Touch  myTouch(8, 2);
ArduCAM myCAM(OV5642, SPI_CS); 
uint32_t previous_time;
long pre_color =0;
void rtp_test(void)
{  
  while(1)
  {
    myTouch.sta = myTouch.TP_Scan(0);      
    if(myTouch.sta&TP_PRES_DOWN)     
    { 
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
            pre_color = myGLCD.getColor();
            Serial.println(pre_color,HEX);
             myGLCD.clrScr();
             myTouch.Drow_menu();
             myGLCD.setColor(pre_color);
      } 
      else  myTouch.TP_Draw_Big_Point(myTouch.x[0],myTouch.y[0]); 
      
     }  
     if (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
    {
      previous_time = millis();
        while (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
        {
          if ((millis() - previous_time) > 30)
          {
            myGLCD.clrScr();
            myTouch.TP_Adjust();
             myTouch.Drow_menu();
          }
        }     
    }
  }
}
void setup()
{
  uint8_t temp = 0;
  Serial.begin(115200);
	// set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myTouch.InitTouch(); 
  temp = EEPROM.read(16);
 if (temp!= 0x0A){
   myTouch.TP_Adjust();
  myTouch.Drow_menu();
 }
 
  else{
   myGLCD.clrScr();
   myTouch.TP_Get_Adjdata();
   myTouch.Drow_menu();
  }
}

void loop()
{
  rtp_test();
}
