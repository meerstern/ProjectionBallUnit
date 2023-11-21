/*
 *  	flash_ctrl.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef FLASH_CTRL_H
#define FLASH_CTRL_H



void StoreFlashUserData(uint8_t *data, size_t size);
void StoreFlashCalibData(uint8_t *data, size_t size);
void RestoreFlashUserData(uint8_t *data, size_t size);
void RestoreFlashCalibData(uint8_t *data, size_t size);
void TestFlashReadWrite();


#endif