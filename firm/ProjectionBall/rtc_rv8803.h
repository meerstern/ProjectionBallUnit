/*
 *  	rtc_rv8803.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef RTC_RV8803_H
#define RTC_RV8803_H

#include <stdint.h>
#include "ProjectionBall.h"

#ifdef USE_RTC_RV8803






#define RV8803C7_ADDR       (0x32<<0)

#define RV8803C7_REG_SEC         0x00
#define RV8803C7_REG_MIN         0x01
#define RV8803C7_REG_HOUR        0x02
#define RV8803C7_REG_WEEK        0x03
#define RV8803C7_REG_DATE        0x04
#define RV8803C7_REG_MNTH        0x05
#define RV8803C7_REG_YEAR        0x06
#define RV8803C7_REG_RAM_1       0x07    //MODE
#define RV8803C7_REG_RAM_2       0x08    //PATTERN  
#define RV8803C7_REG_RAM_3       0x09    //
#define RV8803C7_REG_RAM_4       0x0A    //
#define RV8803C7_REG_RAM_5       0x0B    //
#define RV8803C7_REG_RAM_6       0x0C    //
#define RV8803C7_REG_FLG         0x0E
//#define RV8803C7_REG_RAM_7       0x18
//#define RV8803C7_REG_RAM_8       0x19
//#define RV8803C7_REG_RAM_9       0x1A
//#define RV8803C7_REG_RAM_10      0x1B
//#define RV8803C7_REG_RAM_11      0x1C

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
bool IsRtcBOR();



#endif














#endif