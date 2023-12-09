/*
 *  	motor_ctrl.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */

#ifndef MOTOR_CTRL_H
#define MOTOR_CTRL_H

#define ENABLE_I

#define KP	(8)		//v5:4 		//8:150us
#define KD	(48)	//v5:100	//48:150us
#ifdef ENABLE_I
	#define IKI	(100)//IKI=1/KI //100:150us
#endif


struct MotorCtrlStruct
{
	uint16_t 	encRaw;
	uint16_t 	encBuf[ENCODER_BUFFER_SIZE];
	bool		enableLaser;
	int32_t 	p_cmd;		//Point Command
	int32_t     x_cen;		//Pos Center
	int32_t 	x_cmd;		//Pos Command
	int32_t 	x_err;		//Pos Err
	int32_t		x_sum;		//Pos Err Sum
	int32_t		x_res;		//Pos Response
	int32_t		dx_cmd;		//Vel Command	
	int32_t 	dx_res;		//Vel Response
	int32_t 	dx_err;		//Vel Err
	int32_t		trq_out;	//Torque Output

};


struct MotorLogStruct
{
	int32_t 	x_cmd;		//Pos Command
	int32_t		x_res;		//Pos Response
	int32_t 	x_sum;		//Pos Err Sum
	int32_t		dx_cmd;		//Vel Command	
	int32_t 	dx_res;		//Vel Response
	int32_t		trq_out;	//Torque Output
	
};

extern struct MotorCtrlStruct	motorControl[2];

void MotorCtrlInit();
void MotorCtrlLoop();
void DebugMotorCtrl();
void MotorCtrGetCenterPos(int32_t *pos0, int32_t *pos1);
void MotorCtrSetCenterPos(int32_t pos0, int32_t pos1);
void SetPause(bool isPause);
void SetGain(int32_t kp, int32_t kd, int32_t iki);
void GetGain(int32_t *kp, int32_t *kd, int32_t *iki);
void SetProjectionAngle(int32_t deg);
void GetProjectionAngle(int32_t *deg);


#endif