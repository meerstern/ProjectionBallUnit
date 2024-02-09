/*
 *  	motor_ctrl.c
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "ProjectionBall.h"
#include "motor_ctrl.h"
#include "path_ctrl.h"
#include "path_const.h"
#include "encoder_as5048a.h"
#include "encoder_ma732.h"
#include "rtc_rv8803.h"
#include "rtc_sd30XX.h"

#define 	MAX_LOG_LEN				1500

#ifdef ENABLE_STEP_CMD
	#define 	STEP_START_LEN		500
	#define 	STEP_END_LEN		1000
#endif



#define X_CENTER0					2500//2500
#define X_CENTER1					2000//2000
#define PWM_MAX_DUTY				2048

#define T_LIMIT_MAXERR_N			-1000
#define T_LIMIT_MAXERR_P			+1000
#define T_LIMIT_DIV					20	
#define T_LIMIT_START_CNT			800
#define T_LIMIT_START_CNT_HALF		(T_LIMIT_START_CNT/2)

#ifdef ENABLE_RUNAWAY_DETECTION
	#define MAX_ERR_COUNT			800
	#define POS_ERR_RANGE			100
	#define T_LIMIT_DIV2			10	
#endif

struct MotorCtrlStruct		motorControl[2];
struct MotorLogStruct		motorLog[2][MAX_LOG_LEN];
static uint					pwm_slice_num;
volatile static uint 		InitCount;//Torque Limit for suppress runaway in start up
volatile static uint32_t	ControlCount;
volatile static uint32_t 	ErrCount;
volatile static bool		isPause = false;
volatile static bool		isPause_old = false;
volatile static int32_t 	kpGain;
volatile static int32_t 	kdGain;
volatile static int32_t 	ikiGain;
volatile static uint8_t		uPrjAngle;
volatile static int32_t		prjAngle;

inline static void getResponse();
inline static void shiftBuffer();
inline static void calcResponse();
inline static void getCommand();
inline static void calcErr();
inline static void calcTrqCommand();
inline static void setMotorOutput();
inline static void calcVelErr();
inline static void logSave();




void MotorCtrlInit()
{	
#ifdef USE_MA732
	//Set Filter Setting
	MA732WriteReg(PIN_CS1, MA732_REG_FW,84);//def:64:84::80
	MA732WriteReg(PIN_CS2, MA732_REG_FW,84);//def:64:84::80
#endif

	motorControl[0].x_cen=X_CENTER0;
	motorControl[1].x_cen=X_CENTER1;
	
	motorControl[0].x_sum=0;
	motorControl[1].x_sum=0;
	
	InitCount = 0;
	ControlCount = 0;
	ErrCount = 0;

	kpGain = KP;
	kdGain = KD;
#ifdef ENABLE_I
	ikiGain = IKI;
#endif
#ifndef DISABLE_MOTOR
	gpio_put(PIN_STBY, 1);//1:DRV ON, 0:DRV OFF
	gpio_put(PIN_LSR, 1);//1:ON, 0:OFF
	
	
	//Init Pwm Setting	
	pwm_slice_num = pwm_gpio_to_slice_num(2);
	pwm_set_clkdiv(pwm_slice_num, 3.8);//16kHz
    pwm_set_wrap(pwm_slice_num, PWM_MAX_DUTY);
	pwm_set_chan_level(pwm_slice_num, PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_slice_num, PWM_CHAN_B, 0);
	pwm_set_enabled(pwm_slice_num, true);	
#endif
}


void __time_critical_func(MotorCtrlLoop)()
{	
	
	if(isPause==true)
	{
		gpio_put(PIN_STBY, 0);//1:DRV ON, 0:DRV OFF
		gpio_put(PIN_LSR, 0);//1:ON, 0:OFF
		return;
	}
		
	getResponse();		
	shiftBuffer();
	calcResponse();
	getCommand();
	calcErr();
	calcTrqCommand();
	setMotorOutput();
	logSave();
	calcVelErr();
	
#ifdef ENABLE_STEP_CMD
	if(InitCount<T_LIMIT_START_CNT)
	{
		//Nothing todo 
	}
	else if(ControlCount<MAX_LOG_LEN)
	{
		ControlCount++;
	}		
	else
	{
		gpio_put(PIN_STBY, 0);
	}
#endif

}

void DebugMotorCtrl()
{
	// float angf=0.0f;
	// angf=(float)motorControl[0].x_res/(0x3FFF)*360.0f;	
	// printf("Angle:%1.1f\n",angf);
	
	// printf("res:%d, \t tau:%d \t err:%d \n",
		// motorControl[0].x_res,
		// motorControl[0].trq_out,
		// motorControl[0].x_err);
#ifdef ENABLE_STEP_CMD
	if(ControlCount>(MAX_LOG_LEN-1))
	{
		for(int i=0; i<MAX_LOG_LEN; i++)
		{
			printf("%d,%d,%d,%d, \t%d,%d,%d,%d\n",
				motorLog[0][i].x_cmd,
				motorLog[0][i].x_res,
				motorLog[0][i].x_sum,
				motorLog[0][i].trq_out,
				motorLog[1][i].x_cmd,
				motorLog[1][i].x_res,
				motorLog[1][i].x_sum,
				motorLog[1][i].trq_out
				);			
		}
		
	}
	while(1);
#endif

#if defined( ENABLE_SIN_CMD) ||  defined (ENABLE_DEBUG_INFO)
	printf("\t%d,%d,%d,%d, \t%d,%d,%d,%d\n",
				motorControl[0].x_cmd,
				motorControl[0].x_res,
				motorControl[0].x_sum,
				motorControl[0].trq_out,
				motorControl[1].x_cmd,
				motorControl[1].x_res,
				motorControl[1].x_sum,
				motorControl[1].trq_out
				);	
	DebugPathCtrl();		
#endif

}

void MotorCtrSetCenterPos(int32_t pos0, int32_t pos1)
{
	motorControl[0].x_cen=X_CENTER0 + pos0;
	motorControl[1].x_cen=X_CENTER1 + pos1;
	printf("CenOff: %d, %d CenPos: %d, %d\r\n", pos0, pos1, motorControl[0].x_cen,motorControl[1].x_cen);
}

void MotorCtrGetCenterPos(int32_t *pos0, int32_t *pos1)
{
	*pos0 = motorControl[0].x_cen;
	*pos1 = motorControl[1].x_cen;
}

void SetPause(bool pause)
{
	isPause = pause;

	if(isPause!=isPause_old)
	{
		printf(" *Set Pause Enabled:%d->%d\r\n", isPause_old, isPause);
		if(isPause==false)
		{
			InitCount = 0;
			ControlCount = 0;
			ErrCount = 0;
			motorControl[0].x_sum = 0;
			motorControl[1].x_sum = 0;	
			gpio_put(PIN_STBY, 1);//1:DRV ON, 0:DRV OFF
			gpio_put(PIN_LSR, 1);//1:ON, 0:OFF
		}
		isPause_old = isPause;
	}	
}

void SetGain(int32_t kp, int32_t kd, int32_t iki)
{
	kpGain = kp;
	kdGain = kd;
	ikiGain = iki;
}

void GetGain(int32_t *kp, int32_t *kd, int32_t *iki)
{
	*kp = kpGain;
	*kd = kdGain;
	*iki = ikiGain;
}

void SetProjectionAngle( int32_t deg )
{	
	prjAngle = deg/((int)360/TRIG_FUNCTION_LEN);
	prjAngle = prjAngle*((int)360/TRIG_FUNCTION_LEN);
	uPrjAngle = prjAngle/((int)360/TRIG_FUNCTION_LEN);
	SetRtcRam(PRJ_DEG_SRAM, uPrjAngle);
}

void GetProjectionAngle(int32_t *deg)
{
	*deg = prjAngle;
}

inline static void getResponse()
{
#ifdef USE_MA732
	MA732ReadAngleBit(PIN_CS1, &motorControl[0].encRaw);
	MA732ReadAngleBit(PIN_CS2, &motorControl[1].encRaw);
#endif
#ifdef USE_AS5048A
	AS5048AReadAngleBit(PIN_CS1, &motorControl[0].encRaw);
	AS5048AReadAngleBit(PIN_CS2, &motorControl[1].encRaw);
#endif
}

inline static void shiftBuffer()
{
	
	for(size_t i=0; i< (ENCODER_BUFFER_SIZE-1); i++)
	{
		motorControl[0].encBuf[ENCODER_BUFFER_SIZE-(i+1)]
			=motorControl[0].encBuf[ENCODER_BUFFER_SIZE-(i+2)];
		motorControl[1].encBuf[ENCODER_BUFFER_SIZE-(i+1)]
			=motorControl[1].encBuf[ENCODER_BUFFER_SIZE-(i+2)];			
	}
	
	motorControl[0].encBuf[0]=motorControl[0].encRaw;
	motorControl[1].encBuf[0]=motorControl[1].encRaw;
}

inline static void calcResponse()
{
	int32_t sum[2]={0};
	
	for(size_t i=0; i<ENCODER_BUFFER_SIZE; i++)
	{
		sum[0]+=motorControl[0].encBuf[i];
		sum[1]+=motorControl[1].encBuf[i];		
	}
	sum[0]=(sum[0]/ENCODER_BUFFER_SIZE);
	sum[1]=(sum[1]/ENCODER_BUFFER_SIZE);
	
	motorControl[0].x_res=(sum[0]>>2);//16bit->14bit
	motorControl[1].x_res=(sum[1]>>2);//16bit->14bit	
}

inline static void getCommand()
{
	//Command
	motorControl[0].p_cmd = 0;
	motorControl[1].p_cmd = 0;
	motorControl[0].dx_cmd = 0;
	motorControl[1].dx_cmd = 0;
	motorControl[0].enableLaser = 0;

	GetPathCmd(&motorControl[0].p_cmd, &motorControl[1].p_cmd, &motorControl[0].enableLaser );

#ifdef ENABLE_STEP_CMD
	if(ControlCount>STEP_START_LEN && ControlCount<STEP_END_LEN)
	{
		motorControl[0].p_cmd = -150;
		motorControl[1].p_cmd = -150;		
	}
	else if(ControlCount>STEP_END_LEN)
	{
		motorControl[0].p_cmd = 0;
		motorControl[1].p_cmd = 0;
	}
#endif

#ifdef ENABLE_PROJECTION_ANGLE
	motorControl[0].p_cmd = CalcCosVal(uPrjAngle, motorControl[0].p_cmd) - CalcSinVal(uPrjAngle, motorControl[1].p_cmd);
	motorControl[1].p_cmd = CalcSinVal(uPrjAngle, motorControl[0].p_cmd) + CalcCosVal(uPrjAngle, motorControl[1].p_cmd);
#endif

#ifdef ENABLE_CALIBRATION_MODE
	motorControl[0].p_cmd = 0;
	motorControl[1].p_cmd = 0;
	motorControl[0].enableLaser = 1;
#endif

	//Set Command Value
	motorControl[0].x_cmd = motorControl[0].x_cen - motorControl[0].p_cmd;
	motorControl[1].x_cmd = motorControl[1].x_cen - motorControl[1].p_cmd;
		
}

inline static void calcErr()
{
	motorControl[0].x_err=motorControl[0].x_cmd-motorControl[0].x_res;
	motorControl[1].x_err=motorControl[1].x_cmd-motorControl[1].x_res;	
	
	motorControl[0].x_sum+=motorControl[0].x_err;
	motorControl[1].x_sum+=motorControl[1].x_err;

#ifdef ENABLE_RUNAWAY_DETECTION
	if(InitCount>=T_LIMIT_START_CNT)
	{
		if(	motorControl[0].x_err > POS_ERR_RANGE || motorControl[0].x_err < -POS_ERR_RANGE ||
			motorControl[1].x_err > POS_ERR_RANGE || motorControl[1].x_err < -POS_ERR_RANGE	)
		{
			ErrCount++;
			if(ErrCount>MAX_ERR_COUNT)
			{
				gpio_put(PIN_ERR, 1);
				gpio_put(PIN_STBY, 0);//STOP MOTOR
			}
		}
		else
		{
			ErrCount = 0;
		}
	}
#endif

}

inline static void calcTrqCommand()
{
	motorControl[0].trq_out = kpGain*motorControl[0].x_err;
	motorControl[0].trq_out+= kdGain*motorControl[0].dx_err;
#ifdef ENABLE_I
	motorControl[0].trq_out+= motorControl[0].x_sum/ikiGain;
#endif


	motorControl[1].trq_out = kpGain*motorControl[1].x_err;
	motorControl[1].trq_out+= kdGain*motorControl[1].dx_err;
#ifdef ENABLE_I
	motorControl[1].trq_out+= motorControl[1].x_sum/ikiGain;
#endif					
	
	//Torque Limit for suppress runaway	in start up
	if( InitCount < T_LIMIT_START_CNT_HALF )
	{
		motorControl[0].trq_out=motorControl[0].trq_out/T_LIMIT_DIV;
		motorControl[1].trq_out=0;
		motorControl[0].x_sum = 0;
		motorControl[1].x_sum = 0;
		InitCount++;

	}
	else if(InitCount<T_LIMIT_START_CNT)
	{
		motorControl[0].trq_out=motorControl[0].trq_out/T_LIMIT_DIV;
		motorControl[1].trq_out=motorControl[1].trq_out/T_LIMIT_DIV;
		motorControl[0].x_sum = 0;
		motorControl[1].x_sum = 0;
		InitCount++;
	}
					
	//Torque Limit for suppress runaway				
	 if( motorControl[0].x_err>T_LIMIT_MAXERR_P || motorControl[0].x_err<T_LIMIT_MAXERR_N )
	 {
		motorControl[0].trq_out=motorControl[0].trq_out/T_LIMIT_DIV2;
		motorControl[0].x_sum = 0;
		motorControl[0].dx_err = 0;
	 }
	 	
	
	 if( motorControl[1].x_err>T_LIMIT_MAXERR_P || motorControl[1].x_err<T_LIMIT_MAXERR_N )
	 {
		motorControl[1].trq_out=motorControl[1].trq_out/T_LIMIT_DIV2;
		motorControl[1].x_sum = 0;	
		motorControl[1].dx_err = 0;	
	 }
}

inline static void setMotorOutput()
{
	int32_t duty[2]={0};
	
#ifdef USE_AS5048A
	motorControl[0].trq_out=-1*motorControl[0].trq_out;
	motorControl[1].trq_out=-1*motorControl[1].trq_out;
#endif
	
	duty[0]=(motorControl[0].trq_out>0)?
				(motorControl[0].trq_out):(-1*motorControl[0].trq_out);
	duty[1]=(motorControl[1].trq_out>0)?
				(motorControl[1].trq_out):(-1*motorControl[1].trq_out);
				
	duty[0]=(duty[0]>=PWM_MAX_DUTY)?(PWM_MAX_DUTY-1):(duty[0]);
	duty[1]=(duty[1]>=PWM_MAX_DUTY)?(PWM_MAX_DUTY-1):(duty[1]);

	if(motorControl[0].trq_out>0)
	{
		gpio_put(PIN_A1, 1);
		gpio_put(PIN_A2, 0);
	}
	else if(motorControl[0].trq_out<0)
	{
		gpio_put(PIN_A1, 0);
		gpio_put(PIN_A2, 1);
	}
	else
	{
		gpio_put(PIN_A1, 0);
		gpio_put(PIN_A2, 0);
	}
	
	if(motorControl[1].trq_out>0)
	{
		gpio_put(PIN_B1, 0);
		gpio_put(PIN_B2, 1);
	}
	else if(motorControl[1].trq_out<0)
	{
		gpio_put(PIN_B1, 1);
		gpio_put(PIN_B2, 0);
	}
	else
	{
		gpio_put(PIN_B1, 0);
		gpio_put(PIN_B2, 0);
	}
	//Set Motor Output
	pwm_set_chan_level(pwm_slice_num, PWM_CHAN_A, duty[0]);
	pwm_set_chan_level(pwm_slice_num, PWM_CHAN_B, duty[1]);

	//Set Laser Output
	gpio_put(PIN_LSR, motorControl[0].enableLaser);
	
}

inline static void calcVelErr()
{
	motorControl[0].dx_res = motorControl[0].encBuf[0]-motorControl[0].encBuf[1];
	motorControl[1].dx_res = motorControl[1].encBuf[0]-motorControl[1].encBuf[1];	

	motorControl[0].dx_err = motorControl[0].dx_cmd - motorControl[0].dx_res;
	motorControl[1].dx_err = motorControl[1].dx_cmd - motorControl[1].dx_res;
}

inline static void logSave()
{	
	motorLog[0][ControlCount].x_cmd=motorControl[0].x_cmd;	
	motorLog[1][ControlCount].x_cmd=motorControl[1].x_cmd;
	motorLog[0][ControlCount].x_res=motorControl[0].x_res;
	motorLog[1][ControlCount].x_res=motorControl[1].x_res;
	motorLog[0][ControlCount].dx_cmd=motorControl[0].dx_cmd;	
	motorLog[1][ControlCount].dx_cmd=motorControl[1].dx_cmd;
	motorLog[0][ControlCount].dx_res=motorControl[0].dx_res;
	motorLog[1][ControlCount].dx_res=motorControl[1].dx_res;
	motorLog[0][ControlCount].trq_out=motorControl[0].trq_out;
	motorLog[1][ControlCount].trq_out=motorControl[1].trq_out;	
	motorLog[0][ControlCount].x_sum=motorControl[0].x_sum;
	motorLog[1][ControlCount].x_sum=motorControl[1].x_sum;	
}
