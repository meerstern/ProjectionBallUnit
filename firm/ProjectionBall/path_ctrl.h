/*
 *  	path_ctrl.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef VECTOR_CTRL_H
#define VECTOR_CTRL_H


#define USER_STRING_LEN     40
#define MAX_STRING_LEN      5//Max String Length of projection at once

#ifdef USE_RTC_RV8803
#define MODE_SRAM           RV8803C7_REG_RAM_1
#define PATTERN_SRAM        RV8803C7_REG_RAM_2
#define TIMER_STAT_SRAM     RV8803C7_REG_RAM_3
#define PRJ_DEG_SRAM        RV8803C7_REG_RAM_4
#endif

#ifdef USE_RTC_SD30XX
#define MODE_SRAM           SD30XX_REG_RAM_1
#define PATTERN_SRAM        SD30XX_REG_RAM_2
#define TIMER_STAT_SRAM     SD30XX_REG_RAM_3
#define PRJ_DEG_SRAM        SD30XX_REG_RAM_4
#endif

enum
{
    PATTERN_STAR,   //0
    PATTERN_ARROW,  //1
    PATTERN_MAIL,   //2
    PATTERN_SMILE,  //3
    PATTERN_SUN,    //4
    PATTERN_CLOUD,  //5
    PATTERN_RAIN,   //6
    PATTERN_SNOW,   //7
    PATTERN_THUNDER,//8
    PATTERN_HEART,  //9
    PATTERN_NUM     //10
};

enum
{
    MODE_PATTERN_ALWAYS_ON,
    MODE_PATTERN_ONE_STROKE,
    MODE_PATTERN_ROTATION,
    MODE_WATCH,
    MODE_DATE,
    MODE_MSG,
    MODE_NUM
};


void GetPathCmd(int32_t *cmd0, int32_t *cmd1, bool *laser);
void PathCtrlLoop();
void DebugPathCtrl();
void RestoreUserData();
void SaveUserData();
void UpdateHwRtc();
void GetSettingCenterData(int32_t *xpos, int32_t *ypos);
void SetSettingCenterData(int32_t xpos, int32_t ypos);
void SetUserString(char *str);
void GetUserString(char *str);
void SetResumeTime(uint16_t time);
void GetResumeTime(uint16_t *time);
void SetPauseTime(uint16_t time);
void GetPauseTime(uint16_t *time);

void SetMode(uint8_t mode);
void GetMode(uint8_t *mode);
void SetPattern(uint8_t pattern);
void GetPattern(uint8_t *pattern);

void TimerEnable();
void TimerDisable();
bool GetTimerStatus();
void UpdateUserButton();


#endif