#include "DSP2833x_Device.h"
#include "adc.h"
#include "mppt.h"

#define MPPT_PNO_IQ_DEFAULTS {_IQ(0.0),_IQ(0.0),_IQ(0.0),_IQ(0.9),_IQ(0.0),_IQ(0.01),_IQ(0.0),_IQ(0.0),_IQ(1.0),_IQ(0.0),1,1}

#define DEFAULT_MPPT_PRESCALER ((100000))
#define FILTER_ALPHA ((0.0666))
#define FILTER_ALPHA_IQ ((_IQ(FILTER_ALPHA)))
#define ONE_MINUS_FILTER_ALPHA_IQ ((_IQ(1-FILTER_ALPHA)))
#define FIRST_MPPT_STEP ((_IQ(0.01)))

MPPT_PNO_IQ MPPT = MPPT_PNO_IQ_DEFAULTS;

volatile int32 PowerLimit;
volatile Uint32 MPPTcounter = DEFAULT_MPPT_PRESCALER;
volatile Uint32 MPPTprescaler = DEFAULT_MPPT_PRESCALER;

void MPPT_PNO_IQ_update(MPPT_PNO_handle v, _iq Vraw, _iq Iraw)
{
	v->Ipv = _IQmpy(FILTER_ALPHA_IQ,Iraw) + _IQmpy(ONE_MINUS_FILTER_ALPHA_IQ,v->Ipv);
	v->Vpv = _IQmpy(FILTER_ALPHA_IQ,Vraw) + _IQmpy(ONE_MINUS_FILTER_ALPHA_IQ,v->Vpv);
	if (v->mppt_enable==1)
	{
		if (v->mppt_first == 1)
		{
			v->VmppOut = v->Vpv + FIRST_MPPT_STEP;
			v->mppt_first = 0;
			v->PanelPower_Prev = v->PanelPower;
			MPPTcounter = MPPTprescaler;
		}
		else
		{
			if(!MPPTcounter)
			{
				v->PanelPower = _IQmpy(v->Vpv,v->Ipv);
				v->DeltaP = v->PanelPower - v->PanelPower_Prev;
				if (v->DeltaP > v->DeltaPmin /* & v->PanelPower < PowerLimit*/)
				{
					v->VmppOut = v->Vpv + v->Stepsize;
				}
				else
				{
					if (v->DeltaP < -(v->DeltaPmin) /*| v->PanelPower > PowerLimit*/)
					{
						v->Stepsize = -(v->Stepsize);
						v->VmppOut = v->Vpv + v->Stepsize;
					}
				}
				v->PanelPower_Prev = v->PanelPower;
				if(v->VmppOut < v->MinVolt) v->VmppOut = v->MinVolt;
				if(v->VmppOut > v->MaxVolt) v->VmppOut = v->MaxVolt;
				MPPTcounter = MPPTprescaler;
			}
			else
			{
				MPPTcounter--;
			}
		}
	}
}
