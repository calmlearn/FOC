#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdint.h"

#define AS5600_Address (0x36<<1)
#define RAW_Angle_H (0x0C)
#define RAW_Angle_L (0x0D)


void AS5600_ReadRawAngle(uint8_t* pData)
{
	HAL_I2C_Mem_Read(&hi2c1,AS5600_Address,RAW_Angle_H,I2C_MEMADD_SIZE_8BIT,pData,2,5);
	
}
