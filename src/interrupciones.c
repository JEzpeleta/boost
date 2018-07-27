#include "DSP2833x_Device.h"

interrupt void cpu_timer0_isr(void);
interrupt void cpu_timer1_isr(void);
interrupt void adc_isr(void);

void Setup_Interrupciones(void){
		EALLOW;

		PieVectTable.XINT13 = &cpu_timer1_isr;
		PieVectTable.TINT0 = &cpu_timer0_isr;
		PieVectTable.ADCINT = &adc_isr;
		EDIS;

		InitCpuTimers();

		ConfigCpuTimer(&CpuTimer0,150,1000); //interrupci�n a 1KHz 1ms
		ConfigCpuTimer(&CpuTimer1,150,1000000); //interrupci�n a 1s

		PieCtrlRegs.PIEIER1.bit.INTx7 = 1;		// CPU Timer 0
		PieCtrlRegs.PIEIER1.bit.INTx6 = 1;		// ADC

		IER |= 0x1001; //Habilito interrupci�n 1 y 13(timer 0 y 1)

		EINT;
		ERTM;

		CpuTimer1Regs.TCR.bit.TSS = 0;  // start timer1
		CpuTimer0Regs.TCR.bit.TSS = 0;  // start timer0
}
