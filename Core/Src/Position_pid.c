#include "main.h"
#include "FOC.h"
#include "config.h"

void PositionPid_Init(pid* p)
{
	p->errorint = 0;
	p->errornew = 0;
	p->errorold = 0;
	p->out = 0;
	p->target = 0;
	p->actual = 0;
}

void PositionPid_Update(pid* p,float actualnew)	//actualnew 为传入的最新测量值
{
	p->target = motorAngleSigned(p->target);
	
	p->actualold = p->actual;
	p->actual = actualnew;
	p->errorold = p->errornew;
	p->errornew = motorAngleSigned(p->target - p->actual);
	
	if(p->ki == 0)
	{
		p->errorint = 0;
	}else{
		p->errorint += p->errornew;
	
		if(p->errorint> p->intmax){p->errorint = p->intmax;}
		if(p->errorint< p->intmin){p->errorint = p->intmin;}
	}
	
	
	p->out = p->kp * p->errornew + p->ki * p->errorint + 
						p->kd * motorAngleSigned(p->actualold - p->actual);
//						p->kd * motorAngleSigned(p->errornew - p->errorold);
	
	if(p->out>p->outmax){p->out = p->outmax;}
	if(p->out<p->outmin){p->out = p->outmin;}
}

