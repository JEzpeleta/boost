#ifndef INC_ENSAYOS_H_
#define INC_ENSAYOS_H_

#include "dlog.h"

#define FDLOG_BUFFER_SIZE 0x100

#define START 2
#define RUN 1
#define STOP 0

typedef struct
{
	FDLOG Ii;
	FDLOG Iref;
}LOG;

typedef struct
{
	float buf_Ii[FDLOG_BUFFER_SIZE];
	float buf_Iref[FDLOG_BUFFER_SIZE];
}DATA_BUFFER;

void Setup_FDatalog(void);
void LoopTest(void);

#endif /* INC_ENSAYOS_H_ */
