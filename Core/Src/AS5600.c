#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "stdint.h"
#include "AS5600.h"

#include "config.h"

static uint8_t pData[2];
static volatile float mechanical_rpm;
static volatile float angle_rad;
static  uint8_t initial=1;

	
float AS5600_GetSpeed(void)
{
	return mechanical_rpm;
}

float AS5600_GetAngle(void)
{
	return (angle_rad - angle_zero);
}

HAL_StatusTypeDef AS5600_Read_RawAngle(void)
{
	return HAL_I2C_Mem_Read_IT(&hi2c1,AS5600_Address,RAW_Angle_H,I2C_MEMADD_SIZE_8BIT,pData,2);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        // 进入这里，说明本次请求的两个字节已经接收完成
		uint16_t angle;
		angle = ((pData[0]&0x0F)<<8)|pData[1];
		angle_rad = angle / 4096.0f * 2.0f*PI;
		
		float angle_new,angle_diff;
		static float angle_last;
		
		if(initial)
		{
			angle_last = angle_rad;
			mechanical_rpm = 0.0f;
			initial=0;
			return;
		}
		angle_new = angle_rad;
		angle_diff = angle_new - angle_last;
		angle_last = angle_new;
		
		//消除角度跳变
		if(angle_diff<-PI)
		{
			angle_diff += 2*PI;
		}
		if(angle_diff> PI)
		{
			angle_diff -= 2*PI;
		}
			
		float raw_rpm = angle_diff*60.0f/(dt*2.0f*PI);
		float alph = 0.01;
		mechanical_rpm += alph*(raw_rpm - mechanical_rpm);
    }
}

