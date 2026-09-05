#include "motorapp.h"
#include "FOC.h"       
#include "tim.h"
#include <math.h>

#define PI 3.14159265f
#define MOTOR_POLE_PAIRS  7.0f
#define dt        0.001f   // TIM2：1ms，即1kHz

static float electrical_angle = 0.0f;

void Motor_OpenLoopUpdate(float mechanical_rpm, float uq)
{
    float electrical_speed;

    /* 机械转速rpm(圈/min) → 电角速度rad/s */
    electrical_speed = mechanical_rpm
                     * (2*PI) / 60.0f
                     * MOTOR_POLE_PAIRS;

    /* 1. 根据速度和时间间隔累加电角度 */
    electrical_angle += electrical_speed * dt;

    /* 2. 将角度折回0～2π，兼容反转 */
    electrical_angle = fmodf(electrical_angle,2*PI);

    if (electrical_angle < 0.0f)
    {
        electrical_angle += (2*PI);
    }

    /*
     * 反Park + SVPWM
     * 该函数内部调用MotorPwm_SetDuty，更新三个CCR
     */
    SVPWM_FOC(0.0f,uq, electrical_angle);
}
