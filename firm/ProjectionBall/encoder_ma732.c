/*
 *  	encoder_ma732.c
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ProjectionBall.h"
#include "motor_ctrl.h"
#include "encoder_ma732.h"






void MA732ReadAngleBit(uint8_t cs_pin, uint16_t *angle)
{
	uint8_t 	rdata[2];
	uint8_t 	wdata[2];
	uint16_t 	angle_tmp;
	wdata[0]=0x00;
	wdata[1]=0x00;
	gpio_put(cs_pin, 0);
	spi_write_read_blocking(SPI_PORT,wdata,rdata,2);
	gpio_put(cs_pin, 1);
	angle_tmp=((uint16_t)rdata[0])<<8;
	angle_tmp=(angle_tmp+rdata[1])&0xFFFF;
	*angle=angle_tmp;
}



void MA732ReadReg(uint8_t cs_pin, uint8_t reg, uint8_t *val)
{
	uint8_t 	rdata[2];
	uint8_t 	wdata[2];
	wdata[0]= 0x40 + (reg&0x1F);
	wdata[1]= 0x00;
	gpio_put(cs_pin, 0);
	spi_write_read_blocking(SPI_PORT,wdata,rdata,2);
	gpio_put(cs_pin, 1);
	sleep_ms(30);
	gpio_put(cs_pin, 0);
	rdata[0]=0;
	rdata[1]=0;
	wdata[0]=0;
	wdata[1]=0;
	spi_write_read_blocking(SPI_PORT,wdata,rdata,2);
	gpio_put(cs_pin, 1);
	*val=rdata[0];
	//printf(" ::%X %X ",rdata[0],rdata[1]);
}
void MA732WriteReg(uint8_t cs_pin, uint8_t reg, uint8_t val)
{
	uint8_t 	rdata[2];
	uint8_t 	wdata[2];
	wdata[0]= 0x80 + (reg&0x1F);
	wdata[1]= val;
	gpio_put(cs_pin, 0);
	spi_write_read_blocking(SPI_PORT,wdata,rdata,2);
	gpio_put(cs_pin, 1);
	sleep_ms(20);
	wdata[0]=0x00;
	wdata[1]=0x00;
	gpio_put(cs_pin, 0);
	spi_write_read_blocking(SPI_PORT,wdata,rdata,2);
	gpio_put(cs_pin, 1);
}
