/*
 *  	path_const.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef PATH_CONST_H
#define PATH_CONST_H

#define TRIG_FUNCTION_LEN   120

int32_t CalcCosVal(uint16_t deg, int32_t val);
int32_t CalcSinVal(uint16_t deg, int32_t val);

void GetStarPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetArrowPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetMailPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetSmilePath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetSunPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetCloudPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetRainPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetSnowPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetThunderPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
void GetHeartPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);

#endif