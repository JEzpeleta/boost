#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "iq.h"

#define BASE_PU_CORRIENTE_BAT 15.151515
#define BASE_PU_CORRIENTE 10
#define BASE_PU_TENSION 339
#define K_IQ_4095 0.0002442
#define K_IQ_2047 0.0004819
#define OFFSET_CORRIENTE_BAT 2022
#define OFFSET_CORRIENTE_BUS 48

typedef struct variablesIQ{
	_iq Vi;
	_iq Ii;
	_iq Pi;
	_iq Ibat;
	_iq Io;
	_iq Vo;
	_iq Po;
	_iq I1;
	_iq I2;
	_iq I3;
}variablesIQ;

typedef struct variablesF{
	float Vi;
	float Ii;
	float Pi;
	float Ibat;
	float Io;
	float Vo;
	float Po;
	float I1;
	float I2;
	float I3;
}variablesF;

extern variablesIQ vIQ;
extern variablesF vF;

void Setup_ADC(void);
void Muestreo(void);
void medicionCorriente(void);

#endif /* INC_ADC_H_ */
