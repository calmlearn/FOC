#ifndef __MOTORPWM_H
#define __MOTORPWM_H
#include "main.h"
#include "stdint.h"
void MotorPWM_Disable(void);
void MotorPWM_Enable(void);
void MotorPWM_Init(void);
static float MotorPWM_ClampDuty(float duty);
void Motor_SetDuty(float duty_u,float duty_v,float duty_w);
#endif
