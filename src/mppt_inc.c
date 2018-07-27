#include "DSP2833x_Device.h"
#include "adc.h"
#include "mpptinc.h"

#define MPPT_INC_IQ_DEFAULTS {_IQ(0.0),_IQ(0.0),_IQ(0.0),_IQ(0.0),_IQ(0.9),_IQ(0.0),_IQ(0.01),_IQ(1.0),_IQ(0.0),_IQ(0.0),_IQ(0.0),_IQ(0.0),1,1}

#define DEFAULT_MPPT_PRESCALER ((100000))
#define FILTER_ALPHA ((0.0666))
#define FILTER_ALPHA_IQ ((_IQ(FILTER_ALPHA)))
#define ONE_MINUS_FILTER_ALPHA_IQ ((_IQ(1-FILTER_ALPHA)))
#define FIRST_MPPT_STEP ((_IQ(0.01)))

MPPT_INC_IQ MPPT = MPPT_INC_IQ_DEFAULTS;

volatile int32 PowerLimit;
volatile Uint32 MPPTcounter = DEFAULT_MPPT_PRESCALER;
volatile Uint32 MPPTprescaler = DEFAULT_MPPT_PRESCALER;

void MPPT_INC_IQ_update(MPPT_INC_handle v, _iq Vraw, _iq Iraw)
{
    v->Ipv = _IQmpy(FILTER_ALPHA_IQ,Iraw) + _IQmpy(ONE_MINUS_FILTER_ALPHA_IQ,v->Ipv);
    v->Vpv = _IQmpy(FILTER_ALPHA_IQ,Vraw) + _IQmpy(ONE_MINUS_FILTER_ALPHA_IQ,v->Vpv);
    if (v->mppt_enable==1)
    {
        if (v->mppt_first == 1)
        {
            v->VmppOut = v->Vpv + FIRST_MPPT_STEP;
            v->mppt_first = 0;
            v->Vpv_Prev = v->Vpv;
            v->Ipv_Prev = v->Ipv;
            MPPTcounter = MPPTprescaler;
        }else{
            if(!MPPTcounter)
            {
                v->DeltaV=v->Vpv - v->Vpv_Prev;
                v->DeltaI=v->Ipv - v->Ipv_Prev;
                if (v->DeltaV==_IQ(0.0))
                {
                    if (v->DeltaI==_IQ(0.0))
                    {
                            //no hace nada
                    }else{
                            if(v->DeltaI > _IQ(0.0))
                            {
                                    v->VmppOut = v->Vpv - v->Stepsize;
                            }else
							{
                                    v->VmppOut = v->Vpv + v->Stepsize;
                            }
                        }
                }else{
					if((v->Ipv + (_IQdiv(_IQmpy(v->DeltaI,v->Vpv),v->DeltaV))) == _IQ(0.0)){
                                //no hace nada
                        }else{
							if((v->Ipv + (_IQdiv(_IQmpy(v->DeltaI,v->Vpv),v->DeltaV))) > _IQ(0.0)){
                                v->VmppOut = v->Vpv + v->Stepsize;
                            }
							else{
								v->VmppOut = v->Vpv - v->Stepsize;
							}
                        }
				}
				v->Vpv_Prev = v->Vpv;
				v->Ipv_Prev = v->Ipv;
				if(v->VmppOut < v->MinVolt) v->VmppOut = v->MinVolt;
				if(v->VmppOut > v->MaxVolt) v->VmppOut = v->MaxVolt;
				MPPTcounter = MPPTprescaler;
            }else{
                        MPPTcounter--;
            }
        }
    }
}
