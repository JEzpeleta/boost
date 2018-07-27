#ifndef INC_IQ_H_
#define INC_IQ_H_

#define MATH_TYPE 0

#ifndef DSP28_DATA_TYPES
#define DSP28_DATA_TYPES
typedef int             	int16;
typedef long            	int32;
typedef long long			int64;
typedef unsigned int		Uint16;
typedef unsigned long		Uint32;
typedef unsigned long long	Uint64;
typedef float           	float32;
typedef long double     	float64;
#endif

#include "IQmathLib.h"
#include "MPPT_PNO_IQ.h"
#include "PID_GRANDO_IQ.h"

inline float IQ2F(_iq x){
	return x/(float)(16777216.0);
}

#endif /* INC_IQ_H_ */
