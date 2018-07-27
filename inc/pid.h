#ifndef INC_PID_H_

#define DEFAULT_JE_IQ_TERMINALS {_IQ(0.0),_IQ(0.0),_IQ(0.0)}
#define DEFAULT_JE_IQ_PARAMETERS {_IQ(1.0),_IQ(0.0),_IQ(0.0),_IQ(1.0),_IQ(-1.0)}
#define DEFAULT_JE_IQ_DATA {_IQ(0.0),_IQ(0.0),_IQ(0.0),_IQ(0.0)}
#define ANTI_WINDUP_OFFSET (_IQ(0.05))

typedef struct {
	int32  Ref;  		// Input: reference set-point
	int32  Fbk;   		// Input: feedback
	int32  Out;   		// Output: controller output
} PID_JE_IQ_TERMINALS;

typedef struct {
	int32  Kp;			// Parameter: proportional loop gain
	int32  Ki;			// Parameter: integral gain
	int32  Kc; 		    // Parameter: anti-windup gain
	int32  Umax;		// Parameter: upper saturation limit
	int32  Umin;		// Parameter: lower saturation limit
} PID_JE_IQ_PARAMETERS;

typedef struct {
	int32  Err;			// Data: error term
	int32  Ui;			// Data: integral term
	int32  W1;			// Data: integral enable term
	int32  OutPreSat;	// Data: controller output before internal saturation
} PID_JE_IQ_DATA;

typedef struct {
	PID_JE_IQ_TERMINALS term;
	PID_JE_IQ_PARAMETERS param;
	PID_JE_IQ_DATA	data;
} PID_JE_IQ_CONTROLLER;

typedef PID_JE_IQ_CONTROLLER	*PID_JE_HANDLE;

extern PID_JE_IQ_CONTROLLER pid_Vo;
extern PID_JE_IQ_CONTROLLER pid_Vi;
extern PID_JE_IQ_CONTROLLER pid_Ibat;
extern PID_JE_IQ_CONTROLLER pid_Ii;

void PID_JE_IQ_FUNC(PID_JE_HANDLE v);

#endif /* INC_PID_H_ */
