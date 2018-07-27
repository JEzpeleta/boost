#include "main.h"
#include "ensayos.h"
#include "adc.h"

LOG datalog;
DATA_BUFFER buffer;

typedef struct
{
	float Ii;
} indices;

indices itae = {0.0};

typedef struct{
	float Iref;
}referencias;

referencias testref = {0.0};

int32 ENSAYO = STOP;

void Setup_FDatalog(void)
{
	DCL_createLog(&datalog.Iref, buffer.buf_Iref, FDLOG_BUFFER_SIZE);
	DCL_createLog(&datalog.Ii, buffer.buf_Ii, FDLOG_BUFFER_SIZE);
}

void LoopTest(void)
{
	if(ENSAYO == START)
	{
		IiRef = testref.Iref;
		ENSAYO = RUN;
	}

	if(ENSAYO == RUN)
	{
		DCL_writeLog(&datalog.Iref, IiRef);
		DCL_writeLog(&datalog.Ii, vF.Ii);
		if(FDLOG_SPACE(datalog.Iref)==FDLOG_BUFFER_SIZE){
			ENSAYO = STOP;
			itae.Ii = DCL_runITAE(&datalog.Iref, &datalog.Ii, 1.0/15000);
		}
	}
}
