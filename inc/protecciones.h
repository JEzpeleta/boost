#ifndef INC_PROTECCIONES_H_
#define INC_PROTECCIONES_H_

#define UMAX_BUS 330.0
#define IMAX_BUS 3.0
#define IMAX_I 7.0
#define IMAX_PWM 7.0
#define IMAX_BAT 3.0

typedef struct{
	float VoMax;
	float IoMax;
	float IiMax;
	float IpwmMax;
	float IbatMax;
}limites;

typedef struct{
	int Vo;
	int Io;
	int Ii;
	int Ipwm;
	int Ibat;
}flags;

#define NORMAL 0
#define PROTECCION_ON 1

void Setup_Protecciones(void);
int ComprobarLimites(void);

#endif /* INC_PROTECCIONES_H_ */
