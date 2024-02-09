/*
 *  	path_ctrl.c
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/rtc.h"
#include "hardware/watchdog.h"
#include "ProjectionBall.h"
#include "motor_ctrl.h"
#include "path_ctrl.h"
#include "path_const.h"
#include "path_font.h"
#include "rtc_rv8803.h"
#include "rtc_sd30XX.h"
#include "flash_ctrl.h"

#define REPEAT_NUM  5
#define IN_POS_MAX  48
#define IN_POS_MIN  -IN_POS_MAX


#define ONE_STROKE_MODE_SPEED   5
#define ONE_STROKE_MODE_WAIT    30

typedef struct SettingData
{
    int16_t     pauseTime;
    int16_t     resumeTime;
    char        userString[USER_STRING_LEN];
}SettingData_t;

typedef struct CalibrationData
{
    int32_t     x_cen0;
    int32_t     x_cen1;
}CalibrationData_t;

static SettingData_t settingData;
static CalibrationData_t calibrationData;


static uint8_t SelectPattern = PATTERN_STAR;
static uint8_t SelectMode = MODE_PATTERN_ALWAYS_ON;
static int32_t laser_stroke_cnt = 0;
static int32_t rotate_deg = 0;
static int32_t step = 0;
static int32_t allstep = 0;
static int32_t repeat_cnt = 0;
static uint8_t string_cnt = 0;
static bool	   enablePauseResume = false;
static int32_t x_cen0, x_cen1;
static int16_t resumeTime;
static int16_t pauseTime;

static char PathString[USER_STRING_LEN]={'\0'};
static char SelString[USER_STRING_LEN]={'\0'};
static char TimeString[8]={'\0'};
static uint8_t date_y=0;
static uint8_t date_m=0;
static uint8_t date_d=0;
static uint8_t time_h=0;
static uint8_t time_m=0;
static uint8_t time_s=0;

static uint8_t  strShift = 0;
static uint8_t  strShiftOld = 0;
static uint8_t  allStrShiftFlg = 0;
static uint8_t  strLength = 0;

static bool pattern_sw_old = 1;
static bool mode_sw_old = 1;
static bool pattern_sw = 1;
static bool mode_sw = 1;
static bool pattern_update = false;
static bool mode_update = false;

inline static void checkUserButton();
inline static void getPatternPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
inline static void getStringPath(char *str, uint8_t *strcnt, int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr);
inline static void getDateString(char *str);
inline static void getTimeString(char *str);
inline static void updateDateTime();


void __time_critical_func(GetPathCmd)(int32_t *cmd0, int32_t *cmd1, bool *laser)
{
    int32_t x0 = 0, x1 = 0;

    bool lsr = false;

    switch(SelectMode)
    {
        case MODE_PATTERN_ALWAYS_ON:
            getPatternPath(&step, &allstep, &x0, &x1, &lsr);
            break;

        case MODE_PATTERN_ONE_STROKE:
            getPatternPath(&step, &allstep, &x0, &x1, &lsr);
            if( lsr == 1 && laser_stroke_cnt <= step )
                lsr = 0;            
            break;

        case MODE_PATTERN_ROTATION:
            getPatternPath(&step, &allstep, &x0, &x1, &lsr);
            x0 = CalcCosVal(rotate_deg, x0);
            break;

        case MODE_WATCH:
            getTimeString(TimeString);
            getStringPath(TimeString, &string_cnt, &step, &allstep, &x0, &x1, &lsr);
            break;

        case MODE_DATE:
            getDateString(TimeString);
            getStringPath(TimeString, &string_cnt, &step, &allstep, &x0, &x1, &lsr);
            break;

        case MODE_MSG:
            if(strlen(PathString)>MAX_STRING_LEN) // Auto Scroll
            {
                if( strShift != strShiftOld )
                {
                    string_cnt = 0; 
                    step = 0;
                    strShiftOld = strShift;
                }
                getStringPath(SelString, &string_cnt, &step, &allstep, &x0, &x1, &lsr);   
            }
            else // Projection at once
            {
                getStringPath(PathString, &string_cnt, &step, &allstep, &x0, &x1, &lsr);   
            }                     
            break;

        default:
            break;
    }

#ifdef ENABLE_SIN_CMD
	x0 = CalcSinVal(rotate_deg,SIN_CMD_AMP);
	x1 = CalcSinVal(rotate_deg,SIN_CMD_AMP);
#endif 

    *cmd0 = x0;
    *cmd1 = x1;
    *laser = lsr;

    //Check In Pos
#ifdef ENABLE_IN_POS
    if( motorControl[0].x_err > IN_POS_MIN && motorControl[0].x_err <IN_POS_MAX 
    &&  motorControl[1].x_err > IN_POS_MIN && motorControl[1].x_err <IN_POS_MAX )
        repeat_cnt++;
#else
    repeat_cnt++;
#endif

    if(repeat_cnt>=REPEAT_NUM)
    {
        step++;
        repeat_cnt = 0;
    }

    if( SelectMode == MODE_WATCH || SelectMode == MODE_DATE || SelectMode == MODE_MSG)
    {
        if(CheckNextStrFlg()==true)
            repeat_cnt = -8;
        else if(CheckFirstStrFlg()==true)
            repeat_cnt = -16;
    }
}

void PathCtrlLoop()
{
    checkUserButton();
    updateDateTime();
        
    switch(SelectMode)
    {
        case MODE_PATTERN_ONE_STROKE:
            laser_stroke_cnt = laser_stroke_cnt + ONE_STROKE_MODE_SPEED;
            if( laser_stroke_cnt > (allstep + ONE_STROKE_MODE_WAIT) )
                laser_stroke_cnt = 0;
            break;

        case MODE_PATTERN_ROTATION:        
            rotate_deg = rotate_deg + 3;
            if(rotate_deg>=TRIG_FUNCTION_LEN)
                rotate_deg = 0;
            break;

        case MODE_MSG:
            memset(SelString, '\0', USER_STRING_LEN);
            strLength = strlen(PathString);            
            if(strlen(PathString)>MAX_STRING_LEN)
            {
                memset(SelString, ' ', MAX_STRING_LEN);
                if( 0<= strShift && strShift < MAX_STRING_LEN)
                {
                    strncpy(	&SelString[MAX_STRING_LEN-strShift-1],
                                &PathString[0],
                                strShift+1);
                }
                else if( MAX_STRING_LEN <= strShift && strShift < (strLength+1) )
                {
                    strncpy(	SelString,
                                &PathString[strShift-MAX_STRING_LEN+1],
                                MAX_STRING_LEN);
                }
                else if( (strLength+1) <= strShift && strShift <= (MAX_STRING_LEN+strLength-1) )
                {
                    strncpy(	SelString,
                                &PathString[strShift-MAX_STRING_LEN+1],
                                strLength+MAX_STRING_LEN-strShift-1);
                }

                strShift++;
                if( (strLength+MAX_STRING_LEN-1)<strShift)
                    strShift = 0;
                
            }

            break;

        default:
#ifdef ENABLE_SIN_CMD
            rotate_deg = rotate_deg + 6;
            if(rotate_deg>=TRIG_FUNCTION_LEN)
                rotate_deg = 0;
#endif
            break;
    }    
}

void DebugPathCtrl()
{
    //printf("deg: %03d,\t",rotate_deg);
    //printf("stp: %03d/%03d:%d:%d\t", step, allstep, repeat_cnt, string_cnt);
    //printf("SW:%d, %d, ",SelectPattern,SelectMode);

    int  userInput = getchar_timeout_us(0);
    //char buf[BUFSIZ]={'\0'};
    //fgets(buf, sizeof(int), stdin); 
    if(userInput!=PICO_ERROR_TIMEOUT)
        printf("(%c)",userInput);
    
}

void SetUserString(char *str)
{
    strcpy(PathString, str);
}

void GetUserString(char *str)
{
    strcpy(str, PathString);
}


void RestoreUserData()
{
   
    //Restore User Data from Flash
    RestoreFlashUserData((uint8_t*)&settingData, sizeof(settingData));
    RestoreFlashCalibData((uint8_t*)&calibrationData, sizeof(calibrationData));
    x_cen0 = calibrationData.x_cen0;
    x_cen1 = calibrationData.x_cen1;
    resumeTime = settingData.resumeTime;
    pauseTime = settingData.pauseTime;
    MotorCtrSetCenterPos(x_cen0, x_cen1);
    //printf("Calibration value X:%d, Y:%d \r\n", settingData.x_cen0, settingData.x_cen1);

    if(strlen(settingData.userString)>0 && settingData.userString[0]!=0xFF)
        strcpy(PathString, settingData.userString);
    else
        strcpy(PathString, "Hello");  

    //Pattern & Mode Data
    uint8_t udeg = 0;
    int32_t deg = 0;
    GetRtcRam(MODE_SRAM, &SelectMode);
    sleep_ms(1);		  
    GetRtcRam(PATTERN_SRAM, &SelectPattern);
    sleep_ms(1);

    if( SelectMode >= MODE_NUM || SelectPattern >= PATTERN_NUM ) //RTC After Reset
    {
        SelectMode = 0;
        SelectPattern = 0;
        enablePauseResume = false;
        SetRtcRam(MODE_SRAM, SelectMode);
        sleep_ms(1);
        SetRtcRam(PATTERN_SRAM, SelectPattern);  
        sleep_ms(1);      
        SetRtcRam(TIMER_STAT_SRAM, enablePauseResume);
        sleep_ms(1);
        SetProjectionAngle(0);
    }

    GetRtcRam(TIMER_STAT_SRAM, (uint8_t*)&enablePauseResume); 
    sleep_ms(1);	
    GetRtcRam(PRJ_DEG_SRAM, (uint8_t*)&udeg);
    deg = udeg*((int)360/TRIG_FUNCTION_LEN);
    SetProjectionAngle(deg); 
}

void SaveUserData()
{
    //Save User Data to Flash
    strcpy(settingData.userString, PathString);
   
    calibrationData.x_cen0 = x_cen0;
    calibrationData.x_cen1 = x_cen1;
    settingData.pauseTime = pauseTime;
    settingData.resumeTime = resumeTime;
    watchdog_enable(500, 1);
    multicore_reset_core1();  
    StoreFlashUserData((uint8_t*)&settingData, sizeof(settingData));
    StoreFlashCalibData((uint8_t*)&calibrationData, sizeof(calibrationData));
}

void UpdateHwRtc()
{
	datetime_t dt = {0, 0, 0, 0, 0, 0, 0};
    GetRtcDateTime( (uint8_t*)&dt.year, (uint8_t*)&dt.month, (uint8_t*)&dt.day, (uint8_t*)&dt.hour, (uint8_t*)&dt.min, (uint8_t*)&dt.sec);
    dt.year = dt.year + 2000;
	rtc_set_datetime(&dt);
}

void GetSettingCenterData(int32_t *xpos, int32_t *ypos)
{
    *xpos = x_cen0;
    *ypos = x_cen1;
}

void SetSettingCenterData(int32_t xpos, int32_t ypos)
{
    x_cen0 = xpos;
    x_cen1 = ypos;
}

void SetResumeTime(uint16_t time)
{
   resumeTime = time;
}

void GetResumeTime(uint16_t *time)
{
    *time = resumeTime;
}


void SetPauseTime(uint16_t time)
{
    pauseTime = time;
}

void GetPauseTime(uint16_t *time)
{
    *time = pauseTime;
}

void TimerEnable()
{
    enablePauseResume = true;
    SetRtcRam(TIMER_STAT_SRAM, enablePauseResume);
}

void TimerDisable()
{
    enablePauseResume = false;
    SetRtcRam(TIMER_STAT_SRAM, enablePauseResume);
}

bool GetTimerStatus()
{
    return enablePauseResume;
}

void SetMode(uint8_t mode)
{
    uint8_t md = mode;
    if(md>=MODE_NUM)
            md = 0;
    SelectMode = md;
    SetRtcRam(MODE_SRAM, md);
}

void GetMode(uint8_t *mode)
{
    *mode = SelectMode;
}

void SetPattern(uint8_t pattern)
{
    uint8_t pt = pattern;
    if(pt>=PATTERN_NUM)
        pt = 0;
    SelectPattern = pt;
    SetRtcRam(PATTERN_SRAM, pt);
}

void GetPattern(uint8_t *pattern)
{
    *pattern = SelectPattern;
}



void UpdateUserButton()
{
    pattern_sw = gpio_get(PIN_PATTERN);
    mode_sw = gpio_get(PIN_MODE);
    if(pattern_sw == 1 && pattern_sw_old==0 )
        pattern_update = true;

    if( mode_sw == 1 && mode_sw_old ==0 )
        mode_update = true;

    pattern_sw_old = pattern_sw;
    mode_sw_old = mode_sw;
}

inline static void checkUserButton()
{
    if(pattern_update)
    {        
         SelectPattern++;
        if(SelectPattern>=PATTERN_NUM)
            SelectPattern = 0;
        SetRtcRam(PATTERN_SRAM, SelectPattern);
        pattern_update = false;
    }

    if( mode_update )
    {   
        SelectMode++;     
        if(SelectMode>=MODE_NUM)
            SelectMode = 0;
        SetRtcRam(MODE_SRAM, SelectMode);
        mode_update = false;
    }

}

inline static void updateDateTime()
{
    datetime_t dt = {0, 0, 0, 0, 0, 0, 0};
    rtc_get_datetime(&dt);
    
    if(dt.min == 0 && dt.sec == 0)//Update HW RTC every hour
    {
        UpdateHwRtc();
        rtc_get_datetime(&dt);
    }

    date_y = dt.year%2000;
    date_m = dt.month;
    date_d = dt.day;
    time_h = dt.hour;
    time_m = dt.min;
    time_s = dt.sec;


    //Check Timer Trigger
    if( enablePauseResume == true )
    {
        int16_t nowTime = time_h*100 + time_m;
                
        if( pauseTime< resumeTime ) //ex pause 15:00, resume 18:00
        {
            if( pauseTime < nowTime && nowTime < resumeTime )
                SetPause(true);
            else
                SetPause(false);
        }
        else if( pauseTime > resumeTime ) //ex pause 22:00, resume 8:00
        {
            if( pauseTime < nowTime || nowTime < resumeTime )
                SetPause(true);
            else
                SetPause(false);
        }
    }

}

inline static void getPatternPath(int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr)
{

    switch(SelectPattern)
    {
        case PATTERN_STAR:
            GetStarPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_ARROW:
            GetArrowPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_MAIL:
            GetMailPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_SMILE:
            GetSmilePath(step, allstep, x, y, lsr);
            break;
        case PATTERN_SUN:
            GetSunPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_CLOUD:
            GetCloudPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_RAIN:
            GetRainPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_SNOW:
            GetSnowPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_THUNDER:
            GetThunderPath(step, allstep, x, y, lsr);
            break;
        case PATTERN_HEART:
            GetHeartPath(step, allstep, x, y, lsr);
            break;
        default:
            break;
    }
}

inline static void getStringPath(char *str, uint8_t *strcnt, int32_t *step, int32_t *allstep, int32_t *x, int32_t *y, bool *lsr)
{
    char targetString = *(str+*strcnt);

    if( '0'<=targetString  && targetString <='9' )
    {
        GetNumberPath(str, strcnt, step, x, y, lsr);
    }
    else if( 'A' <= targetString && targetString<='Z')
    {
        GetUpperLetterPath(str, strcnt, step, x, y, lsr);
    }
    else if( 'a' <= targetString && targetString<='z')
    {
        GetLowerLetterPath(str, strcnt, step, x, y, lsr);
    }
    else
    {
        GetMarkPath(str, strcnt, step, x, y, lsr);
    }

    *x = -*x + GetStringOffset(strlen(str), *strcnt);
}



inline static void getDateString(char *str)
{
    //Date Set
    str[0]='0'+(int8_t)date_m/10;
    str[1]='0'+(int8_t)date_m%10;
    str[2]='/';
    str[3]='0'+(int8_t)date_d/10;
    str[4]='0'+(int8_t)date_d%10;
    str[5]='\0';

}

inline static void getTimeString(char *str)
{    
    //Time Set
    str[0]='0'+(int8_t)time_h/10;
    str[1]='0'+(int8_t)time_h%10;
    str[2]=':';
    str[3]='0'+(int8_t)time_m/10;
    str[4]='0'+(int8_t)time_m%10;
    str[5]='\0';

}
