#include "DSP2833x_Device.h"
#include "iq.h"

void Set_Salida(_iq ciclo){
	EPwm1Regs.CMPA.half.CMPA = (Uint16) (IQ2F(ciclo)*5000);
	EPwm2Regs.CMPA.half.CMPA = (Uint16) (IQ2F(ciclo)*5000);
	EPwm3Regs.CMPA.half.CMPA = (Uint16) (IQ2F(ciclo)*5000);
}
