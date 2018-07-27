#include "iq.h"
#include "pid.h"
#include "parametros.h"

PID_JE_IQ_CONTROLLER pid_Vo = {DEFAULT_JE_IQ_TERMINALS,PID_VO_PARAMETERS,DEFAULT_JE_IQ_DATA};
PID_JE_IQ_CONTROLLER pid_Vi = {DEFAULT_JE_IQ_TERMINALS,PID_VI_PARAMETERS,DEFAULT_JE_IQ_DATA};
PID_JE_IQ_CONTROLLER pid_Ibat = {DEFAULT_JE_IQ_TERMINALS,PID_IBAT_PARAMETERS,DEFAULT_JE_IQ_DATA};
PID_JE_IQ_CONTROLLER pid_Ii = {DEFAULT_JE_IQ_TERMINALS,PID_II_PARAMETERS,DEFAULT_JE_IQ_DATA};

void PID_JE_IQ_FUNC(PID_JE_HANDLE v)
{
	/* proportional term */
	v->data.Err = v->term.Ref - v->term.Fbk;

	/* integral term */
	v->data.Ui += (v->data.W1 == _IQ(1.0))? _IQmpy(v->param.Ki,v->data.Err) : _IQmpy(v->param.Kc,v->term.Out - v->data.OutPreSat);

	/* control output */
	v->data.OutPreSat = _IQmpy(v->param.Kp, (v->data.Err + v->data.Ui));
	v->term.Out = _IQsat(v->data.OutPreSat, v->param.Umax, v->param.Umin);
	v->data.W1 = _IQ(1.0);
	if(v->term.Out < v->data.OutPreSat) if(v->term.Ref > v->term.Fbk) v->data.W1 = _IQ(0.0);
	if(v->term.Out > v->data.OutPreSat) if(v->term.Ref < v->term.Fbk) v->data.W1 = _IQ(0.0);
}
