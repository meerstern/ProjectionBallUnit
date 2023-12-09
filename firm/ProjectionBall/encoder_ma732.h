/*
 *  	encoder_ma732.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef ENCODER_MA732_H
#define ENCODER_MA732_H


#define MA732_REG_FW	0x0E//FW:0x33:64us,0x44:128us,0x55::256us,0x66:512us,0x77:1024us
#define MA732_REG_MG	0x1B
#define MA732_REG_HYS   0x10
#define MA732_REG_BCT   0x02
#define MA732_REG_ET    0x03
#define MA732_REG_MGL   0x06

void MA732ReadAngleBit(uint8_t cs_pin, uint16_t *angle);
void MA732ReadReg(uint8_t cs_pin, uint8_t reg, uint8_t *val);
void MA732WriteReg(uint8_t cs_pin, uint8_t reg, uint8_t val);


#endif