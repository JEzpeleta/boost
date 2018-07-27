#ifndef LACAN_H
#define LACAN_H

#define MATH_TYPE 0
#include "DSP2833x_Device.h"
#include "IQmathLib.h"

#define LACAN_LOCAL_ID 0x02
#define LACAN_ECANMBOXES ECanaMboxes
#define LACAN_ECANREGS ECanaRegs
#define LACAN_LAMREGS ECanaLAMRegs
#define LACAN_MOTOREGS ECanaMOTORegs
#define LACAN_TOTAL_MBOXES 32
#define LACAN_SEND_MBOXES 16
#define LACAN_RECEIVE_MBOXES ((LACAN_TOTAL_MBOXES)-(LACAN_SEND_MBOXES))
#define LACAN_MBOX_SET_MASK(n) ((Uint32) 0x00000001 << (n))
#define LACAN_MBOX_RESET_MASK(n) ((Uint32) 0xffffffff ^ (Uint32) (LACAN_MBOX_SET_MASK((n))))
#define LACAN_FUNCTION_BITS 6
#define LACAN_IDENT_BITS 5
#define LACAN_IDENT_MASK 0x1f
#if (LACAN_IDENT_BITS + LACAN_FUNCTION_BITS != 11)
	#error LACAN_IDENT_BITS + LACAN_FUNCTION_BITS != 11
#endif
#define LACAN_BYTE0_RESERVED (8-(LACAN_IDENT_BITS))
#define LACAN_BYTE0_RESERVED_MASK (0x3)
#define LACAN_BUFFER_SIZE 8
#define LACAN_TIMEOUT ((Uint32) 10000000)

// Diccionario de direcciones (ID). 5 bits (0x00 a 0x1f)
#define LACAN_ID_BROADCAST        0x00
#define LACAN_ID_MASTER           0x01
#define LACAN_ID_GEN              0x02
#define LACAN_ID_BOOST            0x03

// Diccionario de funciones (FUN). 6 bits (0x00 a 0x3f)
#define LACAN_FUN_ERR             0x01
#define LACAN_FUN_DO              0x04
#define LACAN_FUN_SET             0x08
#define LACAN_FUN_QRY             0x0C
#define LACAN_FUN_ACK             0x10
#define LACAN_FUN_POST            0x14
#define LACAN_FUN_HB              0x18

// Diccionario de comandos (CMD). 8 bits (0x00 a 0xff)
#define LACAN_CMD_TRIP            0x08
#define LACAN_CMD_STOP            0x0f
#define LACAN_CMD_RESET           0x10
#define LACAN_CMD_COUPLE          0x18
#define LACAN_CMD_START           0x1f
#define LACAN_CMD_DECOUPLE        0x28
#define LACAN_CMD_MAGNETIZE       0x2f

// Diccionario de variables (VAR). 8 bits (0x00 a 0xff)
#define LACAN_VAR_STATUS          0x02
#define LACAN_VAR_I_MAX           0x08
#define LACAN_VAR_I_MIN           0x09
#define LACAN_VAR_I               0x0A
#define LACAN_VAR_I_SETP          0x0B
#define LACAN_VAR_P_MAX           0x10
#define LACAN_VAR_P_MIN           0x11
#define LACAN_VAR_P               0x12
#define LACAN_VAR_P_SETP          0x13
#define LACAN_VAR_V_MAX           0x18
#define LACAN_VAR_V_MIN           0x19
#define LACAN_VAR_V               0x1A
#define LACAN_VAR_V_SETP          0x1B
#define LACAN_VAR_VI              0x1C
#define LACAN_VAR_VI_SETP         0x1D
#define LACAN_VAR_W_MAX           0x20
#define LACAN_VAR_W_MIN           0x21
#define LACAN_VAR_W               0x22
#define LACAN_VAR_W_SETP          0x23
#define LACAN_VAR_BAT_IMAX        0x28
#define LACAN_VAR_BAT_IMIN        0x29
#define LACAN_VAR_BAT_VMAX        0x2A
#define LACAN_VAR_BAT_VMIN        0x2B

// Diccionario de resultados (RES). 8 bits (0x00 a 0xff)
#define LACAN_RES_OK              0x00
#define LACAN_RES_MISSING_PREREQ  0x01
#define LACAN_RES_RECEIVED        0x02
#define LACAN_RES_NOT_IMPLEMENTED 0x08
#define LACAN_RES_OUT_OF_RANGE    0x10
#define LACAN_RES_BUSY            0x18
#define LACAN_RES_DENIED          0x20
#define LACAN_RES_GENERIC_FAILURE 0x28

// Diccionario de errores (ERR). 8 bits (0x00 a 0xff)
#define LACAN_ERR_GENERIC_ERR     0x00
#define LACAN_ERR_OVERVOLTAGE     0x08
#define LACAN_ERR_UNDERVOLTAGE    0x10
#define LACAN_ERR_OVERCURRENT     0x18
#define LACAN_ERR_BAT_OVERCURRENT 0x19
#define LACAN_ERR_OVERTEMPERATURE 0x20
#define LACAN_ERR_OVERSPEED       0x28
#define LACAN_ERR_UNDERSPEED      0x30
#define LACAN_ERR_NO_HEARTBEAT    0x38
#define LACAN_ERR_INTERNAL_TRIP   0x39
#define LACAN_ERR_EXTERNAL_TRIP   0x40

// Diccionario de tipos de solicitudes (REQ). 3 bits (0x0 a 0x7)
#define LACAN_REQ_SET             0x01
#define LACAN_REQ_DO              0x02
#define LACAN_REQ_QRY             0x03

// Códigos de error del protocolo
#define LACAN_SUCCESS             0
#define LACAN_FAILURE             -1
#define LACAN_NO_SUCH_MSG         -2
#define LACAN_NO_BOXES            -3

struct LACAN_MSG{
   Uint16 PENDING:1;
   Uint16 DLC:4;
   Uint16 STDMSGID:11;
   Uint16 BYTE0:8;
   Uint16 BYTE1:8;
   Uint16 BYTE2:8;
   Uint16 BYTE3:8;
   Uint16 BYTE4:8;
   Uint16 BYTE5:8;
   Uint16 BYTE6:8;
   Uint16 BYTE7:8;
};

typedef struct LACAN_MSG LACAN_MSG;

typedef union {
  Uint32 u32;
  int32 i32;
  float f;
  _iq iq;
 } LACAN_data_32;

void LACAN_Init(void);
int16 LACAN_Error(Uint16 errCode);
int16 LACAN_Do(Uint16 destination, Uint16 command);
int16 LACAN_Set(Uint16 destination, Uint16 variable, Uint16 data);
int16 LACAN_Heartbeat(void);
int16 LACAN_Acknowledge(Uint16 requester, Uint16 requestType, Uint16 object, Uint16 result);
int16 LACAN_Query(Uint16 destination, Uint16 variable);
int16 LACAN_Post(Uint16 destination, Uint16 variable, Uint16 data);

int16 LACAN_Poll(void);

interrupt void ecan0intA_isr(void);
interrupt void ecan1intA_isr(void);

extern volatile int processedMsgs;

#endif /* LACAN_H */
