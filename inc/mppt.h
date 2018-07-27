#ifndef INC_MPPT_H_
#define INC_MPPT_H_

#include "iq.h"

//extern MPPT_PNO_IQ MPPT;
//
//void MPPT_PNO_IQ_update(MPPT_PNO_handle v, _iq Vraw, _iq Iraw);

typedef struct {
	int32  Ipv;
	int32  Vpv;
	int32  DeltaVmin;
	int32  DeltaImin;
	int32  MaxVolt;
	int32  MinVolt;
	int32  Stepsize;
	int32  VmppOut;
	int32  DeltaI;
    int32  DeltaV;
    int32  Ipv_Prev;
    int32  Vpv_Prev;
	int16  mppt_enable;
	int16  mppt_first;
} MPPT_INC_IQ;

typedef MPPT_INC_IQ *MPPT_INC_handle;

extern MPPT_INC_IQ MPPT;

void MPPT_INC_IQ_update(MPPT_INC_handle v, _iq Vraw, _iq Iraw);

#endif /*INC_MPPT_H_*/
