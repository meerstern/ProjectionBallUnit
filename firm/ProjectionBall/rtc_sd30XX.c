/*
 *  	rtc_sd30XX.c
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ProjectionBall.h"
#include "rtc_sd30XX.h"

#ifdef USE_RTC_SD30XX

#define HEX2DEC(X)  (X/16*10+X%16)
#define DEC2HEX(X)  (X/10*16+X%10)
#define I2C_PORT i2c0


static void writeEnable();
static void writeDisable();


void GetDateTime()
{
    uint8_t rdat[7];
    uint8_t wdat[1]={0};
    uint8_t sec,min,hour,week,day,month,year;
    char weekStr[5]={'\0'};

	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
	i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 7, false);
    
    sec = rdat[0];
    min = rdat[1];
    hour = rdat[2]&0x7F;
    week = rdat[3];
    day = rdat[4];
    month = rdat[5];
    year = rdat[6];
    printf( "RTC Time:%02X/%02X/%02X %02X:%02X:%02X\r\n",
        year, month, day, hour, min, sec); 
    return;
    
    switch(week)
    {
        case 1:
            strcpy(weekStr,"sun");
            break;
        case 2:
            strcpy(weekStr,"mon");
            break;
        case 4:
            strcpy(weekStr,"tue");
            break;
        case 8:
            strcpy(weekStr,"wed");
            break; 
        case 16:
            strcpy(weekStr,"thu");
            break;
        case 32:
            strcpy(weekStr,"fri");
            break; 
        case 64:
            strcpy(weekStr,"sat");
            break; 
        default:
            break;
    }
    
    printf( "20%02X/%02X/%02X (%s) %02X:%02X:%02X\r\n",
        year, month, day, weekStr, hour, min, sec); 

}


void SetRtcRam(uint8_t addr, uint8_t data)
{
    uint8_t wdat[2]={0};
    wdat[0]=addr;
    wdat[1]=data;
    writeEnable();
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 2, false);
    writeDisable();
}

void GetRtcRam(uint8_t addr, uint8_t *data)
{
    uint8_t rdat[1]={0};
    uint8_t wdat[1]={0};
    wdat[0]=addr;

	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
	i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 1, false);
    *data=rdat[0];
}

void SetRtcTime(uint8_t h, uint8_t m, uint8_t s)
{

    h=DEC2HEX(h);
    m=DEC2HEX(m);
    s=DEC2HEX(s);

    if(h>(0x24-1) || m>(0x60-1) || s>(0x60-1))
    {
        //SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SET TIME Format Err\r\n"); 
        return;
    }            

    printf( "SET TIME:%02X:%02X:%02X\r\n",h, m, s); 

    uint8_t wdat[4]={0};

    wdat[0] = SD30XX_REG_SEC;
    wdat[1] = s;
    wdat[2] = m;
    wdat[3] = h|0x80;
    writeEnable();
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 4, false);
    writeDisable();    
}

void GetRtcTime(uint8_t *h, uint8_t *m, uint8_t *s)
{
    uint8_t rdat[7]={0};
    uint8_t wdat[1]={0};
    
    wdat[0]=SD30XX_REG_SEC;
	
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
	i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 7, false);
	
    rdat[2]=rdat[2]&0x7F;

    *s=HEX2DEC(rdat[0]);
    *m=HEX2DEC(rdat[1]);
    *h=HEX2DEC(rdat[2]);
    
    if(*s==59)
    {
		i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
		i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 7, false);
        *s=HEX2DEC(rdat[0]);
        if(*s!=59)
        {
            rdat[2]=rdat[2]&0x7F;
            //*s=HEX2DEC(rdat[0]);
            *m=HEX2DEC(rdat[1]);
            *h=HEX2DEC(rdat[2]);
        }
    }
}

void SetRtcDate(uint8_t y, uint8_t m, uint8_t d)
{
        
    y=DEC2HEX(y);
    m=DEC2HEX(m);
    d=DEC2HEX(d);

    if(y>(0x99) || m>(0x12) || d>(0x31))
    {
        //SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, "SET DATE Format Err\r\n"); 
        return;
    }            

    printf( "SET DATE:20%02X/%02X/%02X\r\n",y, m, d);        

    uint8_t wdat[4]={0};
    wdat[0]=SD30XX_REG_DATE;
    wdat[1]=d;
    wdat[2]=m;
    wdat[3]=y;
    writeEnable();
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 4, false); 
    writeDisable();
}

void GetRtcDate(uint8_t *y, uint8_t *m, uint8_t *d)
{
    uint8_t rdat[3]={0};
    uint8_t wdat[1]={0};
    wdat[0]=SD30XX_REG_DATE;
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
	i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 3, false);
    *d=HEX2DEC(rdat[0]);
    *m=HEX2DEC(rdat[1]);
    *y=HEX2DEC(rdat[2]);
}

void GetRtcDateTime(uint8_t *year, uint8_t *mon, uint8_t *day, uint8_t *hour, uint8_t *min, uint8_t *sec)
{
    uint8_t rdat[7]={0};
    uint8_t wdat[1]={0};
    wdat[0]=SD30XX_REG_SEC;
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
	i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 7, false);
    rdat[2]=rdat[2]&0x7F;
    *sec=HEX2DEC(rdat[0]);
    *min=HEX2DEC(rdat[1]);
    *hour=HEX2DEC(rdat[2]);
    //*wek=HEX2DEC(rdat[3]);
    *day=HEX2DEC(rdat[4]);
    *mon=HEX2DEC(rdat[5]);
    *year=HEX2DEC(rdat[6]);
    
    if(*sec==59)
    {
		i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
		i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 7, false);
        *sec=HEX2DEC(rdat[0]);
        if(*sec!=59)
        {
            rdat[2]=rdat[2]&0x7F;
            //*sec=HEX2DEC(rdat[0]);
            *min=HEX2DEC(rdat[1]);
            *hour=HEX2DEC(rdat[2]);
            //*wek=HEX2DEC(rdat[3]);
            *day=HEX2DEC(rdat[4]);
            *mon=HEX2DEC(rdat[5]);
            *year=HEX2DEC(rdat[6]);  
        }
    }
}

void SetRtcWeek(uint8_t w)
{
    uint8_t wdat[2]={0};
    wdat[0]=SD30XX_REG_WEEK;
    wdat[1]=pow(2,w);
    writeEnable();
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 2, false); 
    writeDisable();
}

void GetRtcWeek(uint8_t *w)
{
    uint8_t rdat[1]={0};
    uint8_t wdat[1]={0};
    wdat[0]=SD30XX_REG_WEEK;
	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, wdat, 1, true); 
	i2c_read_blocking(I2C_PORT, SD30XX_ADDR, rdat, 1, false);
    
    switch(rdat[0])
    {
        case 1://sun
            *w=0;
            break;
        case 2://mon
            *w=1;
            break;
        case 4://tue
            *w=2;
            break;
        case 8://wed
            *w=3;
            break;
        case 16://thu
            *w=4;
            break;
        case 32://fri
            *w=5;
            break;
        case 64:
            *w=6;
            break;
        default:
            *w=0;
            break;
    }
}

static void writeEnable()
{
    uint8_t en1[2] =    {SD30XX_REG_CTR1, 0xFF};
    uint8_t en23[2] =   {SD30XX_REG_CTR2, 0x80};

	i2c_write_blocking(I2C_PORT, SD30XX_ADDR, en23, 2, false);    
    i2c_write_blocking(I2C_PORT, SD30XX_ADDR, en1, 2, false);
}

static void writeDisable()
{
    uint8_t dis1[2] =   {SD30XX_REG_CTR1, 0x7B};
    uint8_t dis23[2] =  {SD30XX_REG_CTR2, 0x12};

    i2c_write_blocking(I2C_PORT, SD30XX_ADDR, dis1, 2, false);
    i2c_write_blocking(I2C_PORT, SD30XX_ADDR, dis23, 2, false);
}
#endif
