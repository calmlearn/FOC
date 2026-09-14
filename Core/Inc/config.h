#ifndef __CONFIG_H
#define __CONFIG_H

#define PI 3.14159265f
#define dt 0.001
#define MOTOR_POLE_PAIRS  7.0f
#define e_zero_offset 3.9998f //电角度偏置
#define angle_zero 1.469f //机械角度偏置

typedef struct{
	float kp;
	float ki;
	float kd;
	
	float actual;
	float actualold;
	float target;
	float out;
	float outmax;
	float outmin;
	float errorint;
	float errorold;
	float errornew;
	
	float offset;
	float intmax;
	float intmin;
	
}pid;

#endif
