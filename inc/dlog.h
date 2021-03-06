#ifndef INC_DCL_FDLOG_H
#define INC_DCL_FDLOG_H

typedef volatile struct {
	float *fptr;
	float *lptr;
	float *dptr;
} FDLOG;	

#define FDLOG_DEFAULT_SIZE	0x0400

#define FDLOG_DEFAULTS { 	0x00000000,	\
							0x00000000,	\
							0x00000000	\
						}

#define FDLOG_SPACE(buf)		( (buf).lptr - (buf).dptr + 1 )		// space remaining
#define FDLOG_SIZE(buf)			( (buf).lptr - (buf).fptr + 1 )		// buffer size
#define FDLOG_ELEMENT(buf) 		( (buf).dptr - (buf).fptr + 1 )		// current element
#define FDLOG_START(buf)		( (buf).dptr == (buf).fptr )		// start of buffer
#define FDLOG_END(buf)			( (buf).dptr == (buf).lptr )		// end of buffer
#define FDLOG_OUT_OF_RANGE(buf)	( ((buf).dptr < (buf).fptr) || ((buf).dptr > (buf).lptr) )	// over/under-range check
#define FDLOG_EXISTS(buf)		( (buf).lptr != (buf).fptr )		// check if entity exists

static inline void DCL_clearLog(FDLOG *p);
static inline float DCL_writeLog(FDLOG *p, float data);
static inline void DCL_createLog(FDLOG *p, float *addr, unsigned int buflen);
static inline void DCL_resetLog(FDLOG *p);
static inline void DCL_deleteLog(FDLOG *p);
static inline void DCL_fillLog(FDLOG *p, float data);
static inline float DCL_readLog(FDLOG *p);
static inline float DCL_runITAE(FDLOG *rlog, FDLOG *ylog, float prd);

static inline void DCL_deleteLog(FDLOG *p)	{ p->dptr = p->lptr = p->fptr = 0; }

static inline void DCL_resetLog(FDLOG *p)	{ p->dptr = p->fptr; }

static inline void DCL_createLog(FDLOG *p, float *addr, unsigned int buflen)
{
	p->fptr = addr;
	p->lptr = addr + buflen - 1;
	p->dptr = p->fptr;
}

static inline float DCL_writeLog(FDLOG *p, float data)
{
	float d0 = *(p->dptr);

	*(p->dptr) = data;
	if (p->dptr >= p->lptr)
		p->dptr = p->fptr;
	else
		p->dptr++;
	return d0;
}

static inline void DCL_clearLog(FDLOG *p)
{
	p->dptr = p->lptr;
	*(p->dptr) = 0;
	while (p->dptr > p->fptr)
	{
		--p->dptr;
		*(p->dptr) = 0;
	}
}

static inline void DCL_fillLog(FDLOG *p, float data)
{
	p->dptr = p->lptr;
	*(p->dptr) = 0;
	while (p->dptr > p->fptr)
	{
		--p->dptr;
		*(p->dptr) = data;
	}
}


static inline float DCL_readLog(FDLOG *p)
{
	float fl = *(p->dptr);

	if (p->dptr >= p->lptr)
		p->dptr = p->fptr;
	else
		p->dptr++;

	return fl;
}

static inline float DCL_runITAE(FDLOG *rlog, FDLOG *ylog, float prd)
{
	float i = 0.0f;
	float *rch = rlog->dptr;
	float *ych = ylog->dptr;
	float rlt = 0.0f;
	
	rlog->dptr = rlog->fptr;
	ylog->dptr = ylog->fptr;	
	while (rlog->dptr <= rlog->lptr)
	{
		rlt += fabs(*(rlog->dptr) - *(ylog->dptr)) * i;
		i += prd;
		rlog->dptr++;
		ylog->dptr++;
	}
	
	rlog->dptr = rch;
	ylog->dptr = ych;
	
	return rlt;
}

#endif // INC_DCL_FDLOG_H
