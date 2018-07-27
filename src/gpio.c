#include "DSP2833x_Device.h"

void Setup_GPIO(void)
{
	EALLOW;
	GpioCtrlRegs.GPAMUX1.all = 0;         // GPIO15 ... GPIO0 = General Puropse I/O
	GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   //ePWM1B Activa
	GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   //ePWM2B Activa
	GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;   //ePWM3B Activa
	GpioCtrlRegs.GPAMUX2.all = 0;		  // GPIO31 ... GPIO16 = General Purpose I/O
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;  // 0=GPIO,  1=SCIRX-A,  2=I2CSDA-A,  3=TZ2
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;  // 0=GPIO,  1=SCITXD-A,  2=I2CSCL-A,  3=TZ3
	GpioCtrlRegs.GPBMUX1.all = 0;		  // GPIO47 ... GPIO32 = General Purpose I/O
	GpioCtrlRegs.GPBMUX2.all = 0;		  // GPIO63 ... GPIO48 = General Purpose I/O
	GpioCtrlRegs.GPCMUX1.all = 0;		  // GPIO79 ... GPIO64 = General Purpose I/O
	GpioCtrlRegs.GPCMUX2.all = 0;		  // GPIO87 ... GPIO80 = General Purpose I/O

	GpioCtrlRegs.GPADIR.all = 0;

	GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;  //Para probar el CpuTimer
	GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;  //Para pobar el periodo del cilco infinito
	GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;  //Para pobar el periodo de la interrupción del adc
	GpioCtrlRegs.GPBDIR.all = 0;		 // GPIO63-32 as inputs
	GpioCtrlRegs.GPCDIR.all = 0;		 // GPIO87-64 as inputs

	GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;

	EDIS;
}
