/*
 *  	encoder_as5048a.c
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
#include "encoder_as5048a.h"




void AS5048AReadAngleBit(uint8_t cs_pin, uint16_t *angle)
{
	uint8_t 	rdata[2];
	uint8_t 	wdata[2];
	uint16_t 	angle_tmp;
	wdata[0]=0xFF;
	wdata[1]=0xFF;
	gpio_put(cs_pin, 0);
	spi_write_read_blocking(SPI_PORT,wdata,rdata,2);
	gpio_put(cs_pin, 1);
	angle_tmp=((uint16_t)rdata[0])<<8;
	angle_tmp=(angle_tmp+rdata[1])&0x3FFF;
	angle_tmp=angle_tmp<<2;//Compatible for MA732
	*angle=angle_tmp;
}


