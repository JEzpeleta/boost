#include "main.h"

volatile int salidaON = 0;
volatile int idleTicker = 0;
volatile int32 ScalerActualizaciones = 0;
volatile int32 ScalerActualizacionesMax = SCALER_ACTUALIZACIONES;
volatile int32 ScalerTest = 0;
volatile int32 ScalerTestMax = SCALER_TEST;
volatile int32 Modo = NORMAL;
volatile int32 doSample = 0;
volatile int32 doLoop = 0;
volatile int32 doWaverformCheck = 0;
volatile int32 waveformSnapshot = 0;

volatile int32 SEQ1interrupts = 0;
volatile int32 SEQ2interrupts = 0;

void main(void)
{
	InitSysCtrl();
	DINT;
	InitPieCtrl();
	InitPieVectTable();
	InitAdc();

	Setup_GPIO();
	Setup_ePWM();
	Setup_Protecciones();
	Setup_ADC();
	Setup_Interrupciones();
	Setup_FDatalog();

	while(1){
		if(doSample){
			Muestreo();
			doSample = 0;
			if(waveformSnapshot){
				medicionCorriente();
				waveformSnapshot = 0;
			}
		}

		if(doLoop){
			if(Modo == NORMAL)
			{
				if(ComprobarLimites())
				{
					Set_Salida(_IQ(0));
					Modo = PROTECCION_ON;
				}else{
					Control();
				}
			}

			if(ScalerTest == 0){
				LoopTest();
				ScalerTest = ScalerTestMax;
			}else ScalerTest--;

			if(ScalerActualizaciones == 0)
			{
				switch(Modo){
				case NORMAL:
					if(salidaON) Set_Salida(pid_Ii.term.Out);
					else Set_Salida(_IQ(0));
					break;
				case PROTECCION_ON:
					Set_Salida(_IQ(0));
					break;
				}
				ScalerActualizaciones = ScalerActualizacionesMax;
			}else ScalerActualizaciones--;
			doLoop = 0;
		}
	}
} 

interrupt void cpu_timer1_isr(void)
{
	GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
	CpuTimer1.InterruptCount++;
}

interrupt void cpu_timer0_isr(void)
{
	CpuTimer0.InterruptCount++;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}   

interrupt void adc_isr(void){
	if(AdcRegs.ADCST.bit.INT_SEQ1)
	{
		SEQ1interrupts++;

		doSample = 1;
		doLoop = 1;

		AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;       // Reset SEQ1
		AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;		// Clear INT SEQ1 bit
	}
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge interrupt to PIE
}
