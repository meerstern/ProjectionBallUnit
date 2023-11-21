/*
 *  	rtc_sd30XX.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescent
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef RTC_SD30XX_H
#define RTC_SD30XX_H

#include <stdint.h>
#include "ProjectionBall.h"

#ifdef USE_RTC_SD30XX

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5



#define SD30XX_ADDR       (0x32<<0)

#define SD30XX_REG_SEC         0x00
#define SD30XX_REG_MIN         0x01
#define SD30XX_REG_HOUR        0x02
#define SD30XX_REG_WEEK        0x03
#define SD30XX_REG_DATE        0x04
#define SD30XX_REG_MNTH        0x05
#define SD30XX_REG_YEAR        0x06
#define SD30XX_REG_CTR1        0x0F
#define SD30XX_REG_CTR2        0x10
#define SD30XX_REG_CTR3        0x11



#define SD30XX_REG_RAM_1       0x2C    //MODE
#define SD30XX_REG_RAM_2       0x2D    //PATTERN  
#define SD30XX_REG_RAM_3       0x2E    //
#define SD30XX_REG_RAM_4       0x2F    //
#define SD30XX_REG_RAM_5       0x30    //
#define SD30XX_REG_RAM_6       0x31    //
//#define SD30XX_REG_RAM_7       0x32
//#define SD30XX_REG_RAM_8       0x33
//#define SD30XX_REG_RAM_9       0x34
//#define SD30XX_REG_RAM_10      0x35
//#define SD30XX_REG_RAM_11      0x36

void GetDateTime();
void SetRtcRam(uint8_t addr, uint8_t data);
void GetRtcRam(uint8_t addr, uint8_t *data);

void SetRtcTime(uint8_t h, uint8_t m, uint8_t s);
void GetRtcTime(uint8_t *h, uint8_t *m, uint8_t *s);
void SetRtcDate(uint8_t y, uint8_t m, uint8_t d);
void GetRtcDate(uint8_t *y, uint8_t *m, uint8_t *d);
void SetRtcWeek(uint8_t w);
void GetRtcWeek(uint8_t *w);
void GetRtcDateTime(uint8_t *year, uint8_t *mon, uint8_t *day, uint8_t *hour, uint8_t *min, uint8_t *sec);


#endif


#endif