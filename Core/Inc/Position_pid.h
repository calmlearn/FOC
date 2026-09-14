#ifndef __POSITION_PID_H
#define __POSITION_PID_H
#include "config.h"
void PositionPid_Init(pid* p);
void PositionPid_Update(pid* p,float actualnew);
#endif
