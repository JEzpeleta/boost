#ifndef INC_MAIN_H_
#define INC_MAIN_H_

#include "DSP2833x_Device.h"
#include "pwm.h"
#include "gpio.h"
#include "controles.h"
#include "pid.h"
#include "interrupciones.h"
#include "protecciones.h"
#include "salida.h"
#include "ensayos.h"
#include "adc.h"

#define NORMAL 0
#define PROTEGIDO 1

#define SCALER_ACTUALIZACIONES 1
#define SCALER_TEST 1

extern void InitAdc(void);
extern void InitSysCtrl(void);
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitCpuTimers(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);
void SCIA_Init(long,long);

#endif /* INC_MAIN_H_ */
