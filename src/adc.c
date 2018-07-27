#include "DSP2833x_Device.h"
#include "iq.h"
#include "adc.h"

#define MAX(a,b) ((a)>(b)?(a):(b)))

variablesIQ vIQ;
variablesF vF;
volatile float waveform[8];

void MuestreoIQ();
void variablesIQ2F();
inline float IQ2F(_iq);

void Setup_ADC (void){
	AdcRegs.ADCTRL1.all = 0;
	AdcRegs.ADCTRL1.bit.ACQ_PS = 11; 	// 11 = 12 x ADCCLK
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1; 	// 0 = indep. seq., 1 = cascaded sequencer
	AdcRegs.ADCTRL1.bit.CPS = 0;		// 0 = divide by 1
	AdcRegs.ADCTRL1.bit.CONT_RUN = 0;	// 0 = single run mode

	AdcRegs.ADCTRL2.all = 0;
	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;	// 1 = enable SEQ1 interrupt
	//AdcRegs.ADCTRL2.bit.INT_ENA_SEQ2 = 1;	// 1 = enable SEQ2 interrupt
	AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1;	// 1 = SEQ1 start from ePWM_SOCA trigger
	//AdcRegs.ADCTRL2.bit.EPWM_SOCB_SEQ2 = 1;	// 1 = SEQ2 start from ePWM_SOCB trigger

	AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = 0;	// 0 = interrupt after every end of sequence
	//AdcRegs.ADCTRL2.bit.INT_MOD_SEQ2 = 0;	// 0 = interrupt after every end of sequence

	AdcRegs.ADCTRL3.bit.ADCCLKPS = 8;	 // ADC clock: FCLK = HSPCLK / 2 * ADCCLKPS
											 // HSPCLK = 75MHz (see DSP2833x_SysCtrl.c)
											 // FCLK = 12.5 MHz

	AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0x000F;     // 16 conversions from both sequencers

	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0;  //0: I_PWM 1, 1: I_PWM2, 8: I_PWM3
	AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 11; //Corriente de entrada (paneles).
	AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 9;  //Corriente de salida (bus).
	AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 11; //Corriente de entrada (paneles).
	AdcRegs.ADCCHSELSEQ2.bit.CONV04 = 10; //Corriente de bateria
	AdcRegs.ADCCHSELSEQ2.bit.CONV05 = 11; //Corriente de entrada (paneles).
	AdcRegs.ADCCHSELSEQ2.bit.CONV06 = 2;  //Tension de entrada (paneles).
	AdcRegs.ADCCHSELSEQ2.bit.CONV07 = 11; //Corriente de entrada (paneles).

	//AdcRegs.ADCMAXCONV.bit.MAX_CONV2 = 0x0007;     // 8 conversions from Sequencer 2
	AdcRegs.ADCCHSELSEQ3.bit.CONV08 = 3;  //Tension de salida (bus).
	AdcRegs.ADCCHSELSEQ3.bit.CONV09 = 11; //Corriente de entrada (paneles).
	AdcRegs.ADCCHSELSEQ3.bit.CONV10 = 9; //Corriente de salida (bus).
	AdcRegs.ADCCHSELSEQ3.bit.CONV11 = 11; //Corriente de entrada (paneles).
	AdcRegs.ADCCHSELSEQ4.bit.CONV12 = 3; //Tension de salida (bus).
	AdcRegs.ADCCHSELSEQ4.bit.CONV13 = 11; //Corriente de entrada (paneles).
	AdcRegs.ADCCHSELSEQ4.bit.CONV14 = 1; //0: I_PWM 1, 1: I_PWM2, 8: I_PWM3
	AdcRegs.ADCCHSELSEQ4.bit.CONV15 = 11; //Corriente de entrada (paneles).

	EPwm1Regs.ETPS.all = 0x1100;			// Configure ADC start by ePWM1
	/*
 	 bit 15-14     00:     EPWMxSOCB, read-only
	 bit 13-12     01:     SOCBPRD, 01 = generate SOCB on first event
	 bit 11-10     00:     EPWMxSOCA, read-only
	 bit 9-8       01:     SOCAPRD, 01 = generate SOCA on first event
	 bit 7-4       0000:   reserved
	 bit 3-2       00:     INTCNT, don't care
	 bit 1-0       00:     INTPRD, don't care
	*/
	EPwm1Regs.ETSEL.all = 0x4C00;			// Enable SOCA to ADC
	/*
	 bit 15        0:      SOCBEN, 1 = enable SOCB
	 bit 14-12     100:    SOCBSEL, 010 = SOCB on PRD event, 001 = SOCB on CTR = 0, 100 = SOCB on CTR = CMPA
	 bit 11        1:      SOCAEN, 1 = enable SOCA
	 bit 10-8      100:    SOCASEL, 010 = SOCA on PRD event, 100 = SOCA on CTR = CMPA
	 bit 7-4       0000:   reserved
	 bit 3         0:      INTEN, 0 = disable interrupt
	 bit 2-0       000:    INTSEL, don't care
     */}

void Muestreo(void){
	MuestreoIQ(&vIQ);
	variablesIQ2F(&vIQ,&vF);
}

void MuestreoIQ(){
	volatile Uint16 *p;
	int16 i;
	vIQ.Ii = 0;
	p = &AdcMirror.ADCRESULT1;

	#if 1
	for(i = 0; i < 8; i++){
		vIQ.Ii += *p;
		p+=2;
	}
	vIQ.Ii >>= 3;
	vIQ.Ii = _IQ(vIQ.Ii*K_IQ_4095);
	#else
	for(i = 0; i < 8; i++){
		vIQ.Ii += _IQmpy(_IQ(*p * K_IQ_4095),_IQ(*p * K_IQ_4095));
		p++;
	}
	vIQ.Ii = _IQsqrt(vIQ.Ii);
	#endif

	vIQ.Vo = _IQ(AdcMirror.ADCRESULT8*K_IQ_4095);
	vIQ.Vi = _IQ(AdcMirror.ADCRESULT6*K_IQ_4095);
	vIQ.Io = _IQ(AdcMirror.ADCRESULT2*K_IQ_4095-OFFSET_CORRIENTE_BUS*K_IQ_4095);
	vIQ.Po = _IQmpy(vIQ.Vo,vIQ.Io);
	vIQ.Pi = _IQmpy(vIQ.Vi,vIQ.Ii);
	vIQ.Ibat = _IQ(AdcMirror.ADCRESULT4*K_IQ_2047-OFFSET_CORRIENTE_BAT*K_IQ_2047);
	vIQ.I1 = _IQ(AdcMirror.ADCRESULT0*K_IQ_4095);
	vIQ.I2 = _IQ(AdcMirror.ADCRESULT14*K_IQ_4095);
	vIQ.I3 = 0;//_IQ(AdcMirror.ADCRESULT0*K_IQ_4095);
}

void variablesIQ2F(){
	vF.Vo = IQ2F(vIQ.Vo) * BASE_PU_TENSION;
	vF.Vi = IQ2F(vIQ.Vi) * BASE_PU_TENSION;
	vF.Io = IQ2F(vIQ.Io) * BASE_PU_CORRIENTE;
	vF.Ii = IQ2F(vIQ.Ii) * BASE_PU_CORRIENTE;
	vF.Po = vF.Vo * vF.Io;
	vF.Pi = vF.Vi * vF.Ii;
	vF.Ibat = IQ2F(vIQ.Ibat) * BASE_PU_CORRIENTE_BAT;
	vF.I1 = IQ2F(vIQ.I1) * BASE_PU_CORRIENTE;
	vF.I2 = IQ2F(vIQ.I2) * BASE_PU_CORRIENTE;
	vF.I3 = IQ2F(vIQ.I3) * BASE_PU_CORRIENTE;
}

void medicionCorriente(){
	int16 i;
	volatile Uint16 *p;
	p = &AdcMirror.ADCRESULT1;
	for(i = 0; i < 8; i++){
		waveform[i] = *p * K_IQ_4095 * BASE_PU_CORRIENTE;
		p+=2;
	}
}
