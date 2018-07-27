#include "LACAN.h"
#include "adc.h"
#include "controles.h"
#include "main.h"

int16 LACAN_DO_Handler(Uint16 source, Uint16 command){
	if(source == LACAN_ID_MASTER){
		switch(command){
		case LACAN_CMD_START:
			LACAN_Acknowledge(source, LACAN_REQ_DO, command, LACAN_RES_OK);
			break;
		case LACAN_CMD_STOP:
			LACAN_Acknowledge(source, LACAN_REQ_DO, command, LACAN_RES_OK);
			break;
		default:
			LACAN_Acknowledge(source, LACAN_REQ_DO, command, LACAN_RES_NOT_IMPLEMENTED);
		}
	}else{
		LACAN_Acknowledge(source, LACAN_REQ_DO, command, LACAN_RES_DENIED);
	}
	processedMsgs++;
	return 0;
}

int16 LACAN_SET_Handler(Uint16 source, Uint16 variable, LACAN_data_32 data){
	if(source == LACAN_ID_MASTER){
		switch(variable){
		case LACAN_VAR_VI_SETP:
			if(data.iq >= _IQ(0) && data.iq <= _IQ(0.2)){
				ViRef = data.iq;
				LACAN_Acknowledge(source, LACAN_REQ_SET, variable, LACAN_RES_OK);
			}else{
				LACAN_Acknowledge(source, LACAN_REQ_SET, variable, LACAN_RES_OUT_OF_RANGE);
			}
			break;
		default:
			LACAN_Acknowledge(source, LACAN_REQ_SET, variable, LACAN_RES_NOT_IMPLEMENTED);
		}
	}else{
		LACAN_Acknowledge(source, LACAN_REQ_SET, variable, LACAN_RES_DENIED);
	}
	processedMsgs++;
	return 0;
}

int16 LACAN_QRY_Handler(Uint16 source, Uint16 variable){
	if(source == LACAN_ID_MASTER){
		switch(variable){
		case LACAN_VAR_I:
			LACAN_Post(source, variable, vF.Ii);
		default:
			LACAN_Acknowledge(source, LACAN_REQ_QRY, variable, LACAN_RES_NOT_IMPLEMENTED);
		}
	}else{
		LACAN_Acknowledge(source, LACAN_REQ_SET, variable, LACAN_RES_DENIED);
	}
	processedMsgs++;
	return 0;
}
