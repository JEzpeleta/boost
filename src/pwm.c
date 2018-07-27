#include "DSP2833x_Device.h"

void Setup_ePWM(void)
{
	EPwm1Regs.CMPA.half.CMPA = 0;
	EPwm1Regs.TBCTL.bit.CLKDIV = 0;       // CLKDIV = 1 . 	(TB Clock Prescale en /1)
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = 1;    // HSPCLKDIV = 2 	(High Speed TB Clock Prescale en /4)
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;      // up mode
	EPwm1Regs.TBPRD = 5040;			      // 5000 para 15 kHz
	EPwm1Regs.AQCTLB.all = 0x0012;        // zero = set; period = clear

	EPwm2Regs.CMPA.half.CMPA = 0;
	EPwm2Regs.TBCTL.bit.CLKDIV = 0;       // CLKDIV = 1 . 	(TB Clock Prescale en /1)
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = 1;    // HSPCLKDIV = 2 	(High Speed TB Clock Prescale en /4)
	EPwm2Regs.TBCTL.bit.CTRMODE = 0;      // up mode
	EPwm2Regs.TBPRD = 5040;			      // 5000 para 15 kHz
	EPwm2Regs.AQCTLB.all = 0x0012;        // zero = set; period = clear

	EPwm3Regs.CMPA.half.CMPA = 0;
	EPwm3Regs.TBCTL.bit.CLKDIV = 0;       // CLKDIV = 1 . 	(TB Clock Prescale en /1)
	EPwm3Regs.TBCTL.bit.HSPCLKDIV = 1;    // HSPCLKDIV = 2 	(High Speed TB Clock Prescale en /4)
	EPwm3Regs.TBCTL.bit.CTRMODE = 0;      // up mode
	EPwm3Regs.TBPRD = 5040;			      // 5000 para 15 kHz
	EPwm3Regs.AQCTLB.all = 0x0012;        // zero = set; period = clear

	EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;	 // generate a syncout if CTR = 0

	EPwm2Regs.TBCTL.bit.PHSEN = 1;		 // enable phase shift for ePWM2
	EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;  	 // syncin = syncout
	EPwm2Regs.TBPHS.half.TBPHS = 1680;	 // 1/3 phase shift (1667)
	EPwm3Regs.TBCTL.bit.PHSEN = 1;		 // enable phase shift for ePWM3
    EPwm3Regs.TBPHS.half.TBPHS = 3360; 	 // 2/3 phase shift (3333)

//    EPwm4Regs.CMPA.half.CMPA = 0;
//    EPwm4Regs.TBCTL.bit.CLKDIV = 0;
//    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 1;
//    EPwm4Regs.TBCTL.bit.CTRMODE = 0;
//    EPwm4Regs.TBPRD = 500;               // 500 para 150 kHz
//    EPwm4Regs.TBCTL.bit.SYNCOSEL = 3;    // No SYNCOUT
//    EPwm4Regs.AQCTLB.all = 0x0012;       // zero = set; period = clear
//    EPwm4Regs.TBCTL.bit.PHSEN = 1;
//    EPwm4Regs.TBPHS.half.TBPHS = 0; 	 // 0 phase shift
}
