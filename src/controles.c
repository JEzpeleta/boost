#include "DSP2833x_Device.h"
#include "iq.h"
#include "adc.h"
#include "pid.h"
#include "mppt.h"
#include "parametros.h"

void Reg_Vo(_iq Vref);
void Reg_Vi(_iq Vref);
void Reg_Ibat(_iq Iref);
void Reg_Ii(_iq Iref);

volatile int32 IiReg = 1;
volatile float IiRef = 2.0;
volatile int32 IbatReg = 1;
volatile float IbatRef = 0.7;
volatile int32 VoReg = 1;
volatile float VoRef = 290.0;
volatile int32 ViReg = 1;
volatile float ViRef = 500.0;
volatile int32 useMPPT = 0;

volatile float mpptOutTmp = 0.0;
volatile int32 ViRefTmp = _IQ(0);

volatile _iq finalIiRef = _IQ(0.0);
volatile _iq prevFinalIiRef = _IQ(0.0);

void Control(){
	finalIiRef = (IiReg) ? _IQ(IiRef/(float)BASE_PU_CORRIENTE) : _IQ(0.0);

	pid_Ibat.param.Umax = (prevFinalIiRef + ANTI_WINDUP_OFFSET > _IQ(UMAX_PID_IBAT)) ? _IQ(UMAX_PID_IBAT) : prevFinalIiRef + ANTI_WINDUP_OFFSET;
	pid_Ibat.param.Umin = (prevFinalIiRef - ANTI_WINDUP_OFFSET < _IQ(UMIN_PID_IBAT)) ? _IQ(UMIN_PID_IBAT) : prevFinalIiRef - ANTI_WINDUP_OFFSET;
	Reg_Ibat(_IQ(IbatRef/(float)BASE_PU_CORRIENTE_BAT));
	if(IbatReg) {if(pid_Ibat.term.Out<finalIiRef) finalIiRef = pid_Ibat.term.Out;}

	pid_Vo.param.Umax = (prevFinalIiRef + ANTI_WINDUP_OFFSET > _IQ(UMAX_PID_VO))? _IQ(UMAX_PID_VO) : prevFinalIiRef + ANTI_WINDUP_OFFSET;
	pid_Vo.param.Umin = (prevFinalIiRef - ANTI_WINDUP_OFFSET < _IQ(UMIN_PID_VO))? _IQ(UMIN_PID_VO) : prevFinalIiRef - ANTI_WINDUP_OFFSET;
	Reg_Vo(_IQ(VoRef/(float)BASE_PU_TENSION));
	if(VoReg) {if(pid_Vo.term.Out<finalIiRef) finalIiRef = pid_Vo.term.Out;}

	pid_Vi.param.Umax = (prevFinalIiRef + ANTI_WINDUP_OFFSET > _IQ(UMAX_PID_VI))? _IQ(UMAX_PID_VI) : prevFinalIiRef + ANTI_WINDUP_OFFSET;
	pid_Vi.param.Umin = (prevFinalIiRef - ANTI_WINDUP_OFFSET < _IQ(UMIN_PID_VI))? _IQ(UMIN_PID_VI) : prevFinalIiRef - ANTI_WINDUP_OFFSET;
	if(useMPPT){
		MPPT.mppt_enable = 1;
		MPPT_INC_IQ_update(&MPPT,vIQ.Vi,vIQ.Ii);
		Reg_Vi(MPPT.VmppOut); //Suponiendo que MPPT.Out esta en IQ y en PU
		ViRef = vF.Vi;
//		mpptOutTmp = IQ2F(MPPT.VmppOut) * BASE_PU_TENSION;
//		Reg_Vi(_IQ(ViRef/(float)BASE_PU_TENSION));
	}else{
		MPPT.mppt_enable = 0;
		MPPT.mppt_first = 1;
		MPPT_INC_IQ_update(&MPPT,vIQ.Vi,vIQ.Ii); //Solo para actualizar filtro
//		ViRefTmp = _IQ(ViRef/(float)BASE_PU_TENSION);
		Reg_Vi(_IQ(ViRef/(float)BASE_PU_TENSION));
	}
	if(ViReg) {if(pid_Vi.term.Out<finalIiRef) finalIiRef = pid_Vi.term.Out;}

	Reg_Ii(finalIiRef);
	prevFinalIiRef = finalIiRef;
}

void Reg_Vo(_iq ref){
	pid_Vo.term.Ref = ref;
	pid_Vo.term.Fbk = vIQ.Vo;
	PID_JE_IQ_FUNC(&pid_Vo);
}

void Reg_Vi(_iq ref){
	pid_Vi.term.Ref = vIQ.Vi; //Invertido a proposito (si quiero más tensión tengo que pedir menos corriente)
	pid_Vi.term.Fbk = ref;
	PID_JE_IQ_FUNC(&pid_Vi);
}

void Reg_Ii(_iq ref){
	pid_Ii.term.Ref = ref;
	pid_Ii.term.Fbk = vIQ.Ii;
	PID_JE_IQ_FUNC(&pid_Ii);
}

void Reg_Ibat(_iq ref){
	pid_Ibat.term.Ref = ref;
	pid_Ibat.term.Fbk = vIQ.Ibat;
	PID_JE_IQ_FUNC(&pid_Ibat);
}
