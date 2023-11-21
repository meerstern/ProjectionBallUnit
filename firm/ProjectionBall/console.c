/*
 *  	console.c
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/watchdog.h"
#include "ProjectionBall.h"
#include "motor_ctrl.h"
#include "path_ctrl.h"
#include "flash_ctrl.h"
#include "console.h"
#include "rtc_rv8803.h"
#include "rtc_sd30XX.h"



/* DateTime */
#define STR_SET_TIME            "tim="
#define STR_GET_TIME            "tim?"
#define STR_SET_DATE            "day="
#define STR_GET_DATE            "day?"
#define STR_SET_WEEK            "wek="
#define STR_GET_WEEK            "wek?"

/* Control Command */
#define STR_HELP                "help"
#define STR_SET_RESET           "rst!"
#define STR_SET_PAUSE           "pus!"
#define STR_SET_RESUME          "rsm!"
#define STR_SET_DEG             "deg="
#define STR_GET_DEG             "deg?"
#define STR_SET_MODE            "mod="
#define STR_GET_MODE            "mod?"
#define STR_SET_PATTERN         "ptn="
#define STR_GET_PATTERN         "ptn?"



/* Calibration */
#define STR_SET_CENTER_POS      "cen="
#define STR_GET_CENTER_POS      "cen?"
#define SPLIT_STRING        	"=,"

/* Debug */
#define STR_SET_GAIN            "gai="
#define STR_GET_GAIN            "gai?"

/* User Setting */
#define STR_SET_USER_STRING     "str="
#define STR_GET_USER_STRING     "str?"
#define STR_SET_USER_STRING_RAM "stg="
#define STR_GET_USER_STRING_RAM "stg?"
#define STR_SET_RESUME_TIME     "rsm="
#define STR_GET_RESUME_TIME     "rsm?"
#define STR_SET_PAUSE_TIME      "pus="
#define STR_GET_PAUSE_TIME      "pus?"
#define STR_SET_TIMER_ENABLE    "ten!"
#define STR_SET_TIMER_DISABLE   "tds!"
#define STR_GET_TIMER_STATUS    "tst?"


#define BUFFER_LENGTH   256
static char	                buffer[BUFFER_LENGTH];
static char	                uartBuffer[BUFFER_LENGTH];
volatile static uint32_t 	buffer_index = 0;
volatile static uint32_t 	uartBuffer_index = 0;
volatile static bool        hasUartData = false;
volatile static bool        rstEnable = false;

static void consoleCheckDateTimeCommand();
static void consoleCheckCalibrationCommand();
static void consoleCheckUserSettingCommand();
static void consoleCheckControlCommand();
static void consoleCheckTimerCommand();
static void consoleCheckGainCommand();
static void consoleDispHelp();
static void softReset();

void ConsoleGetString()
{
    int c='\0';
    while (true)
    {
        c = getchar_timeout_us(100);

        if(c != PICO_ERROR_TIMEOUT && (c =='\n' ||  c=='\r'))
            break;
        else if (c != PICO_ERROR_TIMEOUT && buffer_index < BUFFER_LENGTH) 
            buffer[buffer_index++] = (c & 0xFF);
        else
            break;
    }

    if( c =='\n' ||  c=='\r' || hasUartData == true)
    {
        if(hasUartData == true)
            strcpy(buffer,uartBuffer);

        printf("%s\r\n",buffer);
        consoleCheckDateTimeCommand();
        consoleCheckCalibrationCommand();
        consoleCheckUserSettingCommand();
        consoleCheckControlCommand();
        consoleCheckTimerCommand();
        consoleCheckGainCommand();  //For Debug Use
        memset((void*)buffer,'\0',sizeof(buffer));
        memset((void*)uartBuffer,'\0',sizeof(uartBuffer));
        buffer_index=0;
        uartBuffer_index = 0;
        hasUartData = false;
    }
}

void OnUartRx()
{
    while (uart_is_readable(UART_ID))
    {
        uartBuffer[uartBuffer_index]=uart_getc(UART_ID);
        uartBuffer_index++;

        if( uartBuffer[uartBuffer_index-1] == '\n' || uartBuffer[uartBuffer_index-1] == '\r' )
        {
            hasUartData = true;
        }
    }
}

bool IsResetEnable()
{
    return rstEnable;
}

static void consoleCheckDateTimeCommand()
{
    char *cmd = (char*)buffer;       

    // Set Time
    if(strncmp(cmd, STR_SET_TIME, 4) == 0)
    {
        long time=0;
        unsigned int  h,m,s;
        char *p=strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) time = atol( p ); 
        h=(uint8_t)(time/10000);
        m=(uint8_t)((time/100)%100);
        s=(uint8_t)(time%100);
        //SetRtcTime(h,m,s);       
    }
    // Get Time    
    else if(strncmp(cmd, STR_GET_TIME, 4) == 0)
    {
        uint8_t s,m,h;
        GetRtcTime(&h, &m, &s);
        printf("NOW TIME:%02d:%02d:%02d\r\n",h, m, s);    
    }
    // Set Date
    else if(strncmp(cmd, STR_SET_DATE, 4) == 0)
    {
        long date=0;
        unsigned int  y,m,d;
        char *p=strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) date = atol( p ); 
        y=date/10000;
        m=(date/100)%100;
        d=date%100;
        SetRtcDate(y, m, d);
    }
    // Get Date
    else if(strncmp(cmd, STR_GET_DATE, 4) == 0)
    {
        uint8_t d,m,y;
        GetRtcDate(&y, &m, &d);
        printf("NOW DATE:20%02d/%02d/%02d\r\n",y, m, d);  
    } 
    // Set Week
    else if(strncmp(cmd, STR_SET_WEEK, 4) == 0)
    {
        long week=0;
        char *p=strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) week = atol( p ); 
        
        if(0>week && week >6)
        {
            printf("SET DATE Format Err\r\n"); 
            return;
        }            
        SetRtcWeek(week);
    }   
    // Get Week
    else if(strncmp(cmd, STR_GET_WEEK, 4) == 0)
    {
        uint8_t week;
        char weekStr[5]={'\0'};
        
        GetRtcWeek(&week);
        switch(week)
        {
            case 0:
                strcpy(weekStr,"sun");
                break;
            case 1:
                strcpy(weekStr,"mon");
                break;
            case 2:
                strcpy(weekStr,"tue");
                break;
            case 3:
                strcpy(weekStr,"wed");
                break; 
            case 4:
                strcpy(weekStr,"thu");
                break;
            case 5:
                strcpy(weekStr,"fri");
                break; 
            case 6:
                strcpy(weekStr,"sat");
                break; 
            default:
                break;
        }        
        printf("NOW WEEK:(%s)\r\n", weekStr);   
    }

}


static void consoleCheckCalibrationCommand()
{
    char *cmd = (char*)buffer;
    int32_t x_cen0, x_cen1;

    // Set Center
    if(strncmp(cmd, STR_SET_CENTER_POS, 4) == 0)
    {
        char *tok;
        char find_num=0;        
        tok = strtok( cmd, SPLIT_STRING);
        SetPause(true);
        GetSettingCenterData(&x_cen0, &x_cen1);
        printf("**Center Offset X:%d, Y:%d ->",x_cen0,x_cen1);
        while( tok != NULL )
        {
            tok = strtok( NULL, SPLIT_STRING);
            if(find_num==0)
                x_cen0=atoi(tok);
            if(find_num==1)
                x_cen1=atoi(tok);
            find_num++;
        }
        printf(" X:%d, Y:%d **\r\n",x_cen0, x_cen1); 
        SetSettingCenterData(x_cen0, x_cen1);
        SaveUserData();
    }
    // Get Censter
    else if(strncmp(cmd, STR_GET_CENTER_POS, 4) == 0)
    {
        GetSettingCenterData(&x_cen0, &x_cen1);
        printf("**Center Offset X:%d, Y:%d**\r\n",x_cen0,x_cen1);
    }  


}

static void consoleCheckUserSettingCommand()
{
    char *cmd = (char*)buffer;
    char userStr[USER_STRING_LEN]={'\0'};

    // Set User String
    if(strncmp(cmd, STR_SET_USER_STRING, 4) == 0)
    {
        char slen=strlen(cmd)-4;
        SetPause(true);
        if(slen>0)
            strcpy(userStr, &cmd[4]);
        SetUserString(userStr);
        printf("Set User String:%s\r\n",userStr);
        SaveUserData();
    }
    // Get User String
    else if(strncmp(cmd, STR_GET_USER_STRING, 4) == 0)
    {
        GetUserString(userStr);
        printf("Get User String:%s\r\n",userStr);
    }
    // Set User String without Save Flash
    else if(strncmp(cmd, STR_SET_USER_STRING_RAM, 4) == 0)
    {
        char slen=strlen(cmd)-4;
        if(slen>0)
            strcpy(userStr, &cmd[4]);
        SetUserString(userStr);
        printf("Set User String without Save :%s\r\n",userStr);
    }
    // Get User String without Save Flash
    else if(strncmp(cmd, STR_GET_USER_STRING_RAM, 4) == 0)
    {
        GetUserString(userStr);
        printf("Get User String without Save :%s\r\n",userStr);
    }
}


static void consoleCheckControlCommand()
{
    char *cmd = (char*)buffer;

    // Set Reset Command
    if(strncmp(cmd, STR_SET_RESET, 4) == 0)
    {
        printf("Reset Command!\r\n");
        softReset();
    }
    // Set Pause Command
    else if(strncmp(cmd, STR_SET_PAUSE, 4) == 0)
    {
        printf("Pause Command!\r\n");
        SetPause(true);
    }
    // Set Resume Command 
    else if(strncmp(cmd, STR_SET_RESUME, 4) == 0)
    {
        printf("Resume Command!\r\n");
        SetPause(false);
    }
    // Set Mode Command
    else if(strncmp(cmd, STR_SET_MODE, 4) == 0)
    {
        uint8_t md = 0;
        char *p=strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) md = atoi( p ); 
        printf("Set Mode:%d\r\n", md);
        SetMode(md);
    }
    // Get Mode Command
    else if(strncmp(cmd, STR_GET_MODE, 4) == 0)
    {
        uint8_t md = 0;
        GetMode(&md);
        printf("Mode:%d\r\n", md);
    }
    // Set PATTERN Command
    else if(strncmp(cmd, STR_SET_PATTERN, 4) == 0)
    {
        uint8_t pt = 0;
        char *p=strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) pt = atoi( p ); 
        printf("Set Pattern:%d\r\n", pt);
        SetPattern(pt);
    }
    // Get PATTERN Command
    else if(strncmp(cmd, STR_GET_PATTERN, 4) == 0)
    {
        uint8_t pt = 0;
        GetPattern(&pt);
        printf("Pattern:%d\r\n", pt);
    }
#ifdef ENABLE_PROJECTION_ANGLE
    // Set Deg Command 
    else if(strncmp(cmd, STR_SET_DEG, 4) == 0)
    {        
        int32_t deg = 0;
        char *p = strpbrk( cmd, "1234567890" );   

        GetProjectionAngle(&deg);
        printf("Angle : %d degree -> ", deg);

        if ( p != NULL ) deg = atol( p ); 

        if( 0<= deg && deg < 360)
        {
            SetProjectionAngle(deg);
        }
        else
        {
            SetProjectionAngle(0);
        }

        GetProjectionAngle(&deg);

        printf("%d degree \r\n", deg);
    }
    //Get Deg Command
    else if(strncmp(cmd, STR_GET_DEG, 4) == 0)
    { 
        int32_t deg = 0;
        GetProjectionAngle(&deg); 
        printf("Projection Angle: %d degree \r\n", deg);   
    }
#endif
    else if(strncmp(cmd, STR_HELP, 4) == 0)
    {
        consoleDispHelp();
    } 
}

static void consoleCheckTimerCommand()
{
    char *cmd = (char*)buffer;

    // Set Resume Time Command
    if(strncmp(cmd, STR_SET_RESUME_TIME, 4) == 0)
    {        
        int16_t time=0;
        int16_t h,m =0;
        char *p = strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) time = atol( p ); 
        h = time/100;
        m = time%100; 
        if( 0<= h && h < 24 && 0 <= m && m < 60)
        {
            SetPause(true);
            printf("Set Resume Time %d:%d\r\n",h, m);
            SetResumeTime(time);
            SaveUserData();
        }
        else
            printf("Set Resume Time Format Err\r\n");        
    }

    // Get Resume Time Command
    else if(strncmp(cmd, STR_GET_RESUME_TIME, 4) == 0)
    {
        int16_t time=0;
        int16_t h,m =0;
        GetResumeTime(&time);
        h = time/100;
        m = time%100; 
        printf("Get Resume Time %d:%d\r\n",h, m);
    }
    // Set Pause Time Command
    else if(strncmp(cmd, STR_SET_PAUSE_TIME, 4) == 0)
    {
        int16_t time=0;
        int16_t h,m =0;
        char *p = strpbrk( cmd, "1234567890" );        
        if ( p != NULL ) time = atol( p ); 
        h = time/100;
        m = time%100; 
        if( 0<= h && h < 24 && 0 <= m && m < 60)
        {
            SetPause(true);
            printf("Set Pause Time %d:%d\r\n",h, m);
            SetPauseTime(time);
            SaveUserData();
        }
        else
            printf("Set Pause Time Format Err\r\n");    

    }
    // Get Pause Time Command
    else if(strncmp(cmd, STR_GET_PAUSE_TIME, 4) == 0)
    {
        int16_t time=0;
        int16_t h,m =0;
        GetPauseTime(&time);
        h = time/100;
        m = time%100; 
        printf("Get Pause Time %d:%d\r\n",h, m);
    }
    // Enable Timer Command
    else if(strncmp(cmd, STR_SET_TIMER_ENABLE, 4) == 0)
    {
        TimerEnable();
        printf("Pause/Resume Enabled\r\n");
        SetRtcRam(TIMER_STAT_SRAM, 1);  
    }
    // Disable Timer Command
    else if(strncmp(cmd, STR_SET_TIMER_DISABLE, 4) == 0)
    {
        TimerDisable();
        printf("Pause/Resume Disabled\r\n");
        SetRtcRam(TIMER_STAT_SRAM, 0);  
    }
    // Get Timer Status Command
    else if(strncmp(cmd, STR_GET_TIMER_STATUS, 4) == 0)
    {        
        printf("Pause/Resume Status:%d\r\n",GetTimerStatus());
    }
}


static void consoleCheckGainCommand()
{
    char *cmd = (char*)buffer;
    // Set Gain
    if(strncmp(cmd, STR_SET_GAIN, 4) == 0)
    {
        char *tok;
        char find_num=0;
        int32_t kp, kd,iki;
        GetGain(&kp, &kd, &iki);
        tok = strtok( cmd, SPLIT_STRING);
#ifdef ENABLE_I 
        printf("**SetGain KP:%d, KD:%d, IKI:%d ->",kp, kd, iki);
#else
        printf("**SetGain KP:%d, KD:%d ->",kp, kd);
#endif
        while( tok != NULL )
        {
            tok = strtok( NULL, SPLIT_STRING);
            if(find_num==0)
                kp=atoi(tok);
            if(find_num==1)
                kd=atoi(tok);
            if(find_num==2)
                iki=atoi(tok);    
            find_num++;
        }
        if(kp==0)
            kp=KP;
        if(kd==0)
            kd=KD;
        if(iki==0)
            iki=99999;
#ifdef ENABLE_I 
        printf(" KP:%d, KD:%d, IKI:%d **\r\n", kp, kd, iki); 
#else
        printf(" KP:%d, KD:%d **\r\n", kp, kd); 
#endif
        SetGain(kp, kd, iki);
    }
    // Get Gain
    else if(strncmp(cmd, STR_GET_GAIN, 4) == 0)
    {
        int32_t kp, kd,iki;
        GetGain(&kp, &kd, &iki);
        
#ifdef ENABLE_I 
        printf("**Gain KP:%d, KD:%d, IKI:%d **\r\n", kp, kd, iki); 
#else
        printf("**Gain KP:%d, KD:%d **\r\n", kp, kd); 
#endif
    }  
}

static void consoleDispHelp()
{
    printf("\t\t--- Command Help Infomation ---\r\n"); 

    printf("\t\t*** Control Command *** \t \r\n");
    printf("Mode Set Cmd: mod=     \tex) mod=3\r\n"); 
    printf("Mode Get Cmd: mod?     \tex) mod?\r\n"); 
    printf("Pattern Set Cmd: ptn=  \tex) ptn=3\r\n"); 
    printf("Pattern Get Cmd: ptn?  \tex) ptn?\r\n"); 
    printf("Projection Angle Set Cmd: deg=\tex) deg=90\r\n"); 
    printf("Projection Angle Get Cmd: deg?\tex) deg?\r\n"); 
    printf("System Restart Cmd: rst!     \tex) rst!\r\n");  
    printf("System Pause Cmd: pus!     \tex) pus!\r\n");  
    printf("System Resume Cmd: rsm!     \tex) rsm!\r\n");
    printf("Timer Enable: ten!     \tex) ten!\r\n"); 
    printf("\r\n"); 

    printf("\t\t*** Timer Command *** \t \r\n");
    printf("Timer Disable: tds!     \tex) tds!\r\n");  
    printf("Timer Status: tst!     \tex) tst!\r\n");  
    printf("Pause Time Get Cmd: pus?       \tex) pus?\r\n"); 
    printf("Pause Time Set Cmd: pus=HHMMSS \tex) pus=134502\r\n"); 
    printf("Resume Time Get Cmd: rsm?       \tex) rsm?\r\n"); 
    printf("Resume Time Set Cmd: rsm=HHMMSS \tex) rsm=154502\r\n");  
    printf("\r\n");  

    printf("\t\t*** Date Time *** \t \r\n"); 
    printf("Time Get Cmd: tim?       \tex) tim?\r\n"); 
    printf("Time Set Cmd: tim=HHMMSS \tex) tim=134502\r\n"); 
    printf("Date Get Cmd: day?       \tex) day?\r\n"); 
    printf("Date Set Cmd: day=YYMMDD \tex) day=210802\r\n"); 
    printf("\r\n"); 

    printf("\t\t*** User Setting *** \t \r\n"); 
    printf("String Get Cmd(Flash): str?     \tex) str?\r\n"); 
    printf("String Set Cmd(Flash): str=     \tex) str=Hello\r\n"); 
    printf("String Get Cmd(RAM): stg?       \tex) stg?\r\n"); 
    printf("String Set Cmd(RAM): stg=       \tex) stg=210802\r\n"); 
    printf("\r\n"); 

    printf("\t\t*** Mirror Calibration Command *** \t \r\n"); 
    printf("Center Pos Get Cmd: cen?     \tex) cen?\r\n"); 
    printf("Center Pos Set Cmd: cen=X,Y  \tex) cen=100,-100\r\n"); 
    printf("\r\n"); 

}

static void softReset()
{
    rstEnable = true;
}

