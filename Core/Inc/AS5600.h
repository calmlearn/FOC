#ifndef __AS5600_H
#define __AS5600_H
#include "main.h"
#include "stdint.h"

#define AS5600_Address (0x36<<1)
#define RAW_Angle_H (0x0C)
#define RAW_Angle_L (0x0D)

uint8_t AS5600_AngleIsValid(void);
float AS5600_GetSpeed(void);
float AS5600_GetAngle(void);
HAL_StatusTypeDef AS5600_Read_RawAngle(void);

#endif
