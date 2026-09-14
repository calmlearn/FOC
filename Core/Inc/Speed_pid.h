#ifndef __SPEED_PID_H
#define __SPEED_PID_H
#include "config.h"
void SpeedPid_Init(pid* p);
void SpeedPid_Update(pid* p,float actualnew);
#endif
