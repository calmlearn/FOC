#include "main.h"
#include "math.h"
#include "motorpwm.h"

#include "config.h"

float motorAngle(float angle)
{
	float a = fmodf(angle,2*PI);
	return a>=0?a:(a+2*PI);
}

void SVPWM_FOC(float Ud,float Uq,float U_angle)
{
	U_angle = motorAngle(U_angle);
	
	//反park
	float U_alph =Ud*cosf(U_angle)-Uq* sinf(U_angle);
	float U_beta = Ud* sinf(U_angle) + Uq*cosf(U_angle);
	
	float U_ref = sqrtf(U_alph*U_alph+U_beta*U_beta) /12;
	if(U_ref> 0.577)
	{
		float s = 0.577/U_ref;
		U_ref = 0.577;
		
		U_alph *= s;
		U_beta *= s;
	}
	
	//反clark
	float  Uu,Uv,Uw;
	Uu = U_alph;
	Uv = -0.5*U_alph +0.8660254*U_beta;
	Uw = -0.5*U_alph -0.8660254*U_beta;
	
	float Umax = fmaxf(Uu, fmaxf(Uv, Uw));
	float Umin = fminf(Uu, fminf(Uv, Uw));
	
	float Uoffset = -0.5f * (Umax + Umin);
	
	float du = 0.5f + (Uu + Uoffset) / 12.0f;
	float dv = 0.5f + (Uv + Uoffset) / 12.0f;
	float dw = 0.5f + (Uw + Uoffset) / 12.0f;
	
	Motor_SetDuty(du, dv, dw);
}
