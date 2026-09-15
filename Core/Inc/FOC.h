#ifndef __FOC_H
#define __FOC_H

#include "stdint.h"
float motorAngle(float angle); //限制角度范围
float motorAngleSigned(float angle);
void SVPWM_FOC(float Ud,float Uq,float U_angle); //计算占空比
#endif
