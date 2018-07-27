#include "protecciones.h"
#include "DSP2833x_Device.h"
#include "adc.h"

extern variablesF vF;
volatile limites limitesProteccion = {UMAX_BUS,IMAX_BUS,IMAX_I,IMAX_PWM,IMAX_BAT};
volatile flags flagsProtecciones = {0,0,0,0,0};

void Setup_Protecciones(void)
{
// Configure Trip Mechanism for the Motor control software
// -Cycle by cycle trip on CPU halt
// -One shot IPM trip zone trip
// These trips need to be repeated for EPWM1 ,2 & 3

//===========================================================================
//Motor Control Trip Config, EPwm1,2,3
//===========================================================================
      EALLOW;
    // CPU Halt Trip
    //  EPwm1Regs.TZSEL.bit.CBC6=0x1;
    //  EPwm2Regs.TZSEL.bit.CBC6=0x1;
    //  EPwm3Regs.TZSEL.bit.CBC6=0x1;

      EPwm1Regs.TZSEL.bit.OSHT1   = 1;  //enable OSHT
      EPwm2Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
      EPwm3Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT

// What do we want the OST/CBC events to do?
// TZA events can force EPWMxA
// TZB events can force EPWMxB

      EPwm1Regs.TZCTL.bit.TZA = 0xAFFF; // EPWMxA will go low
      EPwm1Regs.TZCTL.bit.TZB = 0xAFFF; // EPWMxB will go low

      EPwm2Regs.TZCTL.bit.TZA = 0xAFFF; // EPWMxA will go low
      EPwm2Regs.TZCTL.bit.TZB = 0xAFFF; // EPWMxB will go low

      EPwm3Regs.TZCTL.bit.TZA = 0xAFFF; // EPWMxA will go low
      EPwm3Regs.TZCTL.bit.TZB = 0xAFFF; // EPWMxB will go low

      EDIS;

     // Clear any spurious OV trip
      EPwm1Regs.TZCLR.bit.OST = 1;
      EPwm2Regs.TZCLR.bit.OST = 1;
      EPwm3Regs.TZCLR.bit.OST = 1;
}

int ComprobarLimites(void)
{
	if(vF.Io > limitesProteccion.IoMax) {flagsProtecciones.Io = 1; return 1;}
	if(vF.Ii > limitesProteccion.IiMax) {flagsProtecciones.Ii = 1; return 1;}
	if(vF.Ibat > limitesProteccion.IbatMax) {flagsProtecciones.Ibat = 1; return 1;}
	if(vF.Vo > limitesProteccion.VoMax) {flagsProtecciones.Vo = 1; return 1;}
	if(vF.I1 > limitesProteccion.IpwmMax) {flagsProtecciones.Ipwm = 1; return 1;}
	if(vF.I2 > limitesProteccion.IpwmMax) {flagsProtecciones.Ipwm = 1; return 1;}
	if(vF.I3 > limitesProteccion.IpwmMax) {flagsProtecciones.Ipwm = 1; return 1;}
	return 0;
}
