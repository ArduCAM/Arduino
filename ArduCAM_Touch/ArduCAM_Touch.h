/*
  ArduCAM_Touch.h - Arduino library support for ArduCAM Touch function
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

#ifndef ArduCAM_Touch_h
#define ArduCAM_Touch_h
#include "Arduino.h"
#define PORTRAIT	1
#define LANDSCAPE	0
#define PREC_LOW		1
#define PREC_MEDIUM		2
#define PREC_HI			3
#define PREC_EXTREME	4



#define TP_PRES_DOWN 0x80  			  
#define TP_CATH_PRES 0x40  		
#define CT_MAX_TOUCH  5  

#define ERROR 0.05

#define BOXSIZE 40
union data
		{
		   float val;
		   byte buf[4];
		};

class ArduCAM_Touch
{
	public:
		word	TP_X ,TP_Y;

				ArduCAM_Touch( byte tcs, byte irq);

		void	InitTouch(byte orientation = PORTRAIT);
		void	read();
		bool	dataAvailable();
		int		getX();
		int		getY();
		void	setPrecision(byte precision);
		
		data col_xfac;   
		data col_yfac;
		data col_xoff;   
		data col_yoff;
		
		
		
		
		uint8_t CMD_RDX=0X90;
        uint8_t CMD_RDY=0XD0;
		
		uint16_t width  = 320;
		uint16_t height = 240;
		uint16_t x[CT_MAX_TOUCH]; 		
		uint16_t y[CT_MAX_TOUCH];								
		uint8_t  sta;									
		float xfac;					
		float yfac;
		short xoff;
		short yoff;	   
		uint8_t touchtype =0;
		
	    void TP_Write_Byte(uint8_t num);	
		uint16_t TP_Read_AD(uint8_t CMD);	
		uint16_t TP_Read_XOY(uint8_t xy);   
		uint8_t TP_Read_XY(uint16_t *x,uint16_t *y);	
		uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y);	
		void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color);
		void TP_Draw_Big_Point(uint16_t x,uint16_t y);	
		void Load_Drow_Dialog(void);
	    void TP_fillRect(int x1, int y1, int x2, int y2, int color);
		void TP_clrSrc(void);
		void Drow_menu(void);
		void TP_Save_Adjdata(void);		
		uint8_t TP_Get_Adjdata(void);	
		int myabs(int x);
		void TP_Adjust(void);			 
		void TP_Adj_Info_Show(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t x3,uint16_t y3,uint16_t fac);//ÏÔÊ¾Ð£×¼ÐÅÏ¢
		uint8_t TP_Scan(uint8_t tp);		
		uint8_t TP_Init(void);			 
		
    private:
		byte	T_CS, T_IRQ;
		byte	orient;
		byte	prec;

		void	touch_WriteData(byte data);
		word	touch_ReadData();
};

#endif