#ifndef INC_CONTROLES_H_
#define INC_CONTROLES_H_

#include "iq.h"

void Control();

extern volatile int32 IiReg;
extern volatile float IiRef;
extern volatile int32 doMPPT;
extern volatile int32 IbatReg;
extern volatile float IbatRef;
extern volatile int32 VoReg;
extern volatile float VoRef;
extern volatile int32 ViReg;
extern volatile float ViRef;

#endif /* INC_CONTROLES_H_ */
