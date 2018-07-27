#include "DSP28x_Project.h"
#include "LACAN.h"

Uint16 function, source, destination, length, it, rotate;
LACAN_data_32 tmp_32;
volatile Uint32 LACAN_Busy = (Uint32) 0xffffffff << (LACAN_SEND_MBOXES);
volatile Uint16 LACAN_queue_index = 0;
volatile struct ECAN_REGS LACAN_ECanShadow;
volatile struct MBOX *LACAN_Mbox_ptr;
volatile union CANLAM_REG *LACAN_LAMReg_ptr;
volatile Uint32 *LACAN_MOTOReg_ptr;
volatile struct LACAN_MSG LACAN_queue[LACAN_BUFFER_SIZE];
volatile int tmpCounter = 0;
volatile int processedMsgs = 0;
volatile int handledMsgs = 0;

void InitECana(void);
extern int16 LACAN_Msg_Handler(Uint16 queueIndex);
extern int16 LACAN_DO_Handler(Uint16 source, Uint16 command);
extern int16 LACAN_SET_Handler(Uint16 source, Uint16 variable, LACAN_data_32 data);
extern int16 LACAN_ACK_Handler(Uint16 source, Uint16 requestType, Uint16 object);
extern int16 LACAN_QRY_Handler(Uint16 source, Uint16 variable);
extern int16 LACAN_POST_Handler(Uint16 source, Uint16 variable, LACAN_data_32 data);
extern int16 LACAN_HB_Handler(Uint16 source);
extern int16 LACAN_ERR_Handler(Uint16 source, Uint16 errCode);

void LACAN_Init(void){
	InitECana(); // Initialize eCAN-A module

	// Configure the first LACAN_SEND_MBOXES mailboxes as Tx and all others as Rx
	// Since this write is to the entire register (instead of a bit
	// field) a shadow register is not required.
	LACAN_ECANREGS.CANMD.all = (Uint32) 0xffffffff << LACAN_SEND_MBOXES;

	// Temporarily disable all mailboxes
	// Since this write is to the entire register (instead of a bit
	// field) a shadow register is not required.
	LACAN_ECANREGS.CANME.all = 0x00000000;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + LACAN_SEND_MBOXES;
	LACAN_LAMReg_ptr = &LACAN_LAMREGS.LAM0 + LACAN_SEND_MBOXES;

	for(it=LACAN_SEND_MBOXES;it<LACAN_TOTAL_MBOXES;it++){
		LACAN_Mbox_ptr->MSGID.bit.IDE = 0;
		LACAN_Mbox_ptr->MSGID.bit.AME = 1;
		LACAN_LAMReg_ptr->bit.LAM_H = 8191;
		LACAN_LAMReg_ptr->bit.LAMI = 1;
		LACAN_Mbox_ptr++;
		LACAN_LAMReg_ptr++;
	}

	// Since this write is to the entire register (instead of a bit
	// field) a shadow register is not required.
	EALLOW;
	ECanaRegs.CANMIM.all = 0xffffffff;
	ECanaRegs.CANMIL.all = 0xffffffff;
	ECanaRegs.CANOPC.all = 0xffffffff;

	LACAN_ECanShadow.CANGIM.all = 0x0;
	LACAN_ECanShadow.CANGIM.bit.I0EN = 1; // Enable I0EN
	LACAN_ECanShadow.CANGIM.bit.I1EN = 1; // Enable I1EN
	LACAN_ECanShadow.CANGIM.bit.MTOM = 1; // Enable MBX Timeout interrupt //Time-out related
	LACAN_ECanShadow.CANGIM.bit.AAIM = 1; // Enable Abort Acknowledge interrupt //Time-out related
	ECanaRegs.CANGIM.all = LACAN_ECanShadow.CANGIM.all;

	// Enable all mailboxes
	// Since this write is to the entire register (instead of a bit
	// field) a shadow register is not required.
	LACAN_ECANREGS.CANME.all = 0xffffffff;

	// Configure the eCAN for self test mode
	// Enable the enhanced features of the eCAN.
	EALLOW;
	LACAN_ECanShadow.CANMC.all = ECanaRegs.CANMC.all;
	//LACAN_ECanShadow.CANMC.bit.STM = 1;    // Configure CAN for self-test mode
	ECanaRegs.CANMC.all = LACAN_ECanShadow.CANMC.all;
	PieVectTable.ECAN1INTA = &ecan1intA_isr;	// re - map CAN INT1
	PieVectTable.ECAN0INTA = &ecan0intA_isr;	// re - map CAN INT0
	EDIS;

	PieCtrlRegs.PIEIER9.bit.INTx5 = 1;  // ECAN0INTA
	PieCtrlRegs.PIEIER9.bit.INTx6 = 1;  // ECAN1INTA

	for(it=0;it<LACAN_BUFFER_SIZE;it++){
		LACAN_queue[it].PENDING=0;
	}
}

int16 LACAN_Claim_Mbox(void){
	for(it = 0;it < LACAN_SEND_MBOXES;it++)
	{
		if((~LACAN_Busy) & (0x1 << it)){
			LACAN_Busy |= (0x1 << it);
			return it;
		}
	}
	return LACAN_NO_BOXES;
}

int16 _LACAN_Error(Uint16 mBox, Uint16 errCode){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_ERR << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 2;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = LACAN_ID_BROADCAST << LACAN_BYTE0_RESERVED;
	LACAN_Mbox_ptr->MDL.byte.BYTE1 = errCode;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	LACAN_MOTOReg_ptr = &LACAN_MOTOREGS.MOTO0 + mBox;

	*LACAN_MOTOReg_ptr = LACAN_ECANREGS.CANTSC + LACAN_TIMEOUT; //Time-out related

	LACAN_ECANREGS.CANTOC.all |= LACAN_MBOX_SET_MASK(mBox); //Time-out related

	return LACAN_SUCCESS;
}

int16 _LACAN_Do(Uint16 mBox, Uint16 destination, Uint16 command){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_DO << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 2;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = destination << LACAN_BYTE0_RESERVED;
	LACAN_Mbox_ptr->MDL.byte.BYTE1 = command;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	return LACAN_SUCCESS;
}

int16 _LACAN_Set(Uint16 mBox, Uint16 destination, Uint16 variable, Uint16 data){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_SET << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 3;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = destination << LACAN_BYTE0_RESERVED;
	LACAN_Mbox_ptr->MDL.byte.BYTE1 = variable;
	LACAN_Mbox_ptr->MDL.byte.BYTE2 = data;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	return LACAN_SUCCESS;
}

int16 _LACAN_Heartbeat(Uint16 mBox){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_HB << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 1;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = LACAN_ID_BROADCAST << LACAN_BYTE0_RESERVED;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	return LACAN_SUCCESS;
}

int16 _LACAN_Acknowledge(Uint16 mBox, Uint16 requester, Uint16 requestType, Uint16 object, Uint16 result){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_ACK << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 3;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = (requester << LACAN_BYTE0_RESERVED) + (requestType & LACAN_BYTE0_RESERVED_MASK);
	LACAN_Mbox_ptr->MDL.byte.BYTE1 = object;
	LACAN_Mbox_ptr->MDL.byte.BYTE2 = result;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	return LACAN_SUCCESS;
}

int16 _LACAN_Query(Uint16 mBox, Uint16 destination, Uint16 variable){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_QRY << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 2;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = destination << LACAN_BYTE0_RESERVED;
	LACAN_Mbox_ptr->MDL.byte.BYTE1 = variable;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	return LACAN_SUCCESS;
}

int16 _LACAN_Post(Uint16 mBox, Uint16 destination, Uint16 variable, Uint16 data){
	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all & LACAN_MBOX_RESET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr = &LACAN_ECANMBOXES.MBOX0 + mBox;

	LACAN_Mbox_ptr->MSGID.bit.IDE = 0;

	LACAN_Mbox_ptr->MSGID.bit.STDMSGID = (LACAN_FUN_POST << LACAN_IDENT_BITS) | LACAN_LOCAL_ID;

	LACAN_ECanShadow.CANME.all = LACAN_ECANREGS.CANME.all | LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANME.all = LACAN_ECanShadow.CANME.all;

	LACAN_Mbox_ptr->MSGCTRL.bit.DLC = 3;
	LACAN_Mbox_ptr->MDL.byte.BYTE0 = destination << LACAN_BYTE0_RESERVED;
	LACAN_Mbox_ptr->MDL.byte.BYTE1 = variable;
	LACAN_Mbox_ptr->MDL.byte.BYTE2 = data;

	LACAN_ECanShadow.CANTRS.all = LACAN_MBOX_SET_MASK(mBox);
	LACAN_ECANREGS.CANTRS.all = LACAN_ECanShadow.CANTRS.all;

	return LACAN_SUCCESS;
}

int16 LACAN_Error(Uint16 errCode){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Error(retval,errCode);
}

int16 LACAN_Do(Uint16 destination, Uint16 command){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Do(retval,destination,command);
}

int16 LACAN_Set(Uint16 destination, Uint16 variable, Uint16 data){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Set(retval,destination,variable,data);
}

int16 LACAN_Heartbeat(void){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Heartbeat(retval);
}

int16 LACAN_Acknowledge(Uint16 requester, Uint16 requestType, Uint16 object, Uint16 result){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Acknowledge(retval,requester,requestType,object,result);
}

int16 LACAN_Query(Uint16 destination, Uint16 variable){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Query(retval,destination,variable);
}

int16 LACAN_Post(Uint16 destination, Uint16 variable, Uint16 data){
	int16 retval;
	retval = LACAN_Claim_Mbox();
	if(retval<0) return retval;
	else return _LACAN_Post(retval,destination,variable,data);
}

int16 LACAN_Poll(void){
	Uint16 it;
	for(it=0;it<LACAN_queue_index;it++){
		LACAN_Msg_Handler(it);
		LACAN_queue[it].PENDING = 0;
	}
	LACAN_queue_index = 0;
	return LACAN_SUCCESS;
}

int16 LACAN_Msg_Handler(Uint16 queueIndex){
	function = LACAN_queue[queueIndex].STDMSGID >> LACAN_IDENT_BITS;
	source = LACAN_queue[queueIndex].STDMSGID && LACAN_IDENT_MASK;
	handledMsgs++;
	destination = LACAN_queue[queueIndex].BYTE0 >> LACAN_BYTE0_RESERVED;
	if(destination == LACAN_LOCAL_ID | destination == LACAN_ID_BROADCAST) switch(function){
	case LACAN_FUN_DO:
		return LACAN_DO_Handler(source,LACAN_queue[queueIndex].BYTE1);
	case LACAN_FUN_SET:
		tmp_32.u32 = 0;
		rotate = 0;
		switch(LACAN_queue[queueIndex].DLC){
		case 6:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE5;
			rotate += 8;
		case 5:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE4 << rotate;
			rotate += 8;
		case 4:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE3 << rotate;
			rotate += 8;
		case 3:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE2 << rotate;
			break;
		default:
			return LACAN_FAILURE;
		}
		return LACAN_SET_Handler(source,LACAN_queue[queueIndex].BYTE1,tmp_32);
	case LACAN_FUN_ACK:
		return LACAN_ACK_Handler(source,LACAN_queue[queueIndex].BYTE1,LACAN_queue[queueIndex].BYTE2);
	case LACAN_FUN_QRY:
		return LACAN_QRY_Handler(source,LACAN_queue[queueIndex].BYTE1);
	case LACAN_FUN_POST:
		tmp_32.u32 = 0;
		rotate = 0;
		switch(LACAN_queue[queueIndex].DLC){
		case 6:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE5;
			rotate += 8;
		case 5:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE4 << rotate;
			rotate += 8;
		case 4:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE3 << rotate;
			rotate += 8;
		case 3:
			tmp_32.u32 += (Uint32) LACAN_queue[queueIndex].BYTE2 << rotate;
			break;
		default:
			return LACAN_FAILURE;
		}
		return LACAN_POST_Handler(source,LACAN_queue[queueIndex].BYTE1,tmp_32);
	case LACAN_FUN_ERR:
		return LACAN_ERR_Handler(source,LACAN_queue[queueIndex].BYTE1);
	case LACAN_FUN_HB:
		return LACAN_HB_Handler(source);
	default:
		return LACAN_NO_SUCH_MSG;
	}
	return LACAN_SUCCESS;
}

//int16 LACAN_DO_Handler(Uint16 source, Uint16 command){processedMsgs++; return 0;}
//int16 LACAN_SET_Handler(Uint16 source, Uint16 variable, LACAN_data_32 data){processedMsgs++; return 0;}
int16 LACAN_ACK_Handler(Uint16 source, Uint16 requestType, Uint16 object){processedMsgs++; return 0;}
//int16 LACAN_QRY_Handler(Uint16 source, Uint16 variable){processedMsgs++; return 0;}
int16 LACAN_POST_Handler(Uint16 source, Uint16 variable, LACAN_data_32 data){processedMsgs++; return 0;}
int16 LACAN_HB_Handler(Uint16 source){processedMsgs++; return 0;}
int16 LACAN_ERR_Handler(Uint16 source, Uint16 errCode){processedMsgs++; return 0;}

interrupt void ecan0intA_isr(void){
	unsigned int mailbox_nr;
	tmpCounter++;
	if(LACAN_ECANREGS.CANGIF0.bit.AAIF0){ //Time-out related
		for(mailbox_nr = 0; mailbox_nr < LACAN_SEND_MBOXES; mailbox_nr++) if(LACAN_ECANREGS.CANAA.all & LACAN_MBOX_SET_MASK(mailbox_nr)) break;
		LACAN_ECANREGS.CANAA.all = LACAN_MBOX_SET_MASK(mailbox_nr);
		LACAN_Busy &= ~LACAN_MBOX_SET_MASK(mailbox_nr); //Time-out related
	}
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}

interrupt void ecan1intA_isr(void){
	volatile struct MBOX *mailbox;
	unsigned int mailbox_nr;
	if(LACAN_ECANREGS.CANGIF1.bit.MTOF1){ //Time-out related
		for(mailbox_nr = 0; mailbox_nr < LACAN_SEND_MBOXES; mailbox_nr++) if(LACAN_ECANREGS.CANTOS.all & LACAN_MBOX_SET_MASK(mailbox_nr)) break;
		LACAN_ECanShadow.CANTRR.all = LACAN_MBOX_SET_MASK(mailbox_nr); //Time-out related
		LACAN_ECANREGS.CANTRR.all = LACAN_ECanShadow.CANTRR.all; //Time-out related
	} //Time-out related
  	if(LACAN_ECANREGS.CANGIF1.bit.GMIF1){
  		mailbox_nr = LACAN_ECANREGS.CANGIF1.bit.MIV1;
  		if(mailbox_nr < LACAN_SEND_MBOXES){
  			LACAN_ECANREGS.CANTA.all = LACAN_MBOX_SET_MASK(mailbox_nr);
  			LACAN_Busy &= ~LACAN_MBOX_SET_MASK(mailbox_nr);
  		}else{
  			mailbox = &ECanaMboxes.MBOX0 + mailbox_nr;
			if(mailbox->MSGID.bit.IDE==0){
				if(((mailbox->MDL.byte.BYTE0 >> LACAN_BYTE0_RESERVED) == LACAN_LOCAL_ID) || ((mailbox->MDL.byte.BYTE0 >> LACAN_BYTE0_RESERVED) == LACAN_ID_BROADCAST)){
					if(LACAN_queue_index==LACAN_BUFFER_SIZE){
						LACAN_Msg_Handler(LACAN_BUFFER_SIZE-1);
						LACAN_queue_index--;
					}
					LACAN_queue[LACAN_queue_index].PENDING = 1;
					LACAN_queue[LACAN_queue_index].DLC = mailbox->MSGCTRL.bit.DLC;
					LACAN_queue[LACAN_queue_index].STDMSGID = mailbox->MSGID.bit.STDMSGID;
					switch(LACAN_queue[LACAN_queue_index].DLC){
					case 8:
						LACAN_queue[LACAN_queue_index].BYTE7 = mailbox->MDH.byte.BYTE7;
					case 7:
						LACAN_queue[LACAN_queue_index].BYTE6 = mailbox->MDH.byte.BYTE6;
					case 6:
						LACAN_queue[LACAN_queue_index].BYTE5 = mailbox->MDH.byte.BYTE5;
					case 5:
						LACAN_queue[LACAN_queue_index].BYTE4 = mailbox->MDH.byte.BYTE4;
					case 4:
						LACAN_queue[LACAN_queue_index].BYTE3 = mailbox->MDL.byte.BYTE3;
					case 3:
						LACAN_queue[LACAN_queue_index].BYTE2 = mailbox->MDL.byte.BYTE2;
					case 2:
						LACAN_queue[LACAN_queue_index].BYTE1 = mailbox->MDL.byte.BYTE1;
					case 1:
						LACAN_queue[LACAN_queue_index].BYTE0 = mailbox->MDL.byte.BYTE0;
					}
					LACAN_queue_index++;
				}
			}else{
				tmpCounter++;
			}
			ECanaRegs.CANRMP.all = ((Uint32) 0x00000001 << mailbox_nr);
  		}
  	}
  	PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}

void InitECana(void)		// Initialize eCAN-A module
{
/* Create a shadow register structure for the CAN control registers. This is
 needed, since only 32-bit access is allowed to these registers. 16-bit access
 to these registers could potentially corrupt the register contents or return
 false data. This is especially true while writing to/reading from a bit
 (or group of bits) among bits 16 - 31 */

struct ECAN_REGS ECanaShadow;

	EALLOW;		// EALLOW enables access to protected bits

/* Configure eCAN RX and TX pins for CAN operation using eCAN regs*/

    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;

/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
									// HECC mode also enables time-stamping feature

	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	ECanaShadow.CANMC.bit.SCB = 1;
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

/* Initialize all bits of 'Master Control Field' to zero */
// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
// all bits (including reserved bits) of MSGCTRL must be initialized to zero

    ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX6.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX7.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX8.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX9.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX10.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX11.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX12.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX13.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX14.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX15.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX16.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX17.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX18.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX19.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX20.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX21.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX22.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX23.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX24.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX25.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX26.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX27.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX28.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX29.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX30.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;

// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
//	as a matter of precaution.

	ECanaRegs.CANTA.all	= 0xFFFFFFFF;	/* Clear all TAn bits */

	ECanaRegs.CANRMP.all = 0xFFFFFFFF;	/* Clear all RMPn bits */

	ECanaRegs.CANGIF0.all = 0xFFFFFFFF;	/* Clear all interrupt flag bits */
	ECanaRegs.CANGIF1.all = 0xFFFFFFFF;


/* Configure bit timing parameters for eCANA*/
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	ECanaShadow.CANMC.bit.CCR = 1 ;            // Set CCR = 1
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    ECanaShadow.CANES.all = ECanaRegs.CANES.all;

    do
	{
	    ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 1 );  		// Wait for CCE bit to be set..

    ECanaShadow.CANBTC.all = 0;

    #if (CPU_FRQ_150MHZ)                       // CPU_FRQ_150MHz is defined in DSP2833x_Examples.h
		/* The following block for all 150 MHz SYSCLKOUT (75 MHz CAN clock) - default. Bit rate = 1 Mbps
		   See Note at End of File */
			ECanaShadow.CANBTC.bit.BRPREG = 19; //4: 1 Mbps, 19: 250 kbps, 99: 50 kbps
			ECanaShadow.CANBTC.bit.TSEG2REG = 2;
			ECanaShadow.CANBTC.bit.TSEG1REG = 10;
    #endif


    ECanaShadow.CANBTC.bit.SAM = 1;
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	ECanaShadow.CANMC.bit.CCR = 0 ;            // Set CCR = 0
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    ECanaShadow.CANES.all = ECanaRegs.CANES.all;

    do
    {
       ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 ); 		// Wait for CCE bit to be  cleared..

/* Disable all Mailboxes  */
 	ECanaRegs.CANME.all = 0;		// Required before writing the MSGIDs

    EDIS;
}
