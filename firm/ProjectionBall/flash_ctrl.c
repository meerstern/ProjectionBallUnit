/*
 *  	flash_ctrl.c
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
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/flash.h"
#include "ProjectionBall.h"
#include "flash_ctrl.h"


//W25Q16JV Flash Block
#define FLASH_BASE_ADDR         XIP_BASE
#define FLASH_BLOCK31_ADDR      0x1F0000
#define FLASH_BLOCK30_ADDR      0x1E0000
#define FLASH_BLOCK29_ADDR      0x1D0000
#define FLASH_BLOCK28_ADDR      0x1C0000
#define FLASH_BLOCK27_ADDR      0x1B0000
#define FLASH_BLOCK26_ADDR      0x1A0000
#define FLASH_BLOCK25_ADDR      0x190000
#define FLASH_BLOCK24_ADDR      0x180000
#define FLASH_BLOCK23_ADDR      0x170000
#define FLASH_BLOCK22_ADDR      0x160000
#define FLASH_BLOCK21_ADDR      0x150000

#define FLASH_BLOCK_UNIT        0x10000 
#define FLASH_WRITE_UNIT        FLASH_PAGE_SIZE     //256byte
#define FLASH_ERASE_UNIT        FLASH_SECTOR_SIZE   //4096byte

static uint8_t buff[FLASH_WRITE_UNIT];

static void flashBlockErase(uint32_t blockAddr);
static void flashBlockWrite(uint32_t blockAddr, uint8_t *wData);
static void flashBlockRead(uint32_t blockAddr, uint8_t *rData, size_t rSize);

void StoreFlashUserData(uint8_t *data, size_t size)
{
    size_t wsize = (size>FLASH_WRITE_UNIT)?FLASH_WRITE_UNIT:size;
    memcpy(buff, 0, FLASH_WRITE_UNIT);
    memcpy(buff, data, wsize);
    flashBlockErase(FLASH_BLOCK30_ADDR);
    flashBlockWrite(FLASH_BLOCK30_ADDR, buff);
}

void StoreFlashCalibData(uint8_t *data, size_t size)
{
    size_t wsize = (size>FLASH_WRITE_UNIT)?FLASH_WRITE_UNIT:size;
    memcpy(buff, 0, FLASH_WRITE_UNIT);
    memcpy(buff, data, wsize);
    flashBlockErase(FLASH_BLOCK31_ADDR);
    flashBlockWrite(FLASH_BLOCK31_ADDR, buff);
}

void RestoreFlashUserData(uint8_t *data, size_t size)
{
    size_t rsize = (size>FLASH_WRITE_UNIT)?FLASH_WRITE_UNIT:size;
    memcpy(buff, 0, FLASH_WRITE_UNIT);
    flashBlockRead(FLASH_BLOCK30_ADDR, buff, rsize);
    memcpy(data, buff, rsize);
}

void RestoreFlashCalibData(uint8_t *data, size_t size)
{
    size_t rsize = (size>FLASH_WRITE_UNIT)?FLASH_WRITE_UNIT:size;
    memcpy(buff, 0, FLASH_WRITE_UNIT);
    flashBlockRead(FLASH_BLOCK31_ADDR, buff, rsize);
    memcpy(data, buff, rsize);
}

void TestFlashReadWrite()
{
	uint8_t wData[8]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
	uint8_t rData[8]={0x00};
	sleep_ms(2000);
	//StoreFlashData(wData, sizeof(wData));
	RestoreFlashUserData(rData, sizeof(rData));

	printf("w:%X %X %X %X %X %X %X \r\n", 
			wData[0], wData[1], wData[2], wData[3], wData[4], wData[5], wData[6], wData[7]);
	printf("r:%X %X %X %X %X %X %X \r\n",
			rData[0], rData[1], rData[2], rData[3], rData[4], rData[5], rData[6], rData[7]);
	while(1);

}


static void flashBlockErase(uint32_t blockAddr)
{
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(blockAddr, FLASH_ERASE_UNIT);
    restore_interrupts(interrupts);       
}

static void flashBlockWrite(uint32_t blockAddr, uint8_t *wData)
{
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(blockAddr, wData, FLASH_WRITE_UNIT);
    restore_interrupts(interrupts);   
}

static void flashBlockRead(uint32_t blockAddr, uint8_t *rData, size_t rSize)
{
    const uint8_t *data = (const uint8_t *) (FLASH_BASE_ADDR + blockAddr);  

    for(size_t i=0; i < rSize; i++)
    {
        *(rData + i) = *(data + i);
    }
}