/*
  ArduCAM_Touch.cpp - Arduino library support for ArduCAM Touch function
  Copyright (C)2010 www.ArduCAM.com. All right reserved
  
  Basic functionality of this library are based on the demo-code provided by
  ArduCAM. You can find the latest version of the library at
  http://www.ArduCAM.com

  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version:   1.0  - Dec  21 2012  - initial release

*/

#define PixSizeX	13.78
#define PixOffsX	411
#define PixSizeY	11.01
#define PixOffsY	378

#include "Arduino.h"
#include "ArduCAM_Touch.h"
#include <avr/pgmspace.h>
#include <SPI.h>
#include "UTFT_SPI.h"
#include "EEPROM.h"
#define LCD_CS 10
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
UTFT TP_TFT(LCD_CS);



ArduCAM_Touch::ArduCAM_Touch(byte tcs,byte irq)
{
		T_IRQ = irq;
		T_CS  = tcs;
    SPI.begin();
}

void ArduCAM_Touch::InitTouch(byte orientation)
{
	orient = orientation;
	pinMode(T_CS,   OUTPUT);
	pinMode(T_IRQ,  INPUT);
	digitalWrite(T_CS,  HIGH);
}

void ArduCAM_Touch::touch_WriteData(byte data)
{
	SPI.transfer(data);
}

word ArduCAM_Touch::touch_ReadData()
{
	byte nop = 0;
	word data;
	data = SPI.transfer(nop);
	data = data << 8;
	data |= SPI.transfer(nop);
	data = data >> 4;
	return(data);
}

void ArduCAM_Touch::read()
{
	unsigned long tx=0;
	unsigned long ty=0;

	digitalWrite(T_CS,LOW);                    

	for (int i=0; i<prec; i++)
	{
		touch_WriteData(0x90);        
		ty+=touch_ReadData();

		touch_WriteData(0xD0);      
		tx+=touch_ReadData();
		
	}

	digitalWrite(T_CS,HIGH);

	TP_X=tx/prec;
	TP_Y=ty/prec;
	
}

bool ArduCAM_Touch::dataAvailable()
{
  bool avail;
  avail = !digitalRead(T_IRQ);
  return avail;
}

int ArduCAM_Touch::getX()
{
	int value;

	if (orient == PORTRAIT)
	{
		if (PixSizeX>=0)
		{
			value = 240-((TP_X-PixOffsX)/PixSizeX);
		}
		else
		{
			value = (TP_X-PixOffsX)/-(PixSizeX);
		}
	}
	else
	{
		if (PixSizeY<0)
			value = 400-((TP_Y-PixOffsY)/-PixSizeY);
		else
			value = ((TP_Y-PixOffsY)/PixSizeY);
	}

	if (value < 0)
		value = 0;
	return value;
}

int ArduCAM_Touch::getY()
{
	int value;

	if (orient == PORTRAIT)
	{
		if (PixSizeY<0)
			value = ((TP_Y-PixOffsY)/-PixSizeY);
		else
			value = 320-((TP_Y-PixOffsY)/PixSizeY);
	}
	else
	{
		if (PixSizeX>=0)
		{
			value = 240-((TP_X-PixOffsX)/PixSizeX);
		}
		else
		{
			value = (TP_X-PixOffsX)/-(PixSizeX);
		}
	}

	if (value < 0)
		value = 0;
	return value;  
}

void ArduCAM_Touch::setPrecision(byte precision)
{
	switch (precision)
	{
		case PREC_LOW:
			prec=1;
			break;
		case PREC_MEDIUM:
			prec=10;
			break;
		case PREC_HI:
			prec=25;
			break;
		case PREC_EXTREME:
			prec=100;
			break;
		default:
			prec=10;
			break;
	}
}



void ArduCAM_Touch::TP_Write_Byte(uint8_t num)
{
	SPI.transfer(num);
}


uint16_t ArduCAM_Touch::TP_Read_AD(uint8_t CMD)
{
    uint16_t Num = 0;
	digitalWrite(T_CS,LOW);                  	
	TP_Write_Byte(CMD);
	delayMicroseconds(1);
	Num=touch_ReadData();		
	digitalWrite(T_CS,HIGH);  
	return(Num); 	
}	

#define READ_TIMES 10 	
#define LOST_VAL 1	  
uint16_t ArduCAM_Touch::TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
uint8_t ArduCAM_Touch::TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp; 	
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	*x=xtemp;
	*y=ytemp;
	return 1;
}
#define ERR_RANGE 50 
uint8_t ArduCAM_Touch::TP_Read_XY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else 
		return 0;	  
} 

void ArduCAM_Touch::TP_fillRect(int x1, int y1, int x2, int y2, int color)
{
	TP_TFT.setColor(color);
	TP_TFT.fillRect(x1,y1,x2,y2);
	
}

void ArduCAM_Touch::TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color)
{
	TP_TFT.setColor(color);
	TP_TFT.drawLine(x-12,y,x+13,y);
	TP_TFT.drawLine(x,y-12,x,y+13);
	TP_TFT.drawPixel(x+1,y+1);
	TP_TFT.drawPixel(x-1,y+1);
	TP_TFT.drawPixel(x+1,y-1);
	TP_TFT.drawPixel(x-1,y-1);
	TP_TFT.drawCircle(x,y,6);
}

void ArduCAM_Touch::TP_Draw_Big_Point(uint16_t x,uint16_t y)
{	    
    TP_TFT.drawPixel(x,y); 
	TP_TFT.drawPixel(x+1,y);
	TP_TFT.drawPixel(x,y+1);
	TP_TFT.drawPixel(x+1,y+1);	 	  	
}	

void ArduCAM_Touch::Load_Drow_Dialog(void)
{ 
  	TP_TFT.setFont(SmallFont);
 	TP_TFT.setColor(VGA_WHITE); 
	TP_TFT.print("RST", RIGHT, 20);
    TP_TFT.setColor(VGA_YELLOW);
    TP_TFT.setFont(BigFont);	
}

void ArduCAM_Touch::Drow_menu(void)
{   	
 TP_fillRect(280, BOXSIZE*0, 320, BOXSIZE*1, VGA_RED);
 TP_fillRect(280, BOXSIZE*1, 320, BOXSIZE*2, VGA_YELLOW);
 TP_fillRect(280, BOXSIZE*2, 320, BOXSIZE*3, VGA_GREEN);
 TP_fillRect(280, BOXSIZE*3, 320, BOXSIZE*4, VGA_MAROON);
 TP_fillRect(280, BOXSIZE*4, 320, BOXSIZE*5, VGA_BLUE);
 TP_TFT.setFont(SmallFont);
 TP_TFT.setColor(VGA_WHITE); 
 TP_TFT.print("RST", RIGHT, 220);
}
uint8_t ArduCAM_Touch::TP_Scan(uint8_t tp)
{			   
	if(dataAvailable())
	{
		delay(10);
		if(dataAvailable())
		{
			if(tp){TP_Read_XY2(&x[0],&y[0]);
			}
			 else if(TP_Read_XY2(&x[0],&y[0]))
			{
				x[0]=xfac*x[0]+xoff;
				y[0]=yfac*y[0]+yoff;  
			} 
			if((sta&TP_PRES_DOWN)==0)
			{		 
				sta=TP_PRES_DOWN|TP_CATH_PRES;
				x[4]=x[0];
				y[4]=y[0];  	   			 
			}
	    }		
	}else
	{
		if(sta&TP_PRES_DOWN)
		{
			sta&=~(1<<7);
		}else
		{
			x[4]=0;
			y[4]=0;
			x[0]=0xffff;
			y[0]=0xffff;
		}	    
	}
	return sta;
}	
int ArduCAM_Touch::myabs(int x)
{
	if(x<0)
		x=-x;
	return x;
}
void ArduCAM_Touch::TP_Adjust(void)
{
	uint16_t pos_temp[4][2];
	uint8_t  cnt=0;	
	uint16_t d1,d2;
	uint32_t tem1,tem2;
	double fac; 	
	uint16_t outtime = 0;
 	cnt=0;
    TP_TFT.setFont(BigFont);	
	TP_TFT.setColor(VGA_BLUE);
	TP_Drow_Touch_Point(20,20,VGA_YELLOW);
	sta=0;
	xfac=0;
	TP_TFT.print("Please adjust!", 60, 120);
	while(1)
	{
		sta = TP_Scan(1);
		if((sta&0xc0)==TP_CATH_PRES)
		{		  
			outtime=0;		
			sta&=~(1<<6);
			TP_TFT.print("x=",40,40);
			TP_TFT.printNumI(x[0],60,40);	
            TP_TFT.print("y=",40,60);
			TP_TFT.printNumI(y[0],60,60);			
			pos_temp[cnt][0]=x[0];
			pos_temp[cnt][1]=y[0];
			
			
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,VGA_BLACK);				
					TP_Drow_Touch_Point(width-20,20,VGA_YELLOW);	
					break;
				case 2:
 					TP_Drow_Touch_Point(width-20,20,VGA_BLACK);	
					TP_Drow_Touch_Point(20,height-20,VGA_YELLOW);	
					break;
				case 3:
 					TP_Drow_Touch_Point(20,height-20,VGA_BLACK);			
 					TP_Drow_Touch_Point(width-20,height-20,VGA_YELLOW);
					break;
				case 4:	 
					tem1=myabs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=myabs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					
					d1=sqrt(tem1+tem2);
					

					tem1=myabs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=myabs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);
					
					fac=(float)d1/d2;
				    Serial.println(fac);
					if(fac<(1-ERROR)||fac>(1+ERROR)||d1==0||d2==0)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(width-20,height-20,VGA_BLACK);	
   	 					TP_Drow_Touch_Point(20,20,VGA_YELLOW);							
 						continue;
					}
					tem1=myabs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=myabs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);
					
					tem1=myabs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=myabs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					
					d2=sqrt(tem1+tem2);
		            fac=(float)d1/d2;
				    Serial.println(fac);
					if(fac<(1-ERROR)||fac>(1+ERROR))
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(width-20,height-20,VGA_BLACK);	
   	 					TP_Drow_Touch_Point(20,20,VGA_YELLOW);							
 						continue;
					}
								   
				
					tem1=myabs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=myabs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
	 				d1=sqrt(tem1+tem2);
	               
					tem1=myabs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=myabs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);
					
					fac=(float)d1/d2;
					Serial.println(fac);
					if(fac<(1-ERROR)||fac>(1+ERROR))
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(width-20,height-20,VGA_BLACK);	
   	 					TP_Drow_Touch_Point(20,20,VGA_YELLOW);								
 						continue;
					}
					
					xfac=-(float)(width-40)/(pos_temp[0][0]-pos_temp[1][0]);//xfac
              			
					xoff=(width-xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//xoff
						  
					yfac=-(float)(height-40)/(pos_temp[0][1]-pos_temp[2][1]);//yfac
					
					yoff=(height-yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//yoff
					
					TP_Save_Adjdata();
					EEPROM.write(16, 0x0a);   
												
					if(myabs(xfac)>2||myabs(yfac)>2)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(width-20,height-20,VGA_BLACK);	
   	 					TP_Drow_Touch_Point(20,20,VGA_YELLOW);					
						TP_TFT.print("TP Need readjust!", LEFT, 40);
						touchtype=!touchtype;
						if(touchtype)
						{
						  CMD_RDX=0X90;
						  CMD_RDY=0XD0;	 
						}else				   
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					TP_TFT.setColor(VGA_BLACK);
					TP_TFT.print("Please adjust!", 60, 120);
					TP_TFT.setColor(VGA_YELLOW);
					TP_TFT.print("Adjust OK!", 60, 120);
 					TP_TFT.clrScr();
					return;				 
			}
		}
		delay(10);
		outtime++;
		if(outtime>1000)
		{
		  TP_Get_Adjdata();
		  TP_TFT.setColor(VGA_BLACK);
		  TP_TFT.print("Please adjust!", 60, 120);
		  TP_TFT.setColor(VGA_YELLOW);
		  TP_TFT.print("Adjust OK!", 60, 120);
		  TP_TFT.clrScr();
		  break;
	 	} 
 	}
}

void ArduCAM_Touch::TP_Save_Adjdata(void){
	uint8_t i = 0;
	col_xfac.val = xfac;
	for(i=0;i<4;i++)
    EEPROM.write(i, col_xfac.buf[i]);   
	col_yfac.val = yfac;
	for(i=0;i<4;i++)
    EEPROM.write(i+4, col_yfac.buf[i]);  
	col_xoff.val = xoff;
	for(i=0;i<4;i++)
    EEPROM.write(i+8, col_xoff.buf[i]); 
    col_yoff.val = yoff;
    for(i=0;i<4;i++)
    EEPROM.write(i+12, col_yoff.buf[i]);	
}
uint8_t ArduCAM_Touch::TP_Get_Adjdata(void)
{
	uint8_t i = 0;
	for(i=0;i<4;i++)
    col_xfac.buf[i] = EEPROM.read(i);
    xfac = col_xfac.val;
	for(i=0;i<4;i++)
    col_yfac.buf[i] = EEPROM.read(i+4);
    yfac = col_yfac.val;
	for(i=0;i<4;i++)
    col_xoff.buf[i] = EEPROM.read(i+8);
    xoff = col_xoff.val; 
    for(i=0;i<4;i++)
    col_yoff.buf[i] = EEPROM.read(i+12);
    yoff = col_yoff.val;
	return 1;
}


