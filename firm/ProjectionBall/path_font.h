/*
 *  	path_font.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef PATH_FONT_H
#define PATH_FONT_H


void GetNumberPath(char *str, uint8_t *strcnt, int32_t *step, int32_t *x, int32_t *y, bool *lsr);
void GetUpperLetterPath(char *str, uint8_t *strcnt, int32_t *step, int32_t *x, int32_t *y, bool *lsr);
void GetLowerLetterPath(char *str, uint8_t *strcnt, int32_t *step, int32_t *x, int32_t *y, bool *lsr);
void GetMarkPath(char *str, uint8_t *strcnt, int32_t *step, int32_t *x, int32_t *y, bool *lsr);
int32_t GetStringOffset(int32_t stringLen, int32_t stringCount);
bool CheckNextStrFlg();
bool CheckFirstStrFlg();

#endif