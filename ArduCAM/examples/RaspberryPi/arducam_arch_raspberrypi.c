/*-----------------------------------------

//Update History:
//2016/06/13 	V1.1	by Lee	add support for burst mode

--------------------------------------*/

#include <linux/i2c-dev.h>
#include <wiringPiSPI.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "arducam_arch_raspberrypi.h"


#define	SPI_ARDUCAM_SPEED	1000000
#define	SPI_ARDUCAM		      0

static int FD;

bool wiring_init(void)
{
	wiringPiSetup();
	int spi = wiringPiSPISetup(SPI_ARDUCAM, SPI_ARDUCAM_SPEED);   
	return spi != -1;
}
bool arducam_i2c_init(uint8_t sensor_addr)
{
	FD = wiringPiI2CSetup(sensor_addr);
	return FD != -1;
}
void arducam_delay_ms(uint32_t delay)
{
  usleep(1000*delay);	
}

void arducam_spi_write(uint8_t address, uint8_t value)
{
	uint8_t spiData [2] ;
	spiData [0] = address ;
  spiData [1] = value ; 
	 wiringPiSPIDataRW (SPI_ARDUCAM, spiData, 2) ;
}


uint8_t arducam_spi_read(uint8_t address)
{
	uint8_t spiData[2];
	spiData[0] = address ;
	spiData[1] = 0x00 ;
  	wiringPiSPIDataRW (SPI_ARDUCAM, spiData, 2) ;
  	return spiData[1];
}


void arducam_spi_transfers(uint8_t *buf, uint32_t size)
{
	 wiringPiSPIDataRW (SPI_ARDUCAM, buf, size) ;
}

uint8_t arducam_spi_transfer(uint8_t data)
{
	uint8_t spiData [1] ;
	 spiData [0] = data ;
	 wiringPiSPIDataRW (SPI_ARDUCAM, spiData, 1) ;
	 return spiData [0];
}

uint8_t arducam_i2c_write(uint8_t regID, uint8_t regDat)
{
	if(FD != -1)
	{
		wiringPiI2CWriteReg8(FD,regID,regDat);
		return(1);
	}
	return 0;
}

uint8_t arducam_i2c_read(uint8_t regID, uint8_t* regDat)
{
	if(FD != -1)
	{
		*regDat = wiringPiI2CReadReg8(FD,regID);
		return(1);
	}
	return 0;
}

uint8_t arducam_i2c_write16(uint8_t regID, uint16_t regDat)
{
	if(FD != -1)
	{
		wiringPiI2CWriteReg16(FD,regID,regDat);
		
		return(1);
	}
	return 0;
}

uint8_t arducam_i2c_read16(uint8_t regID, uint16_t* regDat)
{
	if(FD != -1)
	{
		*regDat = wiringPiI2CReadReg16(FD,regID);
		return(1);
	}
	return 0;
}

uint8_t arducam_i2c_word_write(uint16_t regID, uint8_t regDat)
{
	uint8_t reg_H,reg_L;
	uint16_t value;
	reg_H = (regID >> 8) & 0x00ff;
	reg_L = regID & 0x00ff;
	value =  regDat << 8 | reg_L;
	if(FD != -1)
	{
		i2c_smbus_write_word_data(FD, reg_H, value);
		arducam_delay_ms(1);
		return(1);
	}
	return 0;
}

uint8_t arducam_i2c_word_read(uint16_t regID, uint8_t* regDat)
{
	uint8_t reg_H,reg_L;
	int r;
	reg_H = (regID >> 8) & 0x00ff;
	reg_L = regID & 0x00ff;
	if(FD != -1)
	{
		r = i2c_smbus_write_byte_data(FD,reg_H,reg_L);
		if(r<0)
			return 0;
		*regDat = i2c_smbus_read_byte(FD);
		return(1);
	}
	return 0;
}

int arducam_i2c_write_regs(const struct sensor_reg reglist[])
{
	uint16_t reg_addr = 0;
	uint16_t reg_val = 0;
	const struct sensor_reg *next = reglist;

	while ((reg_addr != 0xff) | (reg_val != 0xff))
	{
		//reg_addr = pgm_read_word(&next->reg);
		//reg_val = pgm_read_word(&next->val);
		reg_addr = next->reg;
		reg_val = next->val;
		if (!arducam_i2c_write(reg_addr, reg_val)) {
			return 0;
		}
	   	next++;
	}

	return 1;
}


int arducam_i2c_write_regs16(const struct sensor_reg reglist[])
{
	unsigned int reg_addr,reg_val;
	const struct sensor_reg *next = reglist;

	while ((reg_addr != 0xff) | (reg_val != 0xffff))
	{
		//reg_addr = pgm_read_word(&next->reg);
		//reg_val = pgm_read_word(&next->val);
		reg_addr = next->reg;
		reg_val = next->val;
		if (!arducam_i2c_write16(reg_addr, reg_val)) {
			return 0;
		}
	   	next++;
	   	arducam_delay_ms(1);
	}

	return 1;
}

int arducam_i2c_write_word_regs(const struct sensor_reg reglist[])
{
	unsigned int reg_addr,reg_val;
	const struct sensor_reg *next = reglist;

	while ((reg_addr != 0xffff) | (reg_val != 0xff))
	{
		 //reg_addr = pgm_read_word(&next->reg);
		 //reg_val =  pgm_read_word(&next->val);
		 reg_addr = next->reg;
		 reg_val = next->val;
		 //if (!arducam_i2c_write16(reg_addr, reg_val)) 
		
		 //My changed
		if (!arducam_i2c_word_write(reg_addr, reg_val))
			{
			return 0;
		}
	   	next++;
	   arducam_delay_ms(1);
	}

	return 1;
}
