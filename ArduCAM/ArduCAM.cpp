/*
  ArduCAM.cpp - Arduino library support for CMOS Image Sensor
  Copyright (C)2011-2015 ArduCAM.com. All right reserved

  Basic functionality of this library are based on the demo-code provided by
  ArduCAM.com. You can find the latest version of the library at
  http://www.ArduCAM.com

  Now supported controllers:
    - OV7670
    - MT9D111
    - OV7675
    - OV2640
    - OV3640
    - OV5642
    - OV7660
    - OV7725
    - MT9M112
    - MT9V111
    - OV5640
    - MT9M001
    - MT9T112
    - MT9D112

  We will add support for many other sensors in next release.

  Supported MCU platform
    - Theoretically support all Arduino families
      - Arduino UNO R3      (Tested)
      - Arduino MEGA2560 R3   (Tested)
      - Arduino Leonardo R3   (Tested)
      - Arduino Nano      (Tested)
      - Arduino DUE       (Tested)
      - Arduino Yun       (Tested)
      - Raspberry Pi      (Tested)
      - ESP8266-12        (Tested)

  If you make any modifications or improvements to the code, I would appreciate
  that you share the code with me so that I might include it in the next release.
  I can be contacted through http://www.ArduCAM.com

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
*/

/*------------------------------------
  Revision History:
  2012/09/20  V1.0.0  by Lee  first release
  2012/10/23  V1.0.1  by Lee  Resolved some timing issue for the Read/Write Register
  2012/11/29  V1.1.0  by Lee  Add support for MT9D111 sensor
  2012/12/13  V1.2.0  by Lee  Add support for OV7675 sensor
  2012/12/28  V1.3.0  by Lee  Add support for OV2640,OV3640,OV5642 sensors
  2013/02/26  V2.0.0  by Lee  New Rev.B shield hardware, add support for FIFO control
                              and support Mega1280/2560 boards
  2013/05/28  V2.1.0  by Lee  Add support all drawing functions derived from UTFT library
  2013/08/24  V3.0.0  by Lee  Support ArudCAM shield Rev.C hardware, features SPI interface and low power mode.
                Support almost all series of Arduino boards including DUE.
  2014/02/06  V3.0.1  by Lee  Minor change to the library, fixed some bugs, add self test code to the sketches for easy debugging.
  2014/03/09  V3.1.0  by Lee  Add the more impressive example sketches.
                Optimise the OV5642 settings, improve image quality.
                Add live preview before JPEG capture.
                Add play back photos one by one after BMP capture.
  2014/05/01  V3.1.1  by Lee  Minor changes to add support Arduino IDE for linux distributions.
  2014/09/30  V3.2.0  by Lee  Improvement on OV5642 camera dirver.
  2014/10/06  V3.3.0  by Lee  Add OV7660,OV7725 camera support.
  2015/02/27  V3.4.0  by Lee  Add the support for Arduino Yun board, update the latest UTFT library for ArduCAM.
  2015/06/09  V3.4.1  by Lee  Minor changes and add some comments
  2015/06/19  V3.4.2  by Lee  Add support for MT9M112 camera.
  2015/06/20  V3.4.3  by Lee  Add support for MT9V111 camera.
  2015/06/22  V3.4.4  by Lee  Add support for OV5640 camera.
  2015/06/22  V3.4.5  by Lee  Add support for MT9M001 camera.
  2015/08/05  V3.4.6  by Lee  Add support for MT9T112 camera.
  2015/08/08  V3.4.7  by Lee  Add support for MT9D112 camera.
  2015/09/20  V3.4.8  by Lee  Add support for ESP8266 processor.
  2016/02/03  V3.4.9  by Lee  Add support for Arduino ZERO board.
  2016/06/07  V3.5.0  by Lee  Add support for OV5642_CAM_BIT_ROTATION_FIXED.
  2016/06/14  V3.5.1  by Lee  Add support for ArduCAM-Mini-5MP-Plus OV5640_CAM.
  2016/09/29  V3.5.2  by Lee  Optimize the OV5642 register settings
	2016/10/05	V4.0.0	by Lee	Add support for second generation hardware platforms like ArduCAM shield V2, ArduCAM-Mini-5MP-Plus(OV5642/OV5640).	  
  2016/10/28  V4.0.1  by Lee	Add support for Raspberry Pi
  --------------------------------------*/
#include "memorysaver.h"
#if defined ( RASPBERRY_PI )
	#include <string.h>
	#include <time.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>
	#include <unistd.h>
	#include <wiringPiI2C.h>
	#include <wiringPi.h>
	#include "ArduCAM.h"
	#include "arducam_arch_raspberrypi.h"
#else
	#include "Arduino.h"
	#include "ArduCAM.h"
	#include <Wire.h>
	#include <SPI.h>
	#include "HardwareSerial.h"
	#if defined(__SAM3X8E__)
	#define Wire Wire1
	#endif
#endif


ArduCAM::ArduCAM()
{
  sensor_model = OV7670;
  sensor_addr = 0x42;
}
ArduCAM::ArduCAM(byte model ,int CS)
{
	#if defined (RASPBERRY_PI)
	  
		if(CS>=0)
		{
			B_CS = CS;
		}
	#else
		#if defined(ESP8266)
		  B_CS = CS;
		#else
		  P_CS  = portOutputRegister(digitalPinToPort(CS));
		  B_CS  = digitalPinToBitMask(CS);
		#endif
	#endif
 #if defined (RASPBERRY_PI)
   pinMode(CS, OUTPUT);
 #else
	  pinMode(CS, OUTPUT);
      sbi(P_CS, B_CS);
	#endif
	sensor_model = model;
	switch (sensor_model)
	{
		case OV7660:
		case OV7670:
		case OV7675:
		case OV7725:
		#if defined (RASPBERRY_PI)
				sensor_addr = 0x21;
		#else
		  	sensor_addr = 0x42;
	    #endif		
		break;
		case MT9D111_A: //Standard MT9D111 module
      sensor_addr = 0xba;
    break;
    case MT9D111_B: //Flex MT9D111 AF module
      sensor_addr = 0x90;
    break;
    case MT9M112:
    	#if defined (RASPBERRY_PI)
    		sensor_addr = 0x5d;
    	#else
      	sensor_addr = 0x90;
      #endif
    break;
    case MT9M001:
      sensor_addr = 0xba;
    break;
    case OV3640:
    case OV5640:
    case OV5642:
    case MT9T112:
    case MT9D112:
    	#if defined (RASPBERRY_PI)
    		sensor_addr = 0x3c;
    	#else
      	sensor_addr = 0x78;
       #endif
   break;
    case OV2640:
    case OV9650:
    case OV9655:
    	#if defined (RASPBERRY_PI)
    		sensor_addr = 0x30;
    	#else
      	sensor_addr = 0x60;
      #endif
    break;
		default:
			#if defined (RASPBERRY_PI)
		 		sensor_addr = 0x21;
		 	#else
		 		sensor_addr = 0x42;
      #endif
		break;
	}	
	#if defined (RASPBERRY_PI)
		// initialize i2c:
	if (!arducam_i2c_init(sensor_addr)) {
		printf("ERROR: I2C init failed\n");
	}
	#endif
}

void ArduCAM::InitCAM()
{
 
  switch (sensor_model)
  {
    case OV7660:
      {
#if defined OV7660_CAM
        wrSensorReg8_8(0x12, 0x80);
        delay(100);
        wrSensorRegs8_8(OV7660_QVGA);
#endif
        break;
      }
    case OV7725:
      {
#if defined OV7725_CAM
       byte reg_val;
        wrSensorReg8_8(0x12, 0x80);
        delay(100);
        wrSensorRegs8_8(OV7725_QVGA);
        rdSensorReg8_8(0x15, &reg_val);
        wrSensorReg8_8(0x15, (reg_val | 0x02));
#endif
        break;
      }
    case OV7670:
      {
#if defined OV7670_CAM
        wrSensorReg8_8(0x12, 0x80);
        delay(100);
        wrSensorRegs8_8(OV7670_QVGA);
#endif
        break;
      }
    case OV7675:
      {
#if defined OV7675_CAM
        wrSensorReg8_8(0x12, 0x80);
        delay(100);
        wrSensorRegs8_8(OV7675_QVGA);

#endif
        break;
      }
    case MT9D111_A:
    case MT9D111_B:
      {
#if defined MT9D111_CAM
        wrSensorRegs8_16(MT9D111_QVGA_30fps);
        delay(1000);
        wrSensorReg8_16(0x97, 0x0020);
        wrSensorReg8_16(0xf0, 0x00);
        wrSensorReg8_16(0x21, 0x8403); //Mirror Column
        wrSensorReg8_16(0xC6, 0xA103);//SEQ_CMD
        wrSensorReg8_16(0xC8, 0x0005); //SEQ_CMD
#endif
        break;

      }
    case OV5642:
      {
#if ( defined(OV5642_CAM) || defined(OV5642_MINI_5MP) || defined(OV5642_MINI_5MP_BIT_ROTATION_FIXED) || defined(OV5642_MINI_5MP_PLUS) )
        wrSensorReg16_8(0x3008, 0x80);
        wrSensorRegs16_8(OV5642_QVGA_Preview);
        #if defined (RASPBERRY_PI) 
			  arducam_delay_ms(100);
				#else
        delay(100);
        #endif
        if (m_fmt == JPEG)
        {
        	#if defined (RASPBERRY_PI) 
				  arducam_delay_ms(100);
					#else
	        delay(100);
	        #endif
          wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA);
          wrSensorRegs16_8(ov5642_320x240);
          #if defined (RASPBERRY_PI) 
			  arducam_delay_ms(100);
				#else
        delay(100);
        #endif
          wrSensorReg16_8(0x3818, 0xa8);
          wrSensorReg16_8(0x3621, 0x10);
          wrSensorReg16_8(0x3801, 0xb0);
          #if (defined(OV5642_MINI_5MP_PLUS) || (defined ARDUCAM_SHIELD_V2))
          wrSensorReg16_8(0x4407, 0x04);
          #else
          wrSensorReg16_8(0x4407, 0x0C);
          #endif
        }
        else
        {
        	byte reg_val;
          wrSensorReg16_8(0x4740, 0x21);
          wrSensorReg16_8(0x501e, 0x2a);
          wrSensorReg16_8(0x5002, 0xf8);
          wrSensorReg16_8(0x501f, 0x01);
          wrSensorReg16_8(0x4300, 0x61);
          rdSensorReg16_8(0x3818, &reg_val);
          wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff);
          rdSensorReg16_8(0x3621, &reg_val);
          wrSensorReg16_8(0x3621, reg_val & 0xdf);
        }

#endif
        break;
      }
    case OV5640:
      {
#if ( defined(OV5640_CAM) || defined(OV5640_MINI_5MP_PLUS) )
        delay(100);
        if (m_fmt == JPEG)
        {
          wrSensorReg16_8(0x3103, 0x11);
          wrSensorReg16_8(0x3008, 0x82);
          delay(100);
          wrSensorRegs16_8(OV5640YUV_Sensor_Dvp_Init);
          delay(500);
          wrSensorRegs16_8(OV5640_JPEG_QSXGA);
          wrSensorRegs16_8(OV5640_QSXGA2QVGA);
          #if (defined(OV5640_MINI_5MP_PLUS) || (defined ARDUCAM_SHIELD_V2))
          wrSensorReg16_8(0x4407, 0x04);
          #else
          wrSensorReg16_8(0x4407, 0x0C);
          #endif
        }
        else
        {
          wrSensorReg16_8(0x3103, 0x11);
          wrSensorReg16_8(0x3008, 0x82);
          delay(500);
          wrSensorRegs16_8(OV5640YUV_Sensor_Dvp_Init);
          wrSensorRegs16_8(OV5640_RGB_QVGA);
        }

#endif
        break;
      }
    case OV3640:
      {
#if defined OV3640_CAM
        rtn = wrSensorRegs16_8(OV3640_QVGA);
#endif
        break;
      }
    case OV2640:
      {
#if (defined(OV2640_CAM) || defined(OV2640_MINI_2MP))
        wrSensorReg8_8(0xff, 0x01);
        wrSensorReg8_8(0x12, 0x80);
        delay(100);
        if (m_fmt == JPEG)
        {
          wrSensorRegs8_8(OV2640_JPEG_INIT);
          wrSensorRegs8_8(OV2640_YUV422);
          wrSensorRegs8_8(OV2640_JPEG);
          wrSensorReg8_8(0xff, 0x01);
          wrSensorReg8_8(0x15, 0x00);
          wrSensorRegs8_8(OV2640_320x240_JPEG);
          //wrSensorReg8_8(0xff, 0x00);
          //wrSensorReg8_8(0x44, 0x32);
        }
        else
        {
          wrSensorRegs8_8(OV2640_QVGA);
        }
#endif
        break;
      }
    case OV9655:
      {

        break;
      }
    case MT9M112:
      {
#if defined MT9M112_CAM
        wrSensorRegs8_16(MT9M112_QVGA);
#endif
        break;
      }
    case MT9V111:
      {
#if defined MT9V111_CAM
        //Reset sensor core
        wrSensorReg8_16(0x01, 0x04);
        wrSensorReg8_16(0x0D, 0x01);
        wrSensorReg8_16(0x0D, 0x00);
        //Reset IFP
        wrSensorReg8_16(0x01, 0x01);
        wrSensorReg8_16(0x07, 0x01);
        wrSensorReg8_16(0x07, 0x00);
        delay(100);
        wrSensorRegs8_16(MT9V111_QVGA);
        //delay(1000);
        wrSensorReg8_16(0x97, 0x0020);
        wrSensorReg8_16(0xf0, 0x00);
        wrSensorReg8_16(0x21, 0x8403); //Mirror Column
        wrSensorReg8_16(0xC6, 0xA103);//SEQ_CMD
        wrSensorReg8_16(0xC8, 0x0005); //SEQ_CMD
#endif
        break;
      }

    case MT9M001:
      {
#if defined MT9M001_CAM
        wrSensorRegs8_16(MT9M001_QVGA_30fps);
#endif
        break;
      }
    case MT9T112:
      {
#if defined MT9T112_CAM

        wrSensorReg16_16(0x001a , 0x0219 );
        wrSensorReg16_16(0x001a , 0x0018 );
        //reset camera
        wrSensorReg16_16(0x0014 , 0x2425 );
        wrSensorReg16_16(0x0014 , 0x2145 );
        wrSensorReg16_16(0x0010 , 0x0110 );
        wrSensorReg16_16(0x0012 , 0x00f0 );
        wrSensorReg16_16(0x002a , 0x7f77 );
        wrSensorReg16_16(0x0014 , 0x2545 );
        wrSensorReg16_16(0x0014 , 0x2547 );
        wrSensorReg16_16(0x0014 , 0x3447 );
        wrSensorReg16_16(0x0014 , 0x3047 );
        delay(10);
        wrSensorReg16_16(0x0014 , 0x3046 );
        wrSensorReg16_16(0x0022 , 0x01f4 );
        wrSensorReg16_16(0x001e , 0x0707 );
        wrSensorReg16_16(0x3b84 , 0x01f4 );
        wrSensorReg16_16(0x002e , 0x0500 );
        wrSensorReg16_16(0x0018 , 0x402b );
        wrSensorReg16_16(0x3b82 , 0x0004 );
        wrSensorReg16_16(0x0018 , 0x402f );
        wrSensorReg16_16(0x0018 , 0x402e );
        delay(50);
        wrSensorReg16_16(0x0614 , 0x0001 );
        delay(1);
        wrSensorReg16_16(0x0614 , 0x0001 );
        delay(1);
        wrSensorReg16_16(0x0614 , 0x0001 );
        delay(1);
        wrSensorReg16_16(0x0614 , 0x0001 );
        delay(1);
        wrSensorReg16_16(0x0614 , 0x0001 );
        delay(1);
        wrSensorReg16_16(0x0614 , 0x0001 );
        delay(1);
        delay(10);
        //init pll
        wrSensorReg16_16(0x098e , 0x6800 );
        wrSensorReg16_16(0x0990 , 0x0140 );
        wrSensorReg16_16(0x098e , 0x6802 );
        wrSensorReg16_16(0x0990 , 0x00f0 );
        wrSensorReg16_16(0x098e , 0x68a0 );
        wrSensorReg16_16(0x098e , 0x68a0 );
        wrSensorReg16_16(0x0990 , 0x082d );
        wrSensorReg16_16(0x098e , 0x4802 );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x098e , 0x4804 );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x098e , 0x4806 );
        wrSensorReg16_16(0x0990 , 0x060d );
        wrSensorReg16_16(0x098e , 0x4808 );
        wrSensorReg16_16(0x0990 , 0x080d );
        wrSensorReg16_16(0x098e , 0x480c );
        wrSensorReg16_16(0x0990 , 0x046c );
        wrSensorReg16_16(0x098e , 0x480f );
        wrSensorReg16_16(0x0990 , 0x00cc );
        wrSensorReg16_16(0x098e , 0x4811 );
        wrSensorReg16_16(0x0990 , 0x0381 );
        wrSensorReg16_16(0x098e , 0x4813 );
        wrSensorReg16_16(0x0990 , 0x024f );
        wrSensorReg16_16(0x098e , 0x481d );
        wrSensorReg16_16(0x0990 , 0x0436 );
        wrSensorReg16_16(0x098e , 0x481f );
        wrSensorReg16_16(0x0990 , 0x05d0 );
        wrSensorReg16_16(0x098e , 0x4825 );
        wrSensorReg16_16(0x0990 , 0x1153 );
        wrSensorReg16_16(0x098e , 0x6ca0 );
        wrSensorReg16_16(0x098e , 0x6ca0 );
        wrSensorReg16_16(0x0990 , 0x082d );
        wrSensorReg16_16(0x098e , 0x484a );
        wrSensorReg16_16(0x0990 , 0x0004 );
        wrSensorReg16_16(0x098e , 0x484c );
        wrSensorReg16_16(0x0990 , 0x0004 );
        wrSensorReg16_16(0x098e , 0x484e );
        wrSensorReg16_16(0x0990 , 0x060b );
        wrSensorReg16_16(0x098e , 0x4850 );
        wrSensorReg16_16(0x0990 , 0x080b );
        wrSensorReg16_16(0x098e , 0x4857 );
        wrSensorReg16_16(0x0990 , 0x008c );
        wrSensorReg16_16(0x098e , 0x4859 );
        wrSensorReg16_16(0x0990 , 0x01f1 );
        wrSensorReg16_16(0x098e , 0x485b );
        wrSensorReg16_16(0x0990 , 0x00ff );
        wrSensorReg16_16(0x098e , 0x4865 );
        wrSensorReg16_16(0x0990 , 0x0668 );
        wrSensorReg16_16(0x098e , 0x4867 );
        wrSensorReg16_16(0x0990 , 0x0af0 );
        wrSensorReg16_16(0x098e , 0x486d );
        wrSensorReg16_16(0x0990 , 0x0af0 );
        wrSensorReg16_16(0x098e , 0xa005 );
        wrSensorReg16_16(0x0990 , 0x0001 );
        wrSensorReg16_16(0x098e , 0x6c11 );
        wrSensorReg16_16(0x0990 , 0x0003 );
        wrSensorReg16_16(0x098e , 0x6811 );
        wrSensorReg16_16(0x0990 , 0x0003 );
        wrSensorReg16_16(0x098e , 0xc8a5 );
        wrSensorReg16_16(0x0990 , 0x0025 );
        wrSensorReg16_16(0x098e , 0xc8a6 );
        wrSensorReg16_16(0x0990 , 0x0028 );
        wrSensorReg16_16(0x098e , 0xc8a7 );
        wrSensorReg16_16(0x0990 , 0x002c );
        wrSensorReg16_16(0x098e , 0xc8a8 );
        wrSensorReg16_16(0x0990 , 0x002f );
        wrSensorReg16_16(0x098e , 0xc844 );
        wrSensorReg16_16(0x0990 , 0x00ba );
        wrSensorReg16_16(0x098e , 0xc92f );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x098e , 0xc845 );
        wrSensorReg16_16(0x0990 , 0x009b );
        wrSensorReg16_16(0x098e , 0xc92d );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x098e , 0xc88c );
        wrSensorReg16_16(0x0990 , 0x0082 );
        wrSensorReg16_16(0x098e , 0xc930 );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x098e , 0xc88d );
        wrSensorReg16_16(0x0990 , 0x006d );
        wrSensorReg16_16(0x098e , 0xc92e );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x098e , 0xa002 );
        wrSensorReg16_16(0x0990 , 0x0010 );
        wrSensorReg16_16(0x098e , 0xa009 );
        wrSensorReg16_16(0x0990 , 0x0002 );
        wrSensorReg16_16(0x098e , 0xa00a );
        wrSensorReg16_16(0x0990 , 0x0003 );
        wrSensorReg16_16(0x098e , 0xa00c );
        wrSensorReg16_16(0x0990 , 0x000a );
        wrSensorReg16_16(0x098e , 0x4846 );
        wrSensorReg16_16(0x0990 , 0x0014 );
        wrSensorReg16_16(0x098e , 0x488e );
        wrSensorReg16_16(0x0990 , 0x0014 );
        wrSensorReg16_16(0x098e , 0xc844 );
        wrSensorReg16_16(0x0990 , 0x0085 );
        wrSensorReg16_16(0x098e , 0xc845 );
        wrSensorReg16_16(0x0990 , 0x006e );
        wrSensorReg16_16(0x098e , 0xc88c );
        wrSensorReg16_16(0x0990 , 0x0082 );
        wrSensorReg16_16(0x098e , 0xc88d );
        wrSensorReg16_16(0x0990 , 0x006c );
        wrSensorReg16_16(0x098e , 0xc8a5 );
        wrSensorReg16_16(0x0990 , 0x001b );
        wrSensorReg16_16(0x098e , 0xc8a6 );
        wrSensorReg16_16(0x0990 , 0x001e );
        wrSensorReg16_16(0x098e , 0xc8a7 );
        wrSensorReg16_16(0x0990 , 0x0020 );
        wrSensorReg16_16(0x098e , 0xc8a8 );
        wrSensorReg16_16(0x0990 , 0x0023 );
        //init setting
        wrSensorReg16_16(0x0018 , 0x002a );
        wrSensorReg16_16(0x3084 , 0x2409 );
        wrSensorReg16_16(0x3092 , 0x0a49 );
        wrSensorReg16_16(0x3094 , 0x4949 );
        wrSensorReg16_16(0x3096 , 0x4950 );
        wrSensorReg16_16(0x098e , 0x68a0 );
        wrSensorReg16_16(0x0990 , 0x0a2e );
        wrSensorReg16_16(0x098e , 0x6ca0 );
        wrSensorReg16_16(0x0990 , 0x0a2e );
        wrSensorReg16_16(0x098e , 0x6c90 );
        wrSensorReg16_16(0x0990 , 0x0cb4 );
        wrSensorReg16_16(0x098e , 0x6807 );
        wrSensorReg16_16(0x0990 , 0x0004 );
        wrSensorReg16_16(0x098e , 0xe88e );
        wrSensorReg16_16(0x0990 , 0x0000 );
        wrSensorReg16_16(0x316c , 0x350f );
        wrSensorReg16_16(0x001e , 0x0777 );
        wrSensorReg16_16(0x098e , 0x8400 );
        wrSensorReg16_16(0x0990 , 0x0001 );
        delay(100);
        wrSensorReg16_16(0x098e , 0x8400 );
        wrSensorReg16_16(0x0990 , 0x0006 );
        //Serial.println("MT9T112 init done");
#endif
        break;
      }
    case MT9D112:
      {
#if defined MT9D112_CAM
        wrSensorReg16_16(0x301a , 0x0acc );
        wrSensorReg16_16(0x3202 , 0x0008 );
        delay(100 );
        wrSensorReg16_16(0x341e , 0x8f09 );
        wrSensorReg16_16(0x341c , 0x020c );
        delay(100 );
        wrSensorRegs16_16(MT9D112_default_setting);
        wrSensorReg16_16(0x338c , 0xa103 );
        wrSensorReg16_16(0x3390 , 0x0006 );
        delay(100 );
        wrSensorReg16_16(0x338c , 0xa103 );
        wrSensorReg16_16(0x3390 , 0x0005 );
        delay(100 );
        wrSensorRegs16_16(MT9D112_soc_init);
        delay(200);
        wrSensorReg16_16(0x332E, 0x0020); //RGB565

#endif
      }
    default:

      break;
  }
}

void ArduCAM::flush_fifo(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

void ArduCAM::start_capture(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);
}

void ArduCAM::clear_fifo_flag(void )
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

uint32_t ArduCAM::read_fifo_length(void)
{
	uint32_t len1,len2,len3,length=0;
	len1 = read_reg(FIFO_SIZE1);
  len2 = read_reg(FIFO_SIZE2);
  len3 = read_reg(FIFO_SIZE3) & 0x7f;
  length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
	return length;	
}

#if defined (RASPBERRY_PI)
uint8_t ArduCAM::transfer(uint8_t data)
{
  uint8_t temp;
  temp = arducam_spi_transfer(data);
  return temp;
}

void ArduCAM::transfers(uint8_t *buf, uint32_t size)
{
	arducam_spi_transfers(buf, size);
}

#endif

void ArduCAM::set_fifo_burst()
{
	#if defined (RASPBERRY_PI)
	transfer(BURST_FIFO_READ);
	#else
    SPI.transfer(BURST_FIFO_READ);
   #endif
		
}

void ArduCAM::CS_HIGH(void)
{
	 sbi(P_CS, B_CS);	
}
void ArduCAM::CS_LOW(void)
{
	 cbi(P_CS, B_CS);	
}

uint8_t ArduCAM::read_fifo(void)
{
	uint8_t data;
	data = bus_read(SINGLE_FIFO_READ);
	return data;
}

uint8_t ArduCAM::read_reg(uint8_t addr)
{
	uint8_t data;
	#if defined (RASPBERRY_PI)
		data = bus_read(addr);	
	#else
		data = bus_read(addr & 0x7F);
	#endif
	return data;
}

void ArduCAM::write_reg(uint8_t addr, uint8_t data)
{
	#if defined (RASPBERRY_PI)
		bus_write(addr , data);
	#else
	 bus_write(addr | 0x80, data);
  #endif  
}

//My add
//Set corresponding bit  
void ArduCAM::set_bit(uint8_t addr, uint8_t bit)
{
	uint8_t temp;
	temp = read_reg(addr);
	write_reg(addr, temp | bit);
}
//Clear corresponding bit 
void ArduCAM::clear_bit(uint8_t addr, uint8_t bit)
{
	uint8_t temp;
	temp = read_reg(addr);
	write_reg(addr, temp & (~bit));
}

//Get corresponding bit status
uint8_t ArduCAM::get_bit(uint8_t addr, uint8_t bit)
{
  uint8_t temp;
  temp = read_reg(addr);
  temp = temp & bit;
  return temp;
}

//Set ArduCAM working mode
//MCU2LCD_MODE: MCU writes the LCD screen GRAM
//CAM2LCD_MODE: Camera takes control of the LCD screen
//LCD2MCU_MODE: MCU read the LCD screen GRAM
void ArduCAM::set_mode(uint8_t mode)
{
  switch (mode)
  {
    case MCU2LCD_MODE:
      write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
      break;
    case CAM2LCD_MODE:
      write_reg(ARDUCHIP_MODE, CAM2LCD_MODE);
      break;
    case LCD2MCU_MODE:
      write_reg(ARDUCHIP_MODE, LCD2MCU_MODE);
      break;
    default:
      write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
      break;
  }
}

uint8_t ArduCAM::bus_write(int address,int value)
{	
	cbi(P_CS, B_CS);
	#if defined (RASPBERRY_PI)
		arducam_spi_write(address | 0x80, value);
	#else
		SPI.transfer(address);
		SPI.transfer(value);
	#endif
	sbi(P_CS, B_CS);
	return 1;
}

uint8_t ArduCAM:: bus_read(int address)
{
	uint8_t value;
	cbi(P_CS, B_CS);
	#if defined (RASPBERRY_PI)
		value = arducam_spi_read(address & 0x7F);
		sbi(P_CS, B_CS);
		return value;	
	#else
		#if (defined(ESP8266) || defined(__arm__))
		#if defined(OV5642_MINI_5MP)
		  SPI.transfer(address);
		  value = SPI.transfer(0x00);
		  // correction for bit rotation from readback
		  value = (byte)(value >> 1) | (value << 7);
		  // take the SS pin high to de-select the chip:
		  sbi(P_CS, B_CS);
		  return value;
		#else
		  SPI.transfer(address);
		  value = SPI.transfer(0x00);
		  // take the SS pin high to de-select the chip:
		  sbi(P_CS, B_CS);
		  return value;
		#endif
		#else
		  SPI.transfer(address);
		  value = SPI.transfer(0x00);
		  // take the SS pin high to de-select the chip:
		  sbi(P_CS, B_CS);
		  return value;
		#endif
#endif
}



void ArduCAM::OV2640_set_JPEG_size(uint8_t size)
{
 #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP))
	switch(size)
	{
		case OV2640_160x120:
			wrSensorRegs8_8(OV2640_160x120_JPEG);
			break;
		case OV2640_176x144:
			wrSensorRegs8_8(OV2640_176x144_JPEG);
			break;
		case OV2640_320x240:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
		case OV2640_352x288:
	  	wrSensorRegs8_8(OV2640_352x288_JPEG);
			break;
		case OV2640_640x480:
			wrSensorRegs8_8(OV2640_640x480_JPEG);
			break;
		case OV2640_800x600:
			wrSensorRegs8_8(OV2640_800x600_JPEG);
			break;
		case OV2640_1024x768:
			wrSensorRegs8_8(OV2640_1024x768_JPEG);
			break;
		case OV2640_1280x1024:
			wrSensorRegs8_8(OV2640_1280x1024_JPEG);
			break;
		case OV2640_1600x1200:
			wrSensorRegs8_8(OV2640_1600x1200_JPEG);
			break;
		default:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
	}
#endif
}

void ArduCAM::OV5642_set_JPEG_size(uint8_t size)
{
#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)
  uint8_t reg_val;

  switch (size)
  {
    case OV5642_320x240:
      wrSensorRegs16_8(ov5642_320x240);
      break;
    case OV5642_640x480:
      wrSensorRegs16_8(ov5642_640x480);
      break;
    case OV5642_1024x768:
      wrSensorRegs16_8(ov5642_1024x768);
      break;
    case OV5642_1280x960:
      wrSensorRegs16_8(ov5642_1280x960);
      break;
    case OV5642_1600x1200:
      wrSensorRegs16_8(ov5642_1600x1200);
      break;
    case OV5642_2048x1536:
      wrSensorRegs16_8(ov5642_2048x1536);
      break;
    case OV5642_2592x1944:
      wrSensorRegs16_8(ov5642_2592x1944);
      break;
    default:
      wrSensorRegs16_8(ov5642_320x240);
      break;
  }
#endif
}


void ArduCAM::OV5640_set_JPEG_size(uint8_t size)
{
#if (defined (OV5640_CAM)||defined (OV5640_MINI_5MP_PLUS))
  switch (size)
  {
    case OV5640_320x240:
      wrSensorRegs16_8(OV5640_QSXGA2QVGA);
      break;
    case OV5640_352x288:
      wrSensorRegs16_8(OV5640_QSXGA2CIF);
      break;
    case OV5640_640x480:
      wrSensorRegs16_8(OV5640_QSXGA2VGA);
      break;
    case OV5640_800x480:
      wrSensorRegs16_8(OV5640_QSXGA2WVGA);
      break;
    case OV5640_1024x768:
      wrSensorRegs16_8(OV5640_QSXGA2XGA);
      break;
    case OV5640_1280x960:
      wrSensorRegs16_8(OV5640_QSXGA2SXGA);
      break;
    case OV5640_1600x1200:
      wrSensorRegs16_8(OV5640_QSXGA2UXGA);
      break;
    case OV5640_2048x1536:
      wrSensorRegs16_8(OV5640_QSXGA2QXGA);
      break;
    case OV5640_2592x1944:
      wrSensorRegs16_8(OV5640_JPEG_QSXGA);
      break;
    default:
      //320x240
      wrSensorRegs16_8(OV5640_QSXGA2QVGA);
      break;
  }
#endif

}

void ArduCAM::set_format(byte fmt)
{
  if (fmt == BMP)
    m_fmt = BMP;
  else
    m_fmt = JPEG;
}
	// Write 8 bit values to 8 bit register address
int ArduCAM::wrSensorRegs8_8(const struct sensor_reg reglist[])
{
	#if defined (RASPBERRY_PI)
		arducam_i2c_write_regs(reglist);
	#else
		int err = 0;
	  uint16_t reg_addr = 0;
	  uint16_t reg_val = 0;
	  const struct sensor_reg *next = reglist;
	  while ((reg_addr != 0xff) | (reg_val != 0xff))
	  {
	    reg_addr = pgm_read_word(&next->reg);
	    reg_val = pgm_read_word(&next->val);
	    err = wrSensorReg8_8(reg_addr, reg_val);
	    next++;
		#if defined(ESP8266)
		    yield();
		#endif
	  }
 #endif  
	return 1;
}

	// Write 16 bit values to 8 bit register address
int ArduCAM::wrSensorRegs8_16(const struct sensor_reg reglist[])
{
	#if defined (RASPBERRY_PI)
		arducam_i2c_write_regs16(reglist);
	#else
		int err = 0;
	  unsigned int reg_addr, reg_val;
	  const struct sensor_reg *next = reglist;
	
	  while ((reg_addr != 0xff) | (reg_val != 0xffff))
	  {
	  	#if defined (RASPBERRY_PI)
		   reg_addr =next->reg;
       reg_val = next->val;
	   #else
	     reg_addr = pgm_read_word(&next->reg);
	     reg_val = pgm_read_word(&next->val);
	    #endif
	    err = wrSensorReg8_16(reg_addr, reg_val);
	    //  if (!err)
	    //return err;
	    next++;
		#if defined(ESP8266)
			yield();
		#endif
	  }
  #endif
	return 1;
}

// Write 8 bit values to 16 bit register address
int ArduCAM::wrSensorRegs16_8(const struct sensor_reg reglist[])
{
	#if defined (RASPBERRY_PI)
		arducam_i2c_write_word_regs(reglist);
	#else
		int err = 0;
	  unsigned int reg_addr;
	  unsigned char reg_val;
	  const struct sensor_reg *next = reglist;
	
	  while ((reg_addr != 0xffff) | (reg_val != 0xff))
	  {
	  	
	   #if defined (RASPBERRY_PI)
		   reg_addr =next->reg;
       reg_val = next->val;
	   #else
	     reg_addr = pgm_read_word(&next->reg);
	     reg_val = pgm_read_word(&next->val);
	    #endif
	    err = wrSensorReg16_8(reg_addr, reg_val);
	    //if (!err)
	    //return err;
	    next++;
		#if defined(ESP8266)
			yield();
		#endif
	  }
	#endif
	return 1;
}

//I2C Array Write 16bit address, 16bit data
int ArduCAM::wrSensorRegs16_16(const struct sensor_reg reglist[])
{
	#if defined (RASPBERRY_PI)
	#else
	  int err = 0;
	  unsigned int reg_addr, reg_val;
	  const struct sensor_reg *next = reglist;
	  reg_addr = pgm_read_word(&next->reg);
	  reg_val = pgm_read_word(&next->val);
	  while ((reg_addr != 0xffff) | (reg_val != 0xffff))
	  {
	    err = wrSensorReg16_16(reg_addr, reg_val);
	    //if (!err)
	    //   return err;
	    next++;
	    reg_addr = pgm_read_word(&next->reg);
	    reg_val = pgm_read_word(&next->val);
			#if defined(ESP8266)
			    yield();
			#endif
	  }
	#endif
  return 1;
}



// Read/write 8 bit value to/from 8 bit register address	
byte ArduCAM::wrSensorReg8_8(int regID, int regDat)
{
	#if defined (RASPBERRY_PI)
		arducam_i2c_write( regID , regDat );
	#else
		Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID & 0x00FF);
	  Wire.write(regDat & 0x00FF);
	  if (Wire.endTransmission())
	  {
	    return 0;
	  }
	  delay(1);
	#endif
	return 1;
	
}
byte ArduCAM::rdSensorReg8_8(uint8_t regID, uint8_t* regDat)
{	
	#if defined (RASPBERRY_PI) 
		arducam_i2c_read(regID,regDat);
	#else
		Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID & 0x00FF);
	  Wire.endTransmission();
	
	  Wire.requestFrom((sensor_addr >> 1), 1);
	  if (Wire.available())
	    *regDat = Wire.read();
	  delay(1);
	#endif
	return 1;
	
}
// Read/write 16 bit value to/from 8 bit register address
byte ArduCAM::wrSensorReg8_16(int regID, int regDat)
{
	#if defined (RASPBERRY_PI) 
		arducam_i2c_write16(regID, regDat );
	#else
		Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID & 0x00FF);
	
	  Wire.write(regDat >> 8);            // sends data byte, MSB first
	  Wire.write(regDat & 0x00FF);
	  if (Wire.endTransmission())
	  {
	    return 0;
	  }	
	  delay(1);
	#endif
	return 1;
}
byte ArduCAM::rdSensorReg8_16(uint8_t regID, uint16_t* regDat)
{
	#if defined (RASPBERRY_PI) 
  	arducam_i2c_read16(regID, regDat);
  #else
  	uint8_t temp;
	  Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID);
	  Wire.endTransmission();
	
	  Wire.requestFrom((sensor_addr >> 1), 2);
	  if (Wire.available())
	  {
	    temp = Wire.read();
	    *regDat = (temp << 8) | Wire.read();
	  }
	  delay(1);
	#endif
  	return 1;
}

// Read/write 8 bit value to/from 16 bit register address
byte ArduCAM::wrSensorReg16_8(int regID, int regDat)
{
	#if defined (RASPBERRY_PI) 
		arducam_i2c_word_write(regID, regDat);
		//arducam_delay_ms(1);
	#else
		Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID >> 8);            // sends instruction byte, MSB first
	  Wire.write(regID & 0x00FF);
	  Wire.write(regDat & 0x00FF);
	  if (Wire.endTransmission())
	  {
	    return 0;
	  }
	  delay(1);
	#endif
	return 1;
}
byte ArduCAM::rdSensorReg16_8(uint16_t regID, uint8_t* regDat)
{
	#if defined (RASPBERRY_PI) 
		arducam_i2c_word_read(regID, regDat );
	#else
		Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID >> 8);
	  Wire.write(regID & 0x00FF);
	  Wire.endTransmission();
	  Wire.requestFrom((sensor_addr >> 1), 1);
	  if (Wire.available())
	  {
	    *regDat = Wire.read();
	  }
	  delay(1);
	#endif  
	return 1;
}

//I2C Write 16bit address, 16bit data
byte ArduCAM::wrSensorReg16_16(int regID, int regDat)
{
	#if defined (RASPBERRY_PI)
	#else
	  Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID >> 8);            // sends instruction byte, MSB first
	  Wire.write(regID & 0x00FF);
	  Wire.write(regDat >> 8);            // sends data byte, MSB first
	  Wire.write(regDat & 0x00FF);
	  if (Wire.endTransmission())
	  {
	    return 0;
	  }
	  delay(1);
  #endif
  return (1);
}

//I2C Read 16bit address, 16bit data
byte ArduCAM::rdSensorReg16_16(uint16_t regID, uint16_t* regDat)
{
	#if defined (RASPBERRY_PI)
	#else
	  uint16_t temp;
	  Wire.beginTransmission(sensor_addr >> 1);
	  Wire.write(regID >> 8);
	  Wire.write(regID & 0x00FF);
	  Wire.endTransmission();
	  Wire.requestFrom((sensor_addr >> 1), 2);
	  if (Wire.available())
	  {
	    temp = Wire.read();
	    *regDat = (temp << 8) | Wire.read();
	  }
	  delay(1);
	#endif 
  return (1);
}
#if defined(ESP8266)
inline void ArduCAM::setDataBits(uint16_t bits) {
  const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
  bits--;
  SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}

void ArduCAM::transferBytes_(uint8_t * out, uint8_t * in, uint8_t size) {
  while (SPI1CMD & SPIBUSY) {}
  // Set in/out Bits to transfer

  setDataBits(size * 8);

  volatile uint32_t * fifoPtr = &SPI1W0;
  uint8_t dataSize = ((size + 3) / 4);

  if (out) {
    uint32_t * dataPtr = (uint32_t*) out;
    while (dataSize--) {
      *fifoPtr = *dataPtr;
      dataPtr++;
      fifoPtr++;
    }
  } else {
    // no out data only read fill with dummy data!
    while (dataSize--) {
      *fifoPtr = 0xFFFFFFFF;
      fifoPtr++;
    }
  }

  SPI1CMD |= SPIBUSY;
  while (SPI1CMD & SPIBUSY) {}

  if (in) {
    volatile uint8_t * fifoPtr8 = (volatile uint8_t *) &SPI1W0;
    dataSize = size;
    while (dataSize--) {
#if defined(OV5642_MINI_5MP)
      *in = *fifoPtr8;
      *in = (byte)(*in >> 1) | (*in << 7);
#else
      *in = *fifoPtr8;
#endif
      in++;
      fifoPtr8++;
    }
  }
}

void ArduCAM::transferBytes(uint8_t * out, uint8_t * in, uint32_t size) {
  while (size) {
    if (size > 64) {
      transferBytes_(out, in, 64);
      size -= 64;
      if (out) out += 64;
      if (in) in += 64;
    } else {
      transferBytes_(out, in, size);
      size = 0;
    }
  }
}
#endif
