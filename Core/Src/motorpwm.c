#include "main.h"
#include "tim.h"
#include "motorpwm.h"

void MotorPWM_Disable(void)
{
	HAL_GPIO_WritePin(DRV_EN_GPIO_Port, DRV_EN_Pin, GPIO_PIN_RESET);
}

void MotorPWM_Enable(void)
{
	HAL_GPIO_WritePin(DRV_EN_GPIO_Port, DRV_EN_Pin, GPIO_PIN_SET);
}

void MotorPWM_Init(void)
{
	MotorPWM_Disable();
	
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);
	
}

static float MotorPWM_ClampDuty(float duty)
{
    if (duty < 0.0f)
    {
        return 0.0f;
    }

    if (duty > 1.0f)
    {
        return 1.0f;
    }

    return duty;
}
//占空比=ccr/(arr+1)
void Motor_SetDuty(float duty_u,float duty_v,float duty_w)
{
	uint32_t arr;
    uint32_t ccr_u;
    uint32_t ccr_v;
    uint32_t ccr_w;
	
	duty_u = MotorPWM_ClampDuty(duty_u);
    duty_v = MotorPWM_ClampDuty(duty_v);
    duty_w = MotorPWM_ClampDuty(duty_w);
	
	arr = __HAL_TIM_GET_AUTORELOAD(&htim1);
	
	ccr_u = (uint32_t)(duty_u * (float)(arr+1U));
    ccr_v = (uint32_t)(duty_v * (float)(arr+1U));
    ccr_w = (uint32_t)(duty_w * (float)(arr+1U));
	
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_u);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr_v);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, ccr_w);
}
					  						  
	






