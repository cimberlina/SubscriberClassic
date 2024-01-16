/*
 * log_event.c
 *
 *  Created on: Apr 24, 2012
 *      Author: ironman
 */
#include "includes.h"

//typedef struct 	{
//	int				inuse;
//	//de la conexion por socket
//	SOCKET 			monisock;
//	int				HeartBeatTime;
//	AlarmProtocols	protocol;
//	char			ServerName[128];
//	struct sockaddr_in addr;
//	uint16_t		HBaccount;
//	uint16_t		sockport;
//	int				socktype;
//	int				sockprotocol;
//
//	//de la maquina de estados
//	SM_STATE		state;
//	time_t 			timer;
//	int				retries;
//	uint16_t		flags;
//
//	//del buffer de eventos a transmitir
//	EventRecord eventRecord[20];
//	int eventRec_writeptr;
//	int eventRec_readptr;
//	int eventRec_count;
//
//}  MonitorConnectionStruct;
//

OS_TCB		LogT_Task_TCB;
CPU_STK		LogT_Task_Stk[LogT_Task_STK_SIZE];

EventRecord currentEvent, LcurrentEvent;
uint16_t eventIndex;

// de la implementacion de buffer circular para flash de eventos
uint16_t evflash_wrptr;
uint16_t evflash_rdptr;
uint16_t audit_evflash_wrptr;
uint16_t audit_evflash_rdptr;


const uint8_t hexachar[16] = {'0', '1', '2', '3', '4','5','6','7','8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//void RawCID_LogEvent( uint8_t *cidbuff)
//{
//	uint16_t checksum;
//	int i, len;
//	uint8_t *currentEventPtr;
//	char strbuffer[256];
//
//	currentEvent.index = eventIndex++;
//	if(eventIndex == 0xFFFF)
//		eventIndex = 0x0000;
//	currentEvent.timestamp = SEC_TIMER;
//
//	currentEvent.account = (cidbuff[0] * 0x1000) + (cidbuff[1] * 0x0100) + (cidbuff[2] * 0x0010) + cidbuff[3];
//	currentEvent.cid_qualifier = cidbuff[6];
//	currentEvent.cid_eventcode =  (cidbuff[7] * 0x0100) + (cidbuff[8] * 0x0010) + cidbuff[9];
//	currentEvent.cid_partition = (cidbuff[10] * 0x0010) + cidbuff[11];
//	currentEvent.cid_zoneuser = (cidbuff[12] * 0x0100) + (cidbuff[13] * 0x0010) + cidbuff[14];
//
//	currentEventPtr = (uint8_t *)(&currentEvent);
//	for(i = 0, checksum = 0; i < 14; i++)	{
//		checksum += *(currentEventPtr + i);
//	}
//	checksum &= 0x00FF;
//
//	currentEvent.checksum = (uint8_t)checksum;
//	currentEvent.ack_tag = 0;
//
//	len = BufPrintCidEvent( strbuffer, &currentEvent, 256 );
//
//	CommSendString(COMM0, strbuffer);
//	WriteEventToFlash(&currentEvent);
//
//
//	for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
//		if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )
//			WriteEventToTxBuffer(i, &currentEvent);
//	}
//}

void  LogT_Task(void  *p_arg)
{
    OS_ERR	os_err;
    int rdptr, error, i, chksum;
    EventRecord event;
    uint32_t dfindex;
    uint8_t buffer[DF_EVELEN];
    CPU_SIZE_T len;

    uint8_t *ptr_dest, *ptr_src;

    (void)p_arg;

    LogT_eventRec_writeptr = 0;
    LogT_eventRec_readptr = 0;
    LogT_eventRec_count = 0;
    len = DF_EVELEN;

    while(DEF_ON) {
        WDT_Feed();
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);

        if(LogT_eventRec_count)   {
            rdptr = LogT_eventRec_readptr;

            event.ack_tag = LogT_eventRecord[rdptr].ack_tag;
            event.index = LogT_eventRecord[rdptr].index;
            event.timestamp = LogT_eventRecord[rdptr].timestamp;
            event.account = LogT_eventRecord[rdptr].account;
            event.cid_qualifier = LogT_eventRecord[rdptr].cid_qualifier;
            event.cid_eventcode = LogT_eventRecord[rdptr].cid_eventcode;
            event.cid_partition = LogT_eventRecord[rdptr].cid_partition;
            event.cid_zoneuser = LogT_eventRecord[rdptr].cid_zoneuser;
            event.checksum =  LogT_eventRecord[rdptr].checksum;

            //-----------------------------------------------------------------------
            buffer[0] = event.ack_tag;
            //buffer[1] = (uint8_t)((event.index >> 8) & 0x00FF);
            //buffer[2] = (uint8_t)((event.index) & 0x00FF);

            buffer[1] = (uint8_t)((eventIndex >> 8) & 0x00FF);
            buffer[2] = (uint8_t)((eventIndex) & 0x00FF);

            buffer[3] = (uint8_t)((event.timestamp >> 24) & 0x000000FF);
            buffer[4] = (uint8_t)((event.timestamp >> 16) & 0x000000FF);
            buffer[5] = (uint8_t)((event.timestamp >> 8) & 0x000000FF);
            buffer[6] = (uint8_t)((event.timestamp ) & 0x000000FF);
            buffer[7] = (uint8_t)((event.account >> 8) & 0x00FF);
            buffer[8] = (uint8_t)((event.account ) & 0x00FF);
            buffer[9] = event.cid_qualifier;
            buffer[10] = (uint8_t)((event.cid_eventcode >> 8) & 0x00FF);
            buffer[11] = (uint8_t)((event.cid_eventcode) & 0x00FF);
            buffer[12] = event.cid_partition;
            buffer[13] = (uint8_t)((event.cid_zoneuser >> 8) & 0x00FF);
            buffer[14] = (uint8_t)((event.cid_zoneuser) & 0x00FF);

            chksum = 0;
            for(i = 1; i < 15; i++)	{
                chksum += buffer[i];
            }
            chksum &= 0x00FF;

            buffer[15] = (uint8_t)(chksum & 0x00FF);

            //-----------------------------------------------------------------------
            if((event.cid_eventcode <= 0x999) && (event.cid_eventcode != 0x000)) {
                dfindex = DF_EVENT0 + (evflash_wrptr * DF_EVELEN);
                error = flash0_write(2, buffer, dfindex, DF_EVELEN);
                if ((error = verify_event_wr(buffer, dfindex)) == FALSE)
                    error = flash0_write(2, buffer, dfindex, DF_EVELEN);

                if ((error = verify_event_wr(buffer, dfindex)) == TRUE) {
                    currentEvent.index = eventIndex++;
                    if (eventIndex == 0xFFFF)
                        eventIndex = 0x0000;

                    evflash_wrptr++;
                    if (evflash_wrptr == DF_MAXEVENTS)
                        evflash_wrptr = 0;
                    Set_evwrptr(evflash_wrptr);

                    LogT_eventRec_readptr++;
                    LogT_eventRec_count--;
                    if (LogT_eventRec_readptr == LogT_BUFFLEN) {
                        LogT_eventRec_readptr = 0;
                    }
                }
            }

            //----------------------------------------------------------------------
            event.index = eventIndex-1;
            event.checksum = buffer[15];
            if((event.cid_eventcode <= 0x999) && (event.cid_eventcode != 0x000))	{
                for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
                    if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )	{
                        switch(Monitoreo[i].protocol)	{
                            case AP_NTSEC4:
                            case AP_NTSEC5:
                            case AP_NTSEC6:
                            case AP_NTSEC7:
                                //if(!((i == 0) && ( SystemFlag11 & MACROMODE_FLAG)))
                                    WriteEventToTxBuffer(i, &event);
                                break;
                            case AP_EYSE1:
                                //heartbeat_EYSE1((uint8_t *)(&currentEvent) );
                                //if(!((i == 0) && ( SystemFlag11 & MACROMODE_FLAG)))
                                    WriteEventToTxBuffer(i, &event);
                                break;
                        }
                    }
                }
            }
            //----------------------------------------------------------------------

            OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
            //----------------------------------------------------------------------
        }
    }
}

uint16_t AccountToDigits( uint16_t data)
{
	uint8_t digit[4];
	uint16_t temp;

	digit[0] = data / 1000;
	temp = data % 1000;

	digit[1] = temp / 100;
	temp = temp % 100;

	digit[2] = temp / 10;
	temp = temp % 10;

	digit[3] = temp;

	temp = (digit[0] * 0x1000) + (digit[1] * 0x100) + (digit[2] * 0x10) + digit[3];
	return temp;
}

uint16_t EventCodeToDigits( uint16_t data)
{
	uint8_t digit[3];
	uint16_t temp;

	digit[0] = data / 100;
	temp = data % 100;

	digit[1] = temp / 10;
	temp = temp % 10;

	digit[2] = temp;

	temp = (digit[0] * 0x100) + (digit[1] * 0x10) + digit[2];
	return temp;
}

uint8_t PartitionToDigits( uint8_t data)
{
	uint8_t digit[2], temp;

	digit[0] = data / 10;
	temp = data % 10;

	digit[1] = temp;

	temp = (digit[0] * 0x10) + digit[1];
	return temp;
}

uint16_t ZoneCodeToDigits( uint16_t data)
{
	uint8_t digit[3];
	uint16_t temp;

	digit[0] = data / 100;
	temp = data % 100;

	digit[1] = temp / 10;
	temp = temp % 10;

	digit[2] = temp;

	temp = (digit[0] * 0x100) + (digit[1] * 0x10) + digit[2];

	if((digit[0] > 9) || (digit[1] > 9) || (digit[2] > 9))	{
		return 0;
	}
	return temp;
}

void logCidEvent(uint16_t account, uint8_t qualifier, uint16_t eventcode, uint8_t partition, uint16_t zoneuser)
{
	uint16_t checksum;
	int i;
	uint8_t *currentEventPtr;
	OS_ERR  err;


	OSSemPend(&LogEventRdyPtr, 0, OS_OPT_PEND_BLOCKING, 0, &err);

//	LcurrentEvent.index = eventIndex++;
//	if(eventIndex == 0xFFFF)
//		eventIndex = 0x0000;

	LcurrentEvent.timestamp = SEC_TIMER;
	if(eventcode == REMOTEASALT_TRIGGER)	
		LcurrentEvent.account = account;
	else
		LcurrentEvent.account = AccountToDigits(account);
	
	LcurrentEvent.cid_qualifier = qualifier;
	LcurrentEvent.cid_eventcode = EventCodeToDigits(eventcode);
	LcurrentEvent.cid_partition = PartitionToDigits(partition);
	if((zoneuser >= 0x800) && (partition == hsbc_lock_partition)) {
        LcurrentEvent.cid_zoneuser = zoneuser;
    } else {
		LcurrentEvent.cid_zoneuser = ZoneCodeToDigits(zoneuser);
	}

	currentEventPtr = (uint8_t *)(&LcurrentEvent);
	for(i = 0, checksum = 0; i < 14; i++)	{
		checksum += *(currentEventPtr + i);
	}
	checksum &= 0x00FF;

	LcurrentEvent.checksum = (uint8_t)checksum;
	LcurrentEvent.ack_tag = 0;

//	if(eventcode < 999)	{
//		for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
//			if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )	{
//				switch(Monitoreo[i].protocol)	{
//				case AP_NTSEC4:
//				case AP_NTSEC5:
//				case AP_NTSEC6:
//				case AP_NTSEC7:
//					WriteEventToTxBuffer(i, &LcurrentEvent);
//					break;
//				case AP_EYSE1:
//					//heartbeat_EYSE1((uint8_t *)(&currentEvent) );
//					WriteEventToTxBuffer(i, &LcurrentEvent);
//					break;
//				}
//			}
//		}
//	}

	switch(eventcode)	{
		case 137:
		case 628:
		case 986:
		case 987:
			WriteEventToAuditFlash(&LcurrentEvent);
			break;
		default:
			break;
	}

	if( (eventcode >= 670) && (eventcode <= 683))	{
		WriteEventToAuditFlash(&LcurrentEvent);
	}

	WriteEventToFlash(&LcurrentEvent);
#ifdef R3K_SERIAL
	WriteEventToR3KBuffer(&LcurrentEvent);
//	switch(currentEvent.cid_eventcode)	{
//	case 0x674:
//	case 0x675:
//		WriteEventToR3KBuffer(&currentEvent);
//		break;
//	default:
//		break;
//	}
#endif

#ifdef DEBUG_CID_SERIAL
	//CMI DEBUG -------------------------------------------------
	buf_len = ProtEncoder_R3KCID(&(LcurrentEvent), sendbuffer);
	for(i = 0; i < buf_len; i++)	{
		ComPutChar(DEBUG_COMM, sendbuffer[i]);
		delay_us(100);
	}
	//-------------------------------------------------
#endif


	OSSemPost(&LogEventRdyPtr, OS_OPT_POST_1, &err);

    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);

}

const uint16_t EveMACRO_TBL[MACROEVENTS_LEN] =  { \
        0x110, 0x120, 0x130, 0x401, 0x407,0x602, 0x121, 0x131, 0x139, 0x382, 0x383, \
        0x380, 0x301, 0x356, 0x137, 0x653, 0x111, 0x381,0x387, 0x606,0
};
const uint16_t Eve400_TBL[10] = { 0x401, 0x402, 0x403, 0x407, 0x408, 0x409, 0x410, 0};

int IsInEveMACROTBL(uint16_t event)
{
    int i;

    for(i = 0; i < MACROEVENTS_LEN; i++) {
        if( EveMACRO_TBL[i] == event)
            return 1;
        else if(EveMACRO_TBL[i] == 0)
            return 0;
    }
    return 0;
}

int IsInEve400TBL(uint16_t event)
{
    int i;

    for(i = 0; i < 10; i++) {
        if( Eve400_TBL[i] == event)
            return 1;
        else if(Eve400_TBL[i] == 0)
            return 0;
    }
    return 0;
}

void WriteEventToTxBuffer(int co_id, EventRecord *event)
{
	int wrptr;
    uint16_t eventcodeorig;

	if(!((event->cid_qualifier ==  1) || (event->cid_qualifier ==  3) || (event->cid_qualifier ==  6)) )	{
		return;
	}
//	if((event->cid_eventcode == 0x000) || (event->cid_eventcode == 0xAAA) || (event->cid_eventcode >= 1000))	{
//		return;
//	}

    //-----------------------------------------------------------------------------------------------------------------
    if((SystemFlag11 & MACROMODE_FLAG) && (co_id == 0))   {

        //aca aplico conversion de codigo de eventos para el Macro
        eventcodeorig = event->cid_eventcode;
        if(event->cid_eventcode == 0x408 ) {
            event->cid_eventcode = 0x401;
            //eventcodeorig = 0x408;
        }
        if(event->cid_eventcode == 0x403 ) {
            event->cid_eventcode = 0x401;
            //eventcodeorig = 0x403;
        }
        if(event->cid_eventcode == 0x407 ) {
            event->cid_eventcode = 0x401;
            //eventcodeorig = 0x403;
        }

        if((event->cid_eventcode == 0x110) && ((event->cid_partition == 0) || (event->cid_partition == 1))) {
            SystemFlag11 |= EV110P0_FLAG;
            Mem_Copy(&EV110P0_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((event->cid_eventcode == 0x120) && ((event->cid_partition == 0) || (event->cid_partition == 1))) {
            SystemFlag11 |= EV120P0_FLAG;
            Mem_Copy(&EV120P0_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((event->cid_eventcode == 0x130) && ((event->cid_partition == 0) || (event->cid_partition == 1))) {
            SystemFlag11 |= EV130P0_FLAG;
            Mem_Copy(&EV130P0_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((event->cid_eventcode == 0x110) && (event->cid_partition == 3)) {
            SystemFlag11 |= EV110P3_FLAG;
            Mem_Copy(&EV110P3_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((event->cid_eventcode == 0x120) && (event->cid_partition == 2)) {
            SystemFlag11 |= EV120P2_FLAG;
            if((SystemFlag12 & E120TECNICO_FLAG) && (SystemFlag11 & MACROMODE_FLAG)) {
                //SystemFlag12 &= ~E120TECNICO_FLAG;
                EV120P2_temp.cid_partition = 1;
            }
            Mem_Copy(&EV120P2_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((event->cid_eventcode == 0x130) && (event->cid_partition == 4)) {
            SystemFlag11 |= EV130P4_FLAG;
            if((SystemFlag12 & E130TECNICO_FLAG) && (SystemFlag11 & MACROMODE_FLAG)) {
                //SystemFlag12 &= ~E130TECNICO_FLAG;
                EV130P4_temp.cid_partition = 1;
            }
            Mem_Copy(&EV130P4_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else

        if((IsInEve400TBL(event->cid_eventcode)) && (event->cid_partition == 5)) {
            SystemFlag11 |= EV401P5_FLAG;
            Mem_Copy(&EV401P5_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((IsInEve400TBL(event->cid_eventcode)) && (event->cid_partition == 6)) {
            SystemFlag11 |= EV401P6_FLAG;
            Mem_Copy(&EV401P6_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((IsInEve400TBL(event->cid_eventcode)) && (event->cid_partition == 7)) {
            SystemFlag11 |= EV401P7_FLAG;
            Mem_Copy(&EV401P7_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((IsInEve400TBL(event->cid_eventcode)) && (event->cid_partition == 8)) {
            SystemFlag11 |= EV401P8_FLAG;
            Mem_Copy(&EV401P8_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        } else
        if((IsInEve400TBL(event->cid_eventcode)) && (event->cid_partition == 9)) {
            SystemFlag11 |= EV401P9_FLAG;
            Mem_Copy(&EV401P9_temp, event, sizeof(EventRecord));
            ReadOutEventMacro(0, event);
            return;
        }

        if((event->cid_eventcode == 0x110) && (event->cid_partition >= 10) && (event->cid_partition < 70)) {
            SystemFlag11 &= ~EV110P0_FLAG;
            SystemFlag11 &= ~EV110P3_FLAG;
            SystemFlag11 |= GEV110P0_FLAG;
            SystemFlag11 |= GEV110P3_FLAG;
        }
        if((event->cid_eventcode == 0x120) && (event->cid_partition >= 10) && (event->cid_partition < 70)) {
            SystemFlag11 &= ~EV120P0_FLAG;
            SystemFlag11 &= ~EV120P2_FLAG;
            SystemFlag11 |= GEV120P0_FLAG;
            SystemFlag11 |= GEV120P2_FLAG;
        }
        if((event->cid_eventcode == 0x130) && (event->cid_partition >= 10) && (event->cid_partition < 70)) {   //today
            SystemFlag11 &= ~EV130P0_FLAG;
            SystemFlag11 &= ~EV130P4_FLAG;
            SystemFlag11 |= GEV130P0_FLAG;
            SystemFlag11 |= GEV130P4_FLAG;
        }

        //aca detecto la llegada por datos de los 400 que redundan en bornera delantera
        if((IsInEve400TBL(event->cid_eventcode)) && (VolumetricRedundance[0] == event->cid_partition))    {
            SystemFlag11 &= ~EV401P5_FLAG;
            SystemFlag11 |= GEV401P5_FLAG;
        }
        if((IsInEve400TBL(event->cid_eventcode)) && (VolumetricRedundance[1] == event->cid_partition))    {
            SystemFlag11 &= ~EV401P6_FLAG;
            SystemFlag11 |= GEV401P6_FLAG;
        }
        if((IsInEve400TBL(event->cid_eventcode)) && (VolumetricRedundance[2] == event->cid_partition))    {
            SystemFlag11 &= ~EV401P7_FLAG;
            SystemFlag11 |= GEV401P7_FLAG;
        }
        if((IsInEve400TBL(event->cid_eventcode)) && (VolumetricRedundance[3] == event->cid_partition))    {
            SystemFlag11 &= ~EV401P8_FLAG;
            SystemFlag11 |= GEV401P8_FLAG;
        }
        if((IsInEve400TBL(event->cid_eventcode)) && (VolumetricRedundance[4] == event->cid_partition))    {
            SystemFlag11 &= ~EV401P9_FLAG;
            SystemFlag11 |= GEV401P9_FLAG;
        }

        if( IsInEveMACROTBL(event->cid_eventcode) ) {
            wrptr = Monitoreo[0].eventRec_writeptr++;
            Monitoreo[0].eventRec_count++;
            Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), event, sizeof(EventRecord));
            if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                Monitoreo[0].eventRec_writeptr = 0;
            }
        }
        event->cid_eventcode = eventcodeorig;
    //-----------------------------------------------------------------------------------------------------------------
    } else {
        wrptr = Monitoreo[co_id].eventRec_writeptr++;
        Monitoreo[co_id].eventRec_count++;
        Mem_Copy(&(Monitoreo[co_id].eventRecord[wrptr]), event, sizeof(EventRecord));
        if (Monitoreo[co_id].eventRec_writeptr == TXEVENTBUFFERLEN) {
            Monitoreo[co_id].eventRec_writeptr = 0;
        }
    }
}

unsigned char fsm_MEV110_state;

void fsm_MEV110(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV110_state)    {
        case FMEV110_IDLE:
            if(SystemFlag11 & EV110P0_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV110_state = FMEV110P0;
            } else
            if(SystemFlag11 & EV110P3_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV110_state = FMEV110P03;
            } else if((SystemFlag11 & GEV110P0_FLAG) || (SystemFlag11 & GEV110P3_FLAG))    {
                fsm_MEV110_state = FMEV110_GUARD;
                timeout = SEC_TIMER + 90;
                SystemFlag11 &= ~GEV110P0_FLAG;
                SystemFlag11 &= ~GEV110P3_FLAG;
            }
            break;
        case FMEV110P0:
            if((!(SystemFlag11 & EV110P0_FLAG)) && (!(SystemFlag11 & EV110P3_FLAG)))  {
                fsm_MEV110_state = FMEV110_IDLE;
                if((SystemFlag11 & GEV110P0_FLAG) || (SystemFlag11 & GEV110P3_FLAG))    {
                    fsm_MEV110_state = FMEV110_GUARD;
                    timeout = SEC_TIMER + 90;
                    SystemFlag11 &= ~GEV110P0_FLAG;
                    SystemFlag11 &= ~GEV110P3_FLAG;
                }
            } else
            if(SystemFlag11 & EV110P3_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV110_state = FMEV110P03;
            } else
            if(SEC_TIMER > timeout) {
                //aca tirar evento de particion 0
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV110P0_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV110_state = FMEV110_IDLE;
                SystemFlag11 &= ~EV110P0_FLAG;
                SystemFlag11 &= ~EV110P3_FLAG;
            }
            break;
        case FMEV110P03:
            if((!(SystemFlag11 & EV110P0_FLAG)) && (!(SystemFlag11 & EV110P3_FLAG)))  {
                fsm_MEV110_state = FMEV110_IDLE;
                if((SystemFlag11 & GEV110P0_FLAG) || (SystemFlag11 & GEV110P3_FLAG))    {
                    fsm_MEV110_state = FMEV110_GUARD;
                    timeout = SEC_TIMER + 90;
                    SystemFlag11 &= ~GEV110P0_FLAG;
                    SystemFlag11 &= ~GEV110P3_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                //aca tirar evento de particion 3
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV110P3_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV110_state = FMEV110_IDLE;
                SystemFlag11 &= ~EV110P0_FLAG;
                SystemFlag11 &= ~EV110P3_FLAG;
            }
            break;
        case FMEV110_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV110_state = FMEV110_IDLE;
                SystemFlag11 &= ~EV110P0_FLAG;
                SystemFlag11 &= ~EV110P3_FLAG;
            }
            break;
        default:
            fsm_MEV110_state = FMEV110_IDLE;
            SystemFlag11 &= ~EV110P0_FLAG;
            SystemFlag11 &= ~EV110P3_FLAG;
            break;
    }
}


unsigned char fsm_MEV120_state;

void fsm_MEV120(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV120_state)    {
        case FMEV120_IDLE:
            if(SystemFlag11 & EV120P0_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV120_state = FMEV120P0;
            } else
            if(SystemFlag11 & EV120P2_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV120_state = FMEV120P02;
            } else if((SystemFlag11 & GEV120P0_FLAG) || (SystemFlag11 & GEV120P2_FLAG))    {
                fsm_MEV120_state = FMEV120_GUARD;
                timeout = SEC_TIMER + 90;
                SystemFlag11 &= ~GEV120P0_FLAG;
                SystemFlag11 &= ~GEV120P2_FLAG;
            }
            break;
        case FMEV120P0:
            if((!(SystemFlag11 & EV120P0_FLAG)) && (!(SystemFlag11 & EV120P2_FLAG)))  {
                fsm_MEV120_state = FMEV120_IDLE;
                if((SystemFlag11 & GEV120P0_FLAG) || (SystemFlag11 & GEV120P2_FLAG))    {
                    fsm_MEV120_state = FMEV120_GUARD;
                    timeout = SEC_TIMER + 90;
                    SystemFlag11 &= ~GEV120P0_FLAG;
                    SystemFlag11 &= ~GEV120P2_FLAG;
                }
            } else
            if(SystemFlag11 & EV120P2_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV120_state = FMEV120P02;
            } else
            if(SEC_TIMER > timeout) {
                //aca tirar evento de particion 0
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV120P0_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV120_state = FMEV120_IDLE;
                SystemFlag11 &= ~EV120P0_FLAG;
                SystemFlag11 &= ~EV120P2_FLAG;
            }
            break;
        case FMEV120P02:
            if((!(SystemFlag11 & EV120P0_FLAG)) && (!(SystemFlag11 & EV120P2_FLAG)))  {
                fsm_MEV120_state = FMEV120_IDLE;
                if((SystemFlag11 & GEV120P0_FLAG) || (SystemFlag11 & GEV120P2_FLAG))    {
                    fsm_MEV120_state = FMEV120_GUARD;
                    timeout = SEC_TIMER + 90;
                    SystemFlag11 &= ~GEV120P0_FLAG;
                    SystemFlag11 &= ~GEV120P2_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                //aca tirar evento de particion 2
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                if((SystemFlag12 & E120TECNICO_FLAG) && (SystemFlag11 & MACROMODE_FLAG)) {
                    SystemFlag12 &= ~E120TECNICO_FLAG;
                    EV120P2_temp.cid_partition = 1;
                }
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV120P2_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV120_state = FMEV120_IDLE;
                SystemFlag11 &= ~EV120P0_FLAG;
                SystemFlag11 &= ~EV120P2_FLAG;
            }
            break;
        case FMEV120_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV120_state = FMEV120_IDLE;
                SystemFlag11 &= ~EV120P0_FLAG;
                SystemFlag11 &= ~EV120P2_FLAG;
            }
            break;
        default:
            fsm_MEV120_state = FMEV120_IDLE;
            SystemFlag11 &= ~EV120P0_FLAG;
            SystemFlag11 &= ~EV120P2_FLAG;
            break;
    }
}

unsigned char fsm_MEV130_state;

void fsm_MEV130(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV130_state)    {
        case FMEV130_IDLE:
            if(SystemFlag11 & EV130P0_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV130_state = FMEV130P0;
            } else
            if(SystemFlag11 & EV130P4_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV130_state = FMEV130P04;
            } else if((SystemFlag11 & GEV130P0_FLAG) || (SystemFlag11 & GEV130P4_FLAG))    {
                fsm_MEV130_state = FMEV130_GUARD;
                timeout = SEC_TIMER + 90;
                SystemFlag11 &= ~GEV130P0_FLAG;
                SystemFlag11 &= ~GEV130P4_FLAG;
            }
            break;
        case FMEV130P0:
            if((!(SystemFlag11 & EV130P0_FLAG)) && (!(SystemFlag11 & EV130P4_FLAG)))  {
                fsm_MEV130_state = FMEV130_IDLE;
                if((SystemFlag11 & GEV130P0_FLAG) || (SystemFlag11 & GEV130P4_FLAG))    {
                    fsm_MEV130_state = FMEV130_GUARD;
                    timeout = SEC_TIMER + 90;
                    SystemFlag11 &= ~GEV130P0_FLAG;
                    SystemFlag11 &= ~GEV130P4_FLAG;
                }
            } else
            if(SystemFlag11 & EV130P4_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV130_state = FMEV130P04;
            } else
            if(SEC_TIMER > timeout) {
                //aca tirar evento de particion 0
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV130P0_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV130_state = FMEV130_IDLE;
                SystemFlag11 &= ~EV130P0_FLAG;
                SystemFlag11 &= ~EV130P4_FLAG;
            }
            break;
        case FMEV130P04:
            if((!(SystemFlag11 & EV130P0_FLAG)) && (!(SystemFlag11 & EV130P4_FLAG)))  {
                fsm_MEV130_state = FMEV130_IDLE;
                if((SystemFlag11 & GEV130P0_FLAG) || (SystemFlag11 & GEV130P4_FLAG))    {
                    fsm_MEV130_state = FMEV130_GUARD;
                    timeout = SEC_TIMER + 90;
                    SystemFlag11 &= ~GEV130P0_FLAG;
                    SystemFlag11 &= ~GEV130P4_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                //aca tirar evento de particion 4
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                if((SystemFlag12 & E130TECNICO_FLAG) && (SystemFlag11 & MACROMODE_FLAG)) {
                    SystemFlag12 &= ~E130TECNICO_FLAG;
                    EV130P4_temp.cid_partition = 1;
                }
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV130P4_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV130_state = FMEV130_IDLE;
                SystemFlag11 &= ~EV130P0_FLAG;
                SystemFlag11 &= ~EV130P4_FLAG;
            }
            break;
        case FMEV130_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV130_state = FMEV130_IDLE;
                SystemFlag11 &= ~EV130P0_FLAG;
                SystemFlag11 &= ~EV130P4_FLAG;
            }
            break;
        default:
            fsm_MEV130_state = FMEV130_IDLE;
            SystemFlag11 &= ~EV130P0_FLAG;
            SystemFlag11 &= ~EV130P4_FLAG;
            break;
    }
}

uint8_t fsm_MEV400P5_state;

void fsm_MEV400P5(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV400P5_state)  {
        case FMEV400P5_IDLE:
            if(SystemFlag11 & EV401P5_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV400P5_state = FMEV400P5_RXD;
            } else if(SystemFlag11 & GEV401P5_FLAG)    {
                fsm_MEV400P5_state = FMEV400P5_GUARD;
                timeout = SEC_TIMER + 30;
                SystemFlag11 &= ~GEV401P5_FLAG;
            }
            break;
        case FMEV400P5_RXD:
            if(!(SystemFlag11 & EV401P5_FLAG)) {
                fsm_MEV400P5_state = FMEV400P5_IDLE;
                if(SystemFlag11 & GEV401P5_FLAG)    {
                    fsm_MEV400P5_state = FMEV400P5_GUARD;
                    timeout = SEC_TIMER + 30;
                    SystemFlag11 &= ~GEV401P5_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV401P5_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV400P5_state = FMEV400P5_IDLE;
                SystemFlag11 &= ~EV401P5_FLAG;
            }
            break;
        case FMEV400P5_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV400P5_state = FMEV400P5_IDLE;
                SystemFlag11 &= ~EV401P5_FLAG;
            }
            break;
        default:
            fsm_MEV400P5_state = FMEV400P5_IDLE;
            SystemFlag11 &= ~EV401P5_FLAG;
            break;
    }
}

uint8_t fsm_MEV400P6_state;

void fsm_MEV400P6(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV400P6_state)  {
        case FMEV400P6_IDLE:
            if(SystemFlag11 & EV401P6_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV400P6_state = FMEV400P6_RXD;
            } else if(SystemFlag11 & GEV401P6_FLAG)    {
                fsm_MEV400P6_state = FMEV400P6_GUARD;
                timeout = SEC_TIMER + 30;
                SystemFlag11 &= ~GEV401P6_FLAG;
            }
            break;
        case FMEV400P6_RXD:
            if(!(SystemFlag11 & EV401P6_FLAG)) {
                fsm_MEV400P6_state = FMEV400P6_IDLE;
                if(SystemFlag11 & GEV401P6_FLAG)    {
                    fsm_MEV400P6_state = FMEV400P6_GUARD;
                    timeout = SEC_TIMER + 30;
                    SystemFlag11 &= ~GEV401P6_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV401P6_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV400P6_state = FMEV400P6_IDLE;
                SystemFlag11 &= ~EV401P6_FLAG;
            }
            break;
        case FMEV400P6_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV400P6_state = FMEV400P6_IDLE;
                SystemFlag11 &= ~EV401P6_FLAG;
            }
            break;
        default:
            fsm_MEV400P6_state = FMEV400P6_IDLE;
            SystemFlag11 &= ~EV401P6_FLAG;
            break;
    }
}

uint8_t fsm_MEV400P7_state;

void fsm_MEV400P7(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV400P7_state)  {
        case FMEV400P7_IDLE:
            if(SystemFlag11 & EV401P7_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV400P7_state = FMEV400P7_RXD;
            } else if(SystemFlag11 & GEV401P7_FLAG)    {
                fsm_MEV400P7_state = FMEV400P7_GUARD;
                timeout = SEC_TIMER + 30;
                SystemFlag11 &= ~GEV401P7_FLAG;
            }
            break;
        case FMEV400P7_RXD:
            if(!(SystemFlag11 & EV401P7_FLAG)) {
                fsm_MEV400P7_state = FMEV400P7_IDLE;
                if(SystemFlag11 & GEV401P7_FLAG)    {
                    fsm_MEV400P7_state = FMEV400P7_GUARD;
                    timeout = SEC_TIMER + 30;
                    SystemFlag11 &= ~GEV401P7_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV401P7_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV400P7_state = FMEV400P7_IDLE;
                SystemFlag11 &= ~EV401P7_FLAG;
            }
            break;
        case FMEV400P7_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV400P7_state = FMEV400P7_IDLE;
                SystemFlag11 &= ~EV401P7_FLAG;
            }
            break;
        default:
            fsm_MEV400P7_state = FMEV400P7_IDLE;
            SystemFlag11 &= ~EV401P7_FLAG;
            break;
    }
}

uint8_t fsm_MEV400P8_state;

void fsm_MEV400P8(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV400P8_state)  {
        case FMEV400P8_IDLE:
            if(SystemFlag11 & EV401P8_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV400P8_state = FMEV400P8_RXD;
            } else if(SystemFlag11 & GEV401P8_FLAG)    {
                fsm_MEV400P8_state = FMEV400P8_GUARD;
                timeout = SEC_TIMER + 30;
                SystemFlag11 &= ~GEV401P8_FLAG;
            }
            break;
        case FMEV400P8_RXD:
            if(!(SystemFlag11 & EV401P8_FLAG)) {
                fsm_MEV400P8_state = FMEV400P8_IDLE;
                if(SystemFlag11 & GEV401P8_FLAG)    {
                    fsm_MEV400P8_state = FMEV400P8_GUARD;
                    timeout = SEC_TIMER + 30;
                    SystemFlag11 &= ~GEV401P8_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV401P8_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV400P8_state = FMEV400P8_IDLE;
                SystemFlag11 &= ~EV401P8_FLAG;
            }
            break;
        case FMEV400P8_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV400P8_state = FMEV400P8_IDLE;
                SystemFlag11 &= ~EV401P8_FLAG;
            }
            break;
        default:
            fsm_MEV400P8_state = FMEV400P8_IDLE;
            SystemFlag11 &= ~EV401P8_FLAG;
            break;
    }
}

uint8_t fsm_MEV400P9_state;

void fsm_MEV400P9(void)
{
    static uint32_t  timeout;
    int wrptr;

    switch(fsm_MEV400P9_state)  {
        case FMEV400P9_IDLE:
            if(SystemFlag11 & EV401P9_FLAG) {
                timeout = SEC_TIMER + 30;
                fsm_MEV400P9_state = FMEV400P9_RXD;
            } else if(SystemFlag11 & GEV401P9_FLAG)    {
                fsm_MEV400P9_state = FMEV400P9_GUARD;
                timeout = SEC_TIMER + 30;
                SystemFlag11 &= ~GEV401P9_FLAG;
            }
            break;
        case FMEV400P9_RXD:
            if(!(SystemFlag11 & EV401P9_FLAG)) {
                fsm_MEV400P9_state = FMEV400P9_IDLE;
                if(SystemFlag11 & GEV401P9_FLAG)    {
                    fsm_MEV400P9_state = FMEV400P9_GUARD;
                    timeout = SEC_TIMER + 30;
                    SystemFlag11 &= ~GEV401P9_FLAG;
                }
            } else
            if(SEC_TIMER > timeout) {
                wrptr = Monitoreo[0].eventRec_writeptr++;
                Monitoreo[0].eventRec_count++;
                Mem_Copy(&(Monitoreo[0].eventRecord[wrptr]), &EV401P9_temp, sizeof(EventRecord));
                if (Monitoreo[0].eventRec_writeptr == TXEVENTBUFFERLEN) {
                    Monitoreo[0].eventRec_writeptr = 0;
                }
                fsm_MEV400P9_state = FMEV400P9_IDLE;
                SystemFlag11 &= ~EV401P9_FLAG;
            }
            break;
        case FMEV400P9_GUARD:
            if(SEC_TIMER > timeout) {
                fsm_MEV400P9_state = FMEV400P9_IDLE;
                SystemFlag11 &= ~EV401P9_FLAG;
            }
            break;
        default:
            fsm_MEV400P9_state = FMEV400P9_IDLE;
            SystemFlag11 &= ~EV401P9_FLAG;
            break;
    }
}


void WriteEventToR3KBuffer(EventRecord *event)
{
	int wrptr;

	wrptr = R3KeventRec_writeptr++;
	R3KeventRec_count++;
	Mem_Copy( &(R3KeventRecord[wrptr]), event, sizeof(EventRecord));
	if(R3KeventRec_writeptr == R3KSERIALBUFFLEN)	{
		R3KeventRec_writeptr = 0;
	}

}

void preReadEvent(int co_id, EventRecord *event)
{
	int rdptr;

	SystemFlag2 |= MONBUFFER_BUSY;
	rdptr = Monitoreo[co_id].eventRec_readptr;

	Mem_Copy( event, &(Monitoreo[co_id].eventRecord[rdptr]), sizeof(EventRecord));
	SystemFlag2 &= ~MONBUFFER_BUSY;
}

void R3KpreReadEvent(EventRecord *event)
{
	int rdptr;

	rdptr = R3KeventRec_readptr;

	Mem_Copy( event, &(R3KeventRecord[rdptr]), sizeof(EventRecord));
}

void ReadOutEventMacro( int co_id, EventRecord *event)
{
    uint32_t df_eve_addr;
    uint16_t fnd_index;
    uint8_t buffer[DF_EVELEN];
    OS_ERR os_err;
    int result;

    fnd_index =event->index;
    result = find_event_by_index( fnd_index, &df_eve_addr);

    if(result == TRUE)	{

        flash0_read(buffer, df_eve_addr, DF_EVELEN);
        OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
        switch(co_id)	{
            case 0:
                buffer[0] |= ACKRCVDMON0;
                break;
            case 1:
                buffer[0] |= ACKRCVDMON1;
                break;
        }

        RawWriteEventToFlash(buffer, df_eve_addr);

    }
}

void ReadOutEvent( int co_id, EventRecord *event)
{
	int rdptr, result;
	uint32_t df_eve_addr;
	uint16_t fnd_index;
	uint8_t buffer[DF_EVELEN];
	OS_ERR os_err;

	if( Monitoreo[co_id].eventRec_count == 0 )
		return;
	OSSemPend(&LogEventRdyPtr, 0, OS_OPT_PEND_BLOCKING, 0, &os_err);
	SystemFlag2 |= MONBUFFER_BUSY;
	rdptr = Monitoreo[co_id].eventRec_readptr++;
	Monitoreo[co_id].eventRec_count--;
	Mem_Copy( event, &(Monitoreo[co_id].eventRecord[rdptr]), sizeof(EventRecord));
	if(Monitoreo[co_id].eventRec_readptr == TXEVENTBUFFERLEN)	{
		Monitoreo[co_id].eventRec_readptr = 0;
	}
	//SystemFlag2 &= ~MONBUFFER_BUSY;

	fnd_index =event->index;
	result = find_event_by_index( fnd_index, &df_eve_addr);

	if(result == TRUE)	{

		flash0_read(buffer, df_eve_addr, DF_EVELEN);
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
		switch(co_id)	{
		case 0:
			buffer[0] |= ACKRCVDMON0;
			break;
		case 1:
			buffer[0] |= ACKRCVDMON1;
			break;
		}

		RawWriteEventToFlash(buffer, df_eve_addr);

	}
	SystemFlag2 &= ~MONBUFFER_BUSY;
	OSSemPost(&LogEventRdyPtr, OS_OPT_POST_1, &os_err);
}

void R3KReadOutEvent( EventRecord *event)
{
	int rdptr;

	if( R3KeventRec_count == 0 )
		return;

	rdptr = R3KeventRec_readptr++;
	R3KeventRec_count--;
	Mem_Copy( event, &(R3KeventRecord[rdptr]), sizeof(EventRecord));
	if(R3KeventRec_readptr == R3KSERIALBUFFLEN)	{
		R3KeventRec_readptr = 0;
	}
}

void FormatEventFlash( void )
{
	uint16_t event;
	uint32_t dfindex;
	uint8_t buffer[DF_EVELEN];
	OS_ERR os_err;

	OSSemPend(&LogEventRdyPtr, 0, OS_OPT_PEND_BLOCKING, 0, &os_err);
	for( event = 0; event < DF_EVELEN; event++)	{
		buffer[event] = 0xFF;
	}
	for( event = 0; event < DF_MAXEVENTS; event++)	{
		dfindex = DF_EVENT0 + (event * DF_EVELEN);
		flash0_write(2, buffer, dfindex, DF_EVELEN);
		WDT_Feed();
	}
	evflash_wrptr = 0;
	eventIndex = 0;
	OSSemPost(&LogEventRdyPtr, OS_OPT_POST_1, &os_err);
	Set_evwrptr(evflash_wrptr);
}

int HowManyEvents( void )
{
	int count;
	uint16_t event;
	uint32_t dfindex;
	uint8_t buffer[DF_EVELEN];

	count = 0;

	for( event = 0; event < DF_MAXEVENTS; event++)	{
		dfindex = DF_EVENT0 + (event * DF_EVELEN);
		flash0_read(buffer, dfindex, DF_EVELEN);
		if( (buffer[0] >= 0x00) && (buffer[0] <= 0x03))
			count++;
        else return count;
	}

	return count;
}


void Set_evwrptr( uint16_t index )
{
	uint8_t mybuffer[4];
	OS_ERR os_err;

	//OSSemPend(&LogEventRdyPtr, 0, OS_OPT_PEND_BLOCKING, 0, &os_err);
	mybuffer[0] = 0x55;
	mybuffer[1] = 0xAA;
	mybuffer[2] = (uint8_t)((index >> 8) & 0x00FF);
	mybuffer[3] = (uint8_t)(index & 0x00FF);
	flash0_write(1, mybuffer, DF_EVENTWRPTR_OFFSET, 4);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	mybuffer[0] = 0x55;
	mybuffer[1] = 0xAA;
	mybuffer[2] = (uint8_t)((eventIndex >> 8) & 0x00FF);
	mybuffer[3] = (uint8_t)(eventIndex & 0x00FF);
	flash0_write(1, mybuffer, DF_EVENTINDEX_OFFSET, 4);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	//OSSemPost(&LogEventRdyPtr, OS_OPT_POST_1, &os_err);
}

void Get_evwrptr(void)
{
	uint8_t mybuffer[4];



	OS_ERR os_err;

	//-----------------------------------------------------------------------------
	//Analisis del puntero a ultimo evento y su recuperacion
	flash0_read(mybuffer, DF_EVENTWRPTR_OFFSET, 4);
	OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
	if((mybuffer[0] == 0x55) && (mybuffer[1] == 0xAA))	{
		evflash_wrptr = (mybuffer[2] * 0x100) + mybuffer[3];
	} else
		evflash_wrptr = 0;
//
//	last_evflash_wrptr = evflash_wrptr - 1;
//	count = 0;
//	while(count < DF_MAXEVENTS)	{
//		count++;
//		if( (retval = ReadEventFromFlash( last_evflash_wrptr, &event )) == FALSE)	{
//			if(last_evflash_wrptr == 0)
//				last_evflash_wrptr = DF_MAXEVENTS - 1;
//			else
//				last_evflash_wrptr--;
//		} else	{
//			evflash_wrptr = last_evflash_wrptr + 1;
//			mybuffer[0] = 0x55;
//			mybuffer[1] = 0xAA;
//			mybuffer[2] = (uint8_t)((evflash_wrptr >> 8) & 0x00FF);
//			mybuffer[3] = (uint8_t)(evflash_wrptr & 0x00FF);
//			flash0_write(1, mybuffer, DF_EVENTWRPTR_OFFSET, 4);
//			OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//			break;
//		}
//	}
	//-----------------------------------------------------------------------------

	flash0_read(mybuffer, DF_EVENTINDEX_OFFSET, 4);
	OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
	if((mybuffer[0] == 0x55) && (mybuffer[1] == 0xAA))	{
		eventIndex = (mybuffer[2] * 0x100) + mybuffer[3];
	} else
		eventIndex = 0;
}


int WriteEventToFlash(EventRecord *event)
{
    int wrptr;
    uint8_t buffer[16];
    int error, i, chksum;

    buffer[0] = event->ack_tag;
	buffer[1] = (uint8_t)((event->index >> 8) & 0x00FF);
	buffer[2] = (uint8_t)((event->index) & 0x00FF);
	buffer[3] = (uint8_t)((event->timestamp >> 24) & 0x000000FF);
	buffer[4] = (uint8_t)((event->timestamp >> 16) & 0x000000FF);
	buffer[5] = (uint8_t)((event->timestamp >> 8) & 0x000000FF);
	buffer[6] = (uint8_t)((event->timestamp ) & 0x000000FF);
	buffer[7] = (uint8_t)((event->account >> 8) & 0x00FF);
	buffer[8] = (uint8_t)((event->account ) & 0x00FF);
	buffer[9] = event->cid_qualifier;
	buffer[10] = (uint8_t)((event->cid_eventcode >> 8) & 0x00FF);
	buffer[11] = (uint8_t)((event->cid_eventcode) & 0x00FF);
	buffer[12] = event->cid_partition;
	buffer[13] = (uint8_t)((event->cid_zoneuser >> 8) & 0x00FF);
	buffer[14] = (uint8_t)((event->cid_zoneuser) & 0x00FF);

	chksum = 0;
	for(i = 0; i < 15; i++)	{
		chksum += buffer[i];
	}
	buffer[15] = (uint8_t)(chksum & 0x00FF);

    wrptr = LogT_eventRec_writeptr++;
    LogT_eventRec_count++;

    LogT_eventRecord[wrptr].ack_tag = event->ack_tag;
    LogT_eventRecord[wrptr].index = event->index;
    LogT_eventRecord[wrptr].timestamp = event->timestamp;
    LogT_eventRecord[wrptr].account = event->account;
    LogT_eventRecord[wrptr].cid_qualifier = event->cid_qualifier;
    LogT_eventRecord[wrptr].cid_eventcode = event->cid_eventcode;
    LogT_eventRecord[wrptr].cid_partition = event->cid_partition;
    LogT_eventRecord[wrptr].cid_zoneuser = event->cid_zoneuser;
    LogT_eventRecord[wrptr].checksum = (uint8_t)(chksum & 0x00FF);


    //Mem_Copy( &(LogT_eventRecord[wrptr]), buffer, sizeof(EventRecord));
    if(LogT_eventRec_writeptr == LogT_BUFFLEN)	{
        LogT_eventRec_writeptr = 0;
    }

//	int error, i, chksum;
//	uint8_t buffer[16];
//	uint32_t dfindex;
//	OS_ERR os_err;
//
//	buffer[0] = event->ack_tag;
//	buffer[1] = (uint8_t)((event->index >> 8) & 0x00FF);
//	buffer[2] = (uint8_t)((event->index) & 0x00FF);
//	buffer[3] = (uint8_t)((event->timestamp >> 24) & 0x000000FF);
//	buffer[4] = (uint8_t)((event->timestamp >> 16) & 0x000000FF);
//	buffer[5] = (uint8_t)((event->timestamp >> 8) & 0x000000FF);
//	buffer[6] = (uint8_t)((event->timestamp ) & 0x000000FF);
//	buffer[7] = (uint8_t)((event->account >> 8) & 0x00FF);
//	buffer[8] = (uint8_t)((event->account ) & 0x00FF);
//	buffer[9] = event->cid_qualifier;
//	buffer[10] = (uint8_t)((event->cid_eventcode >> 8) & 0x00FF);
//	buffer[11] = (uint8_t)((event->cid_eventcode) & 0x00FF);
//	buffer[12] = event->cid_partition;
//	buffer[13] = (uint8_t)((event->cid_zoneuser >> 8) & 0x00FF);
//	buffer[14] = (uint8_t)((event->cid_zoneuser) & 0x00FF);
//
//	chksum = 0;
//	for(i = 0; i < 15; i++)	{
//		chksum += buffer[i];
//	}
//	buffer[15] = (uint8_t)(chksum & 0x00FF);
//
//
//	dfindex = DF_EVENT0 + (evflash_wrptr * DF_EVELEN);
//	error = flash0_write(2, buffer, dfindex, DF_EVELEN);
//	//OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
//
//	if( (error=verify_event_wr( buffer, dfindex)) == FALSE )	{
//		error = flash0_write(2, buffer, dfindex, DF_EVELEN);
//		//OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
//	} else
//	if( (error=verify_event_wr( buffer, dfindex)) == FALSE )	{
//		error = flash0_write(2, buffer, dfindex, DF_EVELEN);
//		//OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
//	}
//
//	if( (error=verify_event_wr( buffer, dfindex)) == TRUE )	{
//		evflash_wrptr++;
//		if( evflash_wrptr == DF_MAXEVENTS)
//			evflash_wrptr = 0;
//		Set_evwrptr(evflash_wrptr);
//		return TRUE;
//	} else return FALSE;

}

//--------------------------------------------------------------------------------------
//	Gestion de memoria de auditoria

void Set_audwrptr( uint16_t index )
{
	uint8_t mybuffer[4];
	OS_ERR os_err;

	mybuffer[0] = 0x55;
	mybuffer[1] = 0xAA;
	mybuffer[2] = (uint8_t)((index >> 8) & 0x00FF);
	mybuffer[3] = (uint8_t)(index & 0x00FF);
	flash0_write(1, mybuffer, DF_AUDWRPTR_OFFSET, 4);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
}

void Get_audwrptr(void)
{
	uint8_t mybuffer[4];
	OS_ERR os_err;

	//-----------------------------------------------------------------------------
	//Analisis del puntero a ultimo evento y su recuperacion
	flash0_read(mybuffer, DF_AUDWRPTR_OFFSET, 4);
	OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
	if((mybuffer[0] == 0x55) && (mybuffer[1] == 0xAA))	{
		audit_evflash_wrptr = (mybuffer[2] * 0x100) + mybuffer[3];
	} else
		audit_evflash_wrptr = 0;
}

int WriteEventToAuditFlash(EventRecord *event)
{
	int error, i, chksum;
	uint8_t buffer[16];
	uint32_t dfindex;
	OS_ERR os_err;

    return FALSE;

	buffer[0] = event->ack_tag;
	buffer[1] = (uint8_t)((event->index >> 8) & 0x00FF);
	buffer[2] = (uint8_t)((event->index) & 0x00FF);
	buffer[3] = (uint8_t)((event->timestamp >> 24) & 0x000000FF);
	buffer[4] = (uint8_t)((event->timestamp >> 16) & 0x000000FF);
	buffer[5] = (uint8_t)((event->timestamp >> 8) & 0x000000FF);
	buffer[6] = (uint8_t)((event->timestamp ) & 0x000000FF);
	buffer[7] = (uint8_t)((event->account >> 8) & 0x00FF);
	buffer[8] = (uint8_t)((event->account ) & 0x00FF);
	buffer[9] = event->cid_qualifier;
	buffer[10] = (uint8_t)((event->cid_eventcode >> 8) & 0x00FF);
	buffer[11] = (uint8_t)((event->cid_eventcode) & 0x00FF);
	buffer[12] = event->cid_partition;
	buffer[13] = (uint8_t)((event->cid_zoneuser >> 8) & 0x00FF);
	buffer[14] = (uint8_t)((event->cid_zoneuser) & 0x00FF);

	chksum = 0;
	for(i = 0; i < 15; i++)	{
		chksum += buffer[i];
	}
	buffer[15] = (uint8_t)(chksum & 0x00FF);


	dfindex = DF_EVENT_TEC + (audit_evflash_wrptr * DF_EVELEN);
	error = flash0_write(2, buffer, dfindex, DF_EVELEN);
	//OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);

	if( (error=verify_event_wr( buffer, dfindex)) == FALSE )	{
		error = flash0_write(2, buffer, dfindex, DF_EVELEN);
		//OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
	} else
	if( (error=verify_event_wr( buffer, dfindex)) == FALSE )	{
		error = flash0_write(2, buffer, dfindex, DF_EVELEN);
		//OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
	}

	if( (error=verify_event_wr( buffer, dfindex)) == TRUE )	{
		audit_evflash_wrptr++;
		if( audit_evflash_wrptr == DF_MAXTEVES)
			audit_evflash_wrptr = 0;
		Set_audwrptr(audit_evflash_wrptr);
		return TRUE;
	} else return FALSE;

}

//--------------------------------------------------------------------------------------

int RawWriteEventToFlash(uint8_t buffer[DF_EVELEN], uint32_t dfindex)
{
	int error, i, chksum;

	OS_ERR os_err;



	chksum = 0;
	for(i = 0; i < 15; i++)	{
		chksum += buffer[i];
	}
	buffer[15] = (uint8_t)(chksum & 0x00FF);

	error = flash0_write(2, buffer, dfindex, DF_EVELEN);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	if( (error=verify_event_wr( buffer, dfindex)) == FALSE )	{
		error = flash0_write(2, buffer, dfindex, DF_EVELEN);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	} else
	if( (error=verify_event_wr( buffer, dfindex)) == FALSE )	{
		error = flash0_write(2, buffer, dfindex, DF_EVELEN);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	}

	if( (error=verify_event_wr( buffer, dfindex)) == TRUE )	{
		return TRUE;
	} else return FALSE;

}

int verify_event_wr(uint8_t buffer[DF_EVELEN], uint32_t dfindex)
{
	uint8_t buffer_read[DF_EVELEN], i;
	OS_ERR os_err;

    flash0_read(buffer_read, 0, DF_EVELEN);
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	flash0_read(buffer_read, dfindex, DF_EVELEN);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	for( i = 0; i < DF_EVELEN; i++ )	{
		if(buffer[i] != buffer_read[i])	{
			return FALSE;
		}
	}

	return TRUE;
}

//CID_String_struct CID_String_table[] =
//{
//    { 100, "Medical" },
//    { 101, "Personal Emergency" },
//    { 102, "Fail to report in" },
//    { 110, "Fire" },
//    { 111, "Smoke" },
//    { 112, "Combustion" },
//    { 113, "Water flow" },
//    { 114, "Heat" },
//    { 115, "Pull Station" },
//    { 116, "Duct" },
//    { 117, "Flame" },
//    { 118, "Near Alarm" },
//    { 120, "Panic" },
//    { 121, "Duress" },
//    { 122, "Silent" },
//    { 123, "Audible" },
//    { 124, "Duress � Access granted" },
//    { 125, "Duress � Egress granted" },
//    { 130, "Burglary" },
//    { 131, "Perimeter" },
//    { 132, "Interior" },
//    { 133, "24 Hour (Safe)" },
//    { 134, "Entry/Exit" },
//    { 135, "Day/night" },
//    { 136, "Outdoor" },
//    { 137, "Tamper" },
//    { 138, "Near alarm" },
//    { 139, "Intrusion Verifier" },
//    { 140, "General Alarm" },
//    { 141, "Polling loop open" },
//    { 142, "Polling loop short" },
//    { 143, "Expansion module failure" },
//    { 144, "Sensor tamper" },
//    { 145, "Expansion module tamper" },
//    { 146, "Silent Burglary" },
//    { 147, "Sensor Supervision Failure" },
//    { 150, "24 Hour Non-Burglary" },
//    { 151, "Gas detected" },
//    { 152, "Refrigeration" },
//    { 153, "Loss of heat" },
//    { 154, "Water Leakage" },
//    { 155, "Foil Break" },
//    { 156, "Day Trouble" },
//    { 157, "Low bottled gas level" },
//    { 158, "High temp" },
//    { 159, "Low temp" },
//    { 161, "Loss of air flow" },
//    { 162, "Carbon Monoxide detected" },
//    { 163, "Tank level" },
//    { 200, "Fire Supervisory" },
//    { 201, "Low water pressure" },
//    { 202, "Low CO2" },
//    { 203, "Gate valve sensor" },
//    { 204, "Low water level" },
//    { 205, "Pump activated" },
//    { 206, "Pump failure" },
//    { 300, "System Trouble" },
//    { 301, "AC Loss" },
//    { 302, "Low system battery" },
//    { 303, "RAM Checksum bad" },
//    { 304, "ROM checksum bad" },
//    { 305, "System reset" },
//    { 306, "Panel programming changed" },
//    { 307, "Self- test failure" },
//    { 308, "System shutdown" },
//    { 309, "Battery test failure" },
//    { 310, "Ground fault" },
//    { 311, "Battery Missing/Dead" },
//    { 312, "Power Supply Overcurrent" },
//    { 313, "Engineer Reset" },
//    { 320, "Sounder/Relay" },
//    { 321, "Bell 1" },
//    { 322, "Bell 2" },
//    { 323, "Alarm relay" },
//    { 324, "Trouble relay" },
//    { 325, "Reversing relay" },
//    { 326, "Notification Appliance Ckt. # 3" },
//    { 327, "Notification Appliance Ckt. #4" },
//    { 330, "System Peripheral trouble" },
//    { 331, "Polling loop open" },
//    { 332, "Polling loop short" },
//    { 333, "Expansion module failure" },
//    { 334, "Repeater failure" },
//    { 335, "Local printer out of paper" },
//    { 336, "Local printer failure" },
//    { 337, "Exp. Module DC Loss" },
//    { 338, "Exp. Module Low Batt." },
//    { 339, "Exp. Module Reset" },
//    { 341, "Exp. Module Tamper" },
//    { 342, "Exp. Module AC Loss" },
//    { 343, "Exp. Module self-test fail" },
//    { 344, "RF Receiver Jam Detect" },
//    { 350, "Communication trouble" },
//    { 351, "Telco 1 fault" },
//    { 352, "Telco 2 fault" },
//    { 353, "Long Range Radio xmitter fault" },
//    { 354, "Failure to communicate event" },
//    { 355, "Loss of Radio supervision" },
//    { 356, "Loss of central polling" },
//    { 357, "Long Range Radio VSWR problem" },
//    { 370, "Protection loop" },
//    { 371, "Protection loop open" },
//    { 372, "Protection loop short" },
//    { 373, "Fire trouble" },
//    { 374, "Exit error alarm (zone)" },
//    { 375, "Panic zone trouble" },
//    { 376, "Hold-up zone trouble" },
//    { 377, "Swinger Trouble" },
//    { 378, "Cross-zone Trouble" },
//    { 380, "Sensor trouble" },
//    { 381, "Loss of supervision - RF" },
//    { 382, "Loss of supervision - RPM" },
//    { 383, "Sensor tamper" },
//    { 384, "RF low battery" },
//    { 385, "Smoke detector Hi sensitivity" },
//    { 386, "Smoke detector Low sensitivity" },
//    { 387, "Intrusion detector Hi sensitivity" },
//    { 388, "Intrusion detector Low sensitivity" },
//    { 389, "Sensor self-test failure" },
//    { 391, "Sensor Watch trouble" },
//    { 392, "Drift Compensation Error" },
//    { 393, "Maintenance Alert" },
//    { 400, "Open/Close" },
//    { 401, "O/C by user" },
//    { 402, "Group O/C" },
//    { 403, "Automatic O/C" },
//    { 404, "Late to O/C (Note: use 453, 454 instead )" },
//    { 405, "Deferred O/C (Obsolete- do not use )" },
//    { 406, "Cancel" },
//    { 407, "Remote arm/disarm" },
//    { 408, "Quick arm" },
//    { 409, "Keyswitch O/C" },
//    { 441, "Armed STAY" },
//    { 442, "Keyswitch Armed STAY" },
//    { 450, "Exception O/C" },
//    { 451, "Early O/C" },
//    { 452, "Late O/C" },
//    { 453, "Failed to Open" },
//    { 454, "Failed to Close" },
//    { 455, "Auto-arm Failed" },
//    { 456, "Partial Arm" },
//    { 457, "Exit Error (user)" },
//    { 458, "User on Premises" },
//    { 459, "Recent Close" },
//    { 461, "Wrong Code Entry" },
//    { 462, "Legal Code Entry" },
//    { 463, "Re-arm after Alarm" },
//    { 464, "Auto-arm Time Extended" },
//    { 465, "Panic Alarm Reset" },
//    { 466, "Service On/Off Premises" },
//    { 411, "Callback request made" },
//    { 412, "Successful download/access" },
//    { 413, "Unsuccessful access" },
//    { 414, "System shutdown command received" },
//    { 415, "Dialer shutdown command received" },
//    { 416, "Successful Upload" },
//    { 421, "Access denied" },
//    { 422, "Access report by user" },
//    { 423, "Forced Access" },
//    { 424, "Egress Denied" },
//    { 425, "Egress Granted" },
//    { 426, "Access Door propped open" },
//    { 427, "Access point Door Status Monitor trouble" },
//    { 428, "Access point Request To Exit trouble" },
//    { 429, "Access program mode entry" },
//    { 430, "Access program mode exit" },
//    { 431, "Access threat level change" },
//    { 432, "Access relay/trigger fail" },
//    { 433, "Access RTE shunt" },
//    { 434, "Access DSM shunt" },
//    { 501, "Access reader disable" },
//    { 520, "Sounder/Relay Disable" },
//    { 521, "Bell 1 disable" },
//    { 522, "Bell 2 disable" },
//    { 523, "Alarm relay disable" },
//    { 524, "Trouble relay disable" },
//    { 525, "Reversing relay disable" },
//    { 526, "Notification Appliance Ckt. # 3 disable" },
//    { 527, "Notification Appliance Ckt. # 4 disable" },
//    { 531, "Module Added" },
//    { 532, "Module Removed" },
//    { 551, "Dialer disabled" },
//    { 552, "Radio transmitter disabled" },
//    { 553, "Remote Upload/Download disabled" },
//    { 570, "Zone/Sensor bypass" },
//    { 571, "Fire bypass" },
//    { 572, "24 Hour zone bypass" },
//    { 573, "Burg. Bypass" },
//    { 574, "Group bypass" },
//    { 575, "Swinger bypass" },
//    { 576, "Access zone shunt" },
//    { 577, "Access point bypass" },
//    { 601, "Manual trigger test report" },
//    { 602, "Periodic test report" },
//    { 603, "Periodic RF transmission" },
//    { 604, "Fire test" },
//    { 605, "Status report to follow" },
//    { 606, "Listen- in to follow" },
//    { 607, "Walk test mode" },
//    { 608, "Periodic test - System Trouble Present" },
//    { 609, "Video Xmitter active" },
//    { 611, "Point tested OK" },
//    { 612, "Point not tested" },
//    { 613, "Intrusion Zone Walk Tested" },
//    { 614, "Fire Zone Walk Tested" },
//    { 615, "Panic Zone Walk Tested" },
//    { 616, "Service Request" },
//    { 621, "Event Log reset" },
//    { 622, "Event Log 50% full" },
//    { 623, "Event Log 90% full" },
//    { 624, "Event Log overflow" },
//    { 625, "Time/Date reset" },
//    { 626, "Time/Date inaccurate" },
//    { 627, "Program mode entry" },
//    { 628, "Program mode exit" },
//    { 629, "32 Hour Event log marker" },
//    { 630, "Schedule change" },
//    { 631, "Exception schedule change" },
//    { 632, "Access schedule change" },
//    { 641, "Senior Watch Trouble" },
//    { 642, "Latch-key Supervision" },
//    { 651, "Reserved for Ademco Use" },
//    { 652, "Reserved for Ademco Use" },
//    { 653, "Reserved for Ademco Use" },
//    { 654, "System Inactivity" },
//    { 700, "Estado de Red" },
//	{ 0, ""}
//};

int ReadEventFromFlash( uint16_t evindex, EventRecord *event )
{
	uint8_t buffer[DF_EVELEN];
	int i, chksum;
	uint32_t dfindex;

	dfindex = DF_EVENT0 + (evindex * DF_EVELEN);
	flash0_read(buffer, dfindex, DF_EVELEN);

	if(buffer[0] == 0xFF)
		return FALSE;

	chksum = 0;
	for(i = 1; i < 15; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
    buffer[15] -= buffer[0];
	if( buffer[15] != chksum )
		return FALSE;

	event->ack_tag = buffer[0];
	event->index = (buffer[1] * 0x100) + buffer[2];
	event->timestamp = (buffer[3] * 0x01000000) + (buffer[4] * 0x00010000) + (buffer[5] * 0x00000100) + buffer[6];
	event->account = (buffer[7] * 0x0100) + buffer[8];
	event->cid_qualifier = buffer[9];
	event->cid_eventcode = (buffer[10] * 0x0100) + buffer[11];
	event->cid_partition = buffer[12];
	event->cid_zoneuser = (buffer[13] * 0x0100) + buffer[14];

	return TRUE;
}

int ReadEventFromAuditFlash( uint16_t evindex, EventRecord *event )
{
	uint8_t buffer[DF_EVELEN];
	int i, chksum;
	uint32_t dfindex;

	dfindex = DF_EVENT_TEC + (evindex * DF_EVELEN);
	flash0_read(buffer, dfindex, DF_EVELEN);

	if(buffer[0] == 0xFF)
		return FALSE;

	chksum = 0;
	for(i = 0; i < 15; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	if( buffer[15] != chksum )
		return FALSE;

	event->ack_tag = buffer[0];
	event->index = (buffer[1] * 0x100) + buffer[2];
	event->timestamp = (buffer[3] * 0x01000000) + (buffer[4] * 0x00010000) + (buffer[5] * 0x00000100) + buffer[6];
	event->account = (buffer[7] * 0x0100) + buffer[8];
	event->cid_qualifier = buffer[9];
	event->cid_eventcode = (buffer[10] * 0x0100) + buffer[11];
	event->cid_partition = buffer[12];
	event->cid_zoneuser = (buffer[13] * 0x0100) + buffer[14];

	return TRUE;
}


int BufPrintCidEvent( char *strbuffer, EventRecord *event, int lenbuff )
{
	struct tm evtime;
	int i;
	char data[5];

	for(i = 0; i < lenbuff; i++ )	{
		strbuffer[i] = 0x00;
	}
	Str_Cat( strbuffer, "INX: " );
	Str_Cat(strbuffer, itoa(event->index));
	if(event->index < 10)
		Str_Cat( strbuffer, " \t" );
	Str_Cat( strbuffer, " \t" );
	gmtime( &(event->timestamp), &evtime );
	Str_Cat(strbuffer, asctime(&evtime));
	Str_Cat( strbuffer, " - ACCOUNT: " );


	data[0] = hexachar[((event->account >> 12) & 0x0F)];
	data[1] = hexachar[((event->account >> 8) & 0x0F)];
	data[2] = hexachar[((event->account >> 4) & 0x0F)];
	data[3] = hexachar[(event->account & 0x0F)];
	data[4] = 0;
	Str_Cat( strbuffer, data);

	switch(event->cid_qualifier)	{
	case 1:
		Str_Cat( strbuffer, " EVENT: E" );
		break;
	case 3:
		Str_Cat( strbuffer, " EVENT: R" );
		break;
	case 6:
		Str_Cat( strbuffer, " EVENT: P" );
		break;
	default:
		Str_Cat( strbuffer, " EVENT: X" );
		break;
	}

	data[0] = hexachar[((event->cid_eventcode >> 8) & 0x0F)];
	data[1] = hexachar[((event->cid_eventcode >> 4) & 0x0F)];
	data[2] = hexachar[(event->cid_eventcode & 0x0F)];
	data[3] = 0;
	Str_Cat( strbuffer, data);

	Str_Cat( strbuffer, " \tP: " );
	data[0] = hexachar[((event->cid_partition >> 4) & 0x0F)];
	data[1] = hexachar[(event->cid_partition & 0x0F)];
	data[2] = 0;
	Str_Cat( strbuffer, data);

	Str_Cat( strbuffer, " \tZ/U: " );
	data[0] = hexachar[((event->cid_zoneuser >> 8) & 0x0F)];
	data[1] = hexachar[((event->cid_zoneuser >> 4) & 0x0F)];
	data[2] = hexachar[(event->cid_zoneuser & 0x0F)];
	data[3] = 0;
	Str_Cat( strbuffer, data);

//	if(lenbuff > 128)	{
//		for(i = 0; CID_String_table[i].cidevent != 0; i++)	{
//			if( EventCodeToInt(event->cid_eventcode) == CID_String_table[i].cidevent )	{
//				Str_Cat( strbuffer, " \t" );
//				Str_Cat( strbuffer, CID_String_table[i].cidstring );
//
//				break;
//			}
//		}
//	}

	Str_Cat( strbuffer, "\r\n" );

	for(i = 0; strbuffer[i] != '\0'; i++);

	WDT_Feed();
	return i;

}


//int EventCodeToInt(uint16_t eventcode)
//{
//	int retval;
//
//	retval = (((eventcode >> 12) & 0x0F)*1000) + (((eventcode >> 8) & 0x0F)*100) + (((eventcode >> 4) & 0x0F)*10) + (eventcode & 0x0F);
//	return retval;
//}

time_t ConvTimestamp(char *contimestamp)
{
	char temp[12];
	int i, tempint;
	struct tm currtime;
	time_t time1;

	//convierto a formato timestamp de eventos
	for( i = 0; i < 4; i++)
		temp[i] = contimestamp[i];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_year = tempint - YEAR0;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+4];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mon = tempint-1;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+6];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mday = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+8];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_hour = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+10];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_min = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+12];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_sec = tempint;

	time1 = mktime(&currtime);		//timestamp para busqueda

	return time1;
}

int GetTimestampIndexEv_GE( char *contimestamp)
{
	int retval, i, tempint;
	struct tm currtime;
	char temp[12];
	time_t time1, currmax;
	EventRecord event;
    uint16_t ceroindex;

	//convierto a formato timestamp de eventos
	for( i = 0; i < 4; i++)
		temp[i] = contimestamp[i];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_year = tempint - YEAR0;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+4];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mon = tempint-1;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+6];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mday = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+8];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_hour = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+10];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_min = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+12];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_sec = tempint;

	time1 = mktime(&currtime);		//timestamp para busqueda

	currmax = 0xFFFFFFFF;
	retval = -1;
    ReadEventFromFlash( 0, &event );
    ceroindex = event.index;
	for( i = 0; i < DF_MAXEVENTS; i++)	{
		if((tempint = ReadEventFromFlash( i, &event )) == TRUE)	{
            if((event.ack_tag < 0x00) || (event.ack_tag > 0x03))
                return retval;
            if(i == 0)  {
                if((event.timestamp >= time1) && (event.timestamp <= currmax) )	{
                    currmax = event.timestamp;
                    retval = i;
                }
            } else
			if((event.timestamp >= time1) && (event.timestamp <= currmax) && (event.index != ceroindex))	{
				currmax = event.timestamp;
				retval = i;
			}
		}
	}

	return retval;
}

int GetTimestampIndexEv_LE( char *contimestamp)
{
	int retval, i, tempint;
	struct tm currtime;
	char temp[12];
	time_t time2, currmax;
	EventRecord event;

	//convierto a formato timestamp de eventos
	for( i = 0; i < 4; i++)
		temp[i] = contimestamp[i];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_year = tempint - YEAR0;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+4];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mon = tempint-1;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+6];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mday = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+8];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_hour = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+10];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_min = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+12];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_sec = tempint;

	time2 = mktime(&currtime);		//timestamp para busqueda

	currmax = 0x00000000;
	retval = -1;
	for( i = 0; i < DF_MAXEVENTS; i++)	{
		if(ReadEventFromFlash( i, &event ))	{
			if((event.timestamp <= time2) && (event.timestamp >= currmax))	{
				currmax = event.timestamp;
				retval = i;
			}
		}
	}

	return retval;
}

//**********************************************************************************************************
// Funciones de consola para visualizacion de los eventos memorizados en la DataFlash
//

int con_EvDF_format(ConsoleState* state)
{
	auto int temp;

	temp = state->numparams - state->commandparams;
	if ((state->commandparams == 1) &&
	    (Str_Cmp(con_getparam(state->command, temp), "yes") == 0)) {
		state->conio->puts("Wait, please ...\n\r");
		FormatEventFlash();
		state->conio->puts("Done!\n\r");
		return 1;
	} else if ((state->commandparams == 1) &&
	           (Str_Cmp(con_getparam(state->command, temp), "no") == 0)) {
		//state->echo = 0;
		return 1;
	} else {
		state->conio->puts("Usage: ev_df_format yes\n\r");
		state->error = CON_ERR_BADPARAMETER;
		return -1;
	}
}

//int con_DumpEvMemory(ConsoleState* state)
//{
//	int index, index1, index2, i;
//	uint32_t dfindex;
//	uint8_t buffer[DF_EVELEN];
//	char tmpbuffer[16];
//	char tsndbuffer[132];
//
//
//	for( i = 0; i < 132; i++ )	{
//		tsndbuffer[i] = 0;
//	}
//
//	switch(state->numparams)	{
//	case 2:
//		index1 = atoi(con_getparam(state->command, 1));
//		index2 = index1;
//		break;
//	case 3:
//		index1 = atoi(con_getparam(state->command, 1));
//		index2 = atoi(con_getparam(state->command, 2));
//		break;
//	}
//
//
//
//	if( (index1 >= DF_MAXEVENTS) || (index2 >= DF_MAXEVENTS))	{
//		state->conio->puts("Indice espcificado fuera de rango\n\r");
//		return -1;
//	}
//
//	for( index = index1; index <= index2; index++)	{
//		dfindex = DF_EVENT0 + (index * DF_EVELEN);
//		flash0_read(buffer, dfindex, DF_EVELEN);
//		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)itoa(index));
//		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)";\t");
//		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)itoa(dfindex));
//		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)":\t ");
//
//
//		for( i = 0; i < DF_EVELEN; i++)	{
//			BuffPutHex( tmpbuffer, buffer[i]);
//			Str_Cat(tmpbuffer, " ");
//			Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)tmpbuffer);
//
//		}
//		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)"\r\n");
//		state->conio->puts(tsndbuffer);
//
//		tsndbuffer[0] = 0;
//
//	}
//	state->conio->puts("\r\n");
//	return 1;
//}

//int con_DumpEvIndex(ConsoleState* state)
//{
//	int index, len;
//	EventRecord curr_event;
//	char buffer[128];
//
//	index = atoi(con_getparam(state->command, 1));
//
//	if( index >= DF_MAXEVENTS)	{
//		state->conio->puts("Indice especificado fuera de rango\n\r");
//		return -1;
//	}
//
//	len = 0;
//	if( ReadEventFromFlash( index, &curr_event ) )	{
//		len = BufPrintCidEvent( buffer, &curr_event, 128 );
//	}
//	if(len)	{
//		state->conio->puts(buffer);
//		return 1;
//	}
//
//	state->conio->puts("Error de lectura de evento\n\r");
//	return -1;
//}

int con_audit_dump(ConsoleState* state)
{
	int index, len;
	EventRecord curr_event;
	char buffer[128];

	for(index = 0; index < DF_MAXTEVES; index++)	{
		len = 0;
		if( ReadEventFromAuditFlash( index, &curr_event ) )	{
			len = BufPrintCidEvent( buffer, &curr_event, 128 );
		}
		if(len)	{
			state->conio->puts(buffer);
		}		
	}

	return 1;	
}


int eveset_number( char *evestr)
{
    int evesetnum;

    //Str_Cmp(con_getparam(state->command, temp), "yes") == 0)
    if(Str_Cmp(evestr, "APERTURA") == 0)
        evesetnum = EVSET_APERTURA;
    else if(Str_Cmp(evestr, "ALARMAS") == 0)
        evesetnum = EVSET_ALARMAS;
    else if(Str_Cmp(evestr, "F220") == 0)
        evesetnum = EVSET_F220;
    else if(Str_Cmp(evestr, "SUPERVISION") == 0)
        evesetnum = EVSET_SUPERVISION;
    else if(Str_Cmp(evestr, "ROTURA") == 0)
        evesetnum = EVSET_ROTURA;
    else if(Str_Cmp(evestr, "PREVE") == 0)
        evesetnum = EVSET_PREVE;
    else if(Str_Cmp(evestr, "RESET") == 0)
        evesetnum = EVSET_RESET;
    else if(Str_Cmp(evestr, "DESPROTECCION") == 0)
        evesetnum = EVSET_DESPROTECCION;
    else if(Str_Cmp(evestr, "TECNICO") == 0)
        evesetnum = EVSET_TECNICO;
    else if(Str_Cmp(evestr, "ALL") == 0)
        evesetnum = EVSET_ALL;
    else
        evesetnum = -1;

    return evesetnum;
}

const uint16_t EveSet_TBL[EVSET_TYPE_LEN][EVSET_LEN] = {
        {0x137, 0x147, 0x674, 0x675, 0x913, 0x914, 0},
        {0x120, 0x121, 0x122, 0x123, 0x127, 0x128, 0x165, 0x166, 0x167, 0x168, 0x169, 0x888, 0x889, 0x130, 0x131, 0x132,
         0x133, 0x134, 0x135, 0x138, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188, 0x189, 0x190, 0x191, 0x192,
         0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117, 0x118, 0x175, 0x176, 0x139, 0x136, 0x145, 0x393, 0},
        {0x301, 0x302, 0x333, 0x337, 0x342, 0},
        { 0x699, 0x700, 0x602, 0},
        {0x143, 0x144, 0x300, 0x303, 0x304, 0x305, 0x308, 0x309, 0x310, 0x311, 0x312, 0x314, 0x315, 0x321, 0x322, 0x323, 0x324,
         0x325, 0x333, 0x341, 0x350, 0x351,0x354, 0x373,0x380, 0x381, 0x382, 0x383, 0x384, 0x385, 0x386, 0x387, 0x388, 0x735, 0x943, 0},
        { 0x356,0 },
        { 0x900, 0x901, 0x902, 0x903, 0x904, 0},
        {0x400, 0x401, 0x402, 0x403, 0x404, 0x405, 0x407, 0x408, 0x409, 0x441, 0x442, 0x451, 0x452, 0x456, 0x470, 0x801, 0},
        {0x306, 0x353, 0x406, 0x412, 0x421, 0x422, 0x423, 0x429, 0x430, 0x460, 0x528, 0x529, 0x530, 0x552, 0x570, 0x571, 0x572,
         0x572, 0x573, 0x574,  0x621, 0x622, 0x623, 0x624, 0x625, 0x626, 0x627, 0x628, 0x670, 0x671, 0x672, 0x673,
         0x676, 0x677, 0x678, 0x679, 0x680, 0x681, 0x682, 0x777, 0x778, 0x779, 0x780, 0x781, 0x785, 0x812, 0x813, 0x814, 0x815,
         0x879, 0x880, 0x881, 0x882, 0x883, 0x884, 0x885, 0x886, 0x887, 0x891, 0x892, 0x895, 0x897, 0x905, 0x906, 0x909, 0x910, 0x911,
         0x912, 0x915, 0x916, 0x917, 920, 0x921, 0x922, 0x923, 0x927,0x931, 0x933, 0x934, 0x941, 0x942, 0x944, 0x945, 0x946, 0x961,
         0x962, 0x972, 0x973, 0x974, 0x975, 0x978, 0x979, 0x980, 0x981, 0x982, 0x983, 0x984, 0x985, 0x986, 0x987, 0x988, 0x989,
         0x990, 0x991, 0x995, 0x996, 0 }
};

int IsInEveTBL( int evset, uint16_t event)
{
    int i;

    if(evset == EVSET_ALL)
        return 1;

    for(i = 0; i < EVSET_LEN; i++) {
        if( EveSet_TBL[evset][i] == event)
            return 1;
        else if(EveSet_TBL[evset][i] == 0)
            return 0;
    }
    return 0;
}

int con_DumpEventByTime(ConsoleState* state)
{
	uint16_t indexT1, indexT2, i;
	char buffer[128];
	EventRecord thisevent;
	time_t t1, t2;
    int retval, eventset;

    if(state->commandparams >= 1 ) {
        eventset = eveset_number(con_getparam(state->command, 1));
        if(eventset == -1)  {
            state->conio->puts("Error de parametros\n\r");
            return -1;
        }
    } else  {
        state->conio->puts("Error de parametros\n\r");
        return -1;
    }

	switch(state->numparams)	{
	case 2:
		i = HowManyEvents();
		if(i > evflash_wrptr)	{
			indexT1 = evflash_wrptr;
			indexT2 = evflash_wrptr -1;
		} else	{
			indexT1 = 0;
			indexT2 = evflash_wrptr -1;
		}
		break;
	case 3:
		retval = GetTimestampIndexEv_GE(con_getparam(state->command, 2));
        if(retval == -1)   {
            state->conio->puts("\n\r");
            return 1;
        }
        indexT1 = retval;
		indexT2 = evflash_wrptr - 1;
		break;
	case 4:
		t1 = ConvTimestamp(con_getparam(state->command, 2));
		t2 = ConvTimestamp(con_getparam(state->command, 3));
		if( t1 > t2)	{
			state->conio->puts("Invalid parameters\n\r");
			return -1;

		} else if( t2 > t1)	{
			indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 2));
			indexT2 = GetTimestampIndexEv_LE(con_getparam(state->command, 3));
		} else	{
			indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 2));
			indexT2 = evflash_wrptr - 1;
		}
		break;
	default:
		state->conio->puts("Error en cantidad de parametros\n\r");
		return -1;
	}

	if(indexT2 >= indexT1)	{
		for( i = indexT1; i <= indexT2; i++)	{
			if(ReadEventFromFlash( i, &thisevent ))	{
                if(IsInEveTBL(eventset, thisevent.cid_eventcode)) {
                    BufPrintCidEvent(buffer, &thisevent, 128);
                    state->conio->puts(buffer);
                }
			} else	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
	} else	{
		for( i = indexT1; i < DF_MAXEVENTS; i++)	{
			if(ReadEventFromFlash( i, &thisevent ))	{
                if(IsInEveTBL(eventset, thisevent.cid_eventcode)) {
                    BufPrintCidEvent(buffer, &thisevent, 128);
                    state->conio->puts(buffer);
                }
			} else	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		for( i = 0; i <= indexT2; i++)	{
			if(ReadEventFromFlash( i, &thisevent ))	{
                if(IsInEveTBL(eventset, thisevent.cid_eventcode)) {
                    BufPrintCidEvent(buffer, &thisevent, 128);
                    state->conio->puts(buffer);
                }
			} else	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
	}
	state->conio->puts("\n\r");
	return 1;
}


//int con_DumpEventTypeByTime(ConsoleState* state)
//{
//    uint16_t indexT1, indexT2, i;
//    char buffer[128];
//    EventRecord thisevent;
//    time_t t1, t2;
//
//    switch(state->numparams)	{
//        case 1:
//            i = HowManyEvents();
//            if(i > evflash_wrptr)	{
//                indexT1 = evflash_wrptr;
//                indexT2 = evflash_wrptr -1;
//            } else	{
//                indexT1 = 0;
//                indexT2 = evflash_wrptr -1;
//            }
//            break;
//        case 2:
//            indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
//            indexT2 = evflash_wrptr - 1;
//            break;
//        case 3:
//            t1 = ConvTimestamp(con_getparam(state->command, 1));
//            t2 = ConvTimestamp(con_getparam(state->command, 2));
//            if( t1 > t2)	{
//                state->conio->puts("Invalid parameters\n\r");
//                return -1;
//
//            } else if( t2 > t1)	{
//                indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
//                indexT2 = GetTimestampIndexEv_LE(con_getparam(state->command, 2));
//            } else	{
//                indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
//                indexT2 = evflash_wrptr - 1;
//            }
//            break;
//        default:
//            state->conio->puts("Error en cantidad de parametros\n\r");
//            return -1;
//    }
//
//    if(indexT2 >= indexT1)	{
//        for( i = indexT1; i <= indexT2; i++)	{
//            if(ReadEventFromFlash( i, &thisevent ))	{
//                BufPrintCidEvent( buffer, &thisevent, 128 );
//                state->conio->puts(buffer);
//            } else	{
//                state->conio->puts("\n\r");
//                return 1;
//            }
//        }
//    } else	{
//        for( i = indexT1; i < DF_MAXEVENTS; i++)	{
//            if(ReadEventFromFlash( i, &thisevent ))	{
//                BufPrintCidEvent( buffer, &thisevent, 128 );
//                state->conio->puts(buffer);
//            } else	{
//                state->conio->puts("\n\r");
//                return 1;
//            }
//        }
//        for( i = 0; i <= indexT2; i++)	{
//            if(ReadEventFromFlash( i, &thisevent ))	{
//                BufPrintCidEvent( buffer, &thisevent, 128 );
//                state->conio->puts(buffer);
//            } else	{
//                state->conio->puts("\n\r");
//                return 1;
//            }
//        }
//    }
//    state->conio->puts("\n\r");
//    return 1;
//}


int con_setrtc(ConsoleState* state)
{
	char temp[12], *contimestamp;
	int tempint, i;
	struct tm currtime, currtime1;

	time_t tmpSEC_TIMER;

	uint32_t error;
	uint32_t ts_inicial, ts_nuevo, ts_licvence, delta_ts, ts_nuevolicvence;

	ts_inicial = SEC_TIMER;

	if(state->numparams != 4)	{
		state->conio->puts("Error en cantidad de parametros\n\r");
		return -1;
	}

	contimestamp = con_getparam(state->command, 2);

	//convierto a formato timestamp de eventos
	for( i = 0; i < 4; i++)
		temp[i] = contimestamp[i];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_year = tempint - YEAR0;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+4];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mon = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+6];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mday = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+8];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_hour = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+10];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_min = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = contimestamp[i+12];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_sec = tempint;

	tempint = atoi(con_getparam(state->command, 3));
	currtime.tm_wday = tempint-1;

	logCidEvent(account, 1, 944, 0, 0);

	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, currtime.tm_sec);
	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, currtime.tm_min);
	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, currtime.tm_hour);
	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, currtime.tm_mon);
	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, currtime.tm_year);
	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, currtime.tm_mday);
	RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK, currtime.tm_wday);

	currtime.tm_mon -= 1;

	SEC_TIMER = mktime(&currtime);

	//----------------------------------------------------------------
	currtime1.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
	currtime1.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
	currtime1.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
	currtime1.tm_mon = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MONTH);
	currtime1.tm_year = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
	currtime1.tm_mday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
	currtime1.tm_wday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK);
    currtime1.tm_mon -= 1;
	tmpSEC_TIMER = mktime(&currtime1);

	//---------------------------------------------------------------
    fsm_wdog_evo( 99, 0 );
	ts_nuevo = SEC_TIMER;

	logCidEvent(account, 1, 945, 0, 0);

	error = flash0_read(temp, DF_LICTSVENC_OFFSET, 4);
	ts_licvence  = temp[3];
	ts_licvence += ((temp[2] << 8)  & 0x0000FF00);
	ts_licvence += ((temp[1] << 16) & 0x00FF0000);
	ts_licvence += ((temp[0] << 24) & 0xFF000000);

	if( ts_inicial <= ts_licvence)	{		//si hay licencia activa

		delta_ts = ts_licvence - ts_inicial;

		if( ts_nuevo > ts_licvence)	{		//jodete, hiciste que se venciera la licencia
			temp[0] = 0xA5;         //se acabo la licensia para el login
            error = flash0_write(1, temp, LOGIN_ENABLED, 1);
            RADAR_flags &= ~LIC_ENTER;
		} else	{
			ts_nuevolicvence = ts_nuevo + delta_ts;

			temp[0] = (ts_nuevolicvence >> 24) & 0x000000FF;
			temp[1] = (ts_nuevolicvence >> 16) & 0x000000FF;
			temp[2] = (ts_nuevolicvence >> 8) & 0x000000FF;
			temp[3] = (ts_nuevolicvence) & 0x000000FF;
			error = flash0_write(1, temp, DF_LICTSVENC_OFFSET, 4);		//guarde vencimiento de licensia
		}
	}

	return 1;
}

int con_set1rtc(ConsoleState* state)
{
	char temp[12], *contimestamp;
	int tempint, i;
	struct tm currtime;
	time_t pgmsectime;
	uint32_t error;
	uint32_t ts_inicial, ts_nuevo, ts_licvence, delta_ts, ts_nuevolicvence;

    ts_inicial = SEC_TIMER;

    if(state->numparams != 4)	{
        state->conio->puts("Error en cantidad de parametros\n\r");
        return -1;
    }

    contimestamp = con_getparam(state->command, 2);

    //convierto a formato timestamp de eventos
    for( i = 0; i < 4; i++)
        temp[i] = contimestamp[i];
    temp[i] = '\0';
    tempint = atoi(temp);
    currtime.tm_year = tempint - YEAR0;

    for( i = 0; i < 2; i++)
        temp[i] = contimestamp[i+4];
    temp[i] = '\0';
    tempint = atoi(temp);
    currtime.tm_mon = tempint;

    for( i = 0; i < 2; i++)
        temp[i] = contimestamp[i+6];
    temp[i] = '\0';
    tempint = atoi(temp);
    currtime.tm_mday = tempint;

    for( i = 0; i < 2; i++)
        temp[i] = contimestamp[i+8];
    temp[i] = '\0';
    tempint = atoi(temp);
    currtime.tm_hour = tempint;

    for( i = 0; i < 2; i++)
        temp[i] = contimestamp[i+10];
    temp[i] = '\0';
    tempint = atoi(temp);
    currtime.tm_min = tempint;

    for( i = 0; i < 2; i++)
        temp[i] = contimestamp[i+12];
    temp[i] = '\0';
    tempint = atoi(temp);
    currtime.tm_sec = tempint;

    tempint = atoi(con_getparam(state->command, 3));
    currtime.tm_wday = tempint-1;

    //logCidEvent(account, 1, 944, 0, 0);

    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, currtime.tm_sec);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, currtime.tm_min);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, currtime.tm_hour);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, currtime.tm_mon);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, currtime.tm_year);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, currtime.tm_mday);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK, currtime.tm_wday);

    currtime.tm_mon -= 1;

    SEC_TIMER = mktime(&currtime);
    fsm_wdog_evo( 99, 0 );
    ts_nuevo = SEC_TIMER;

    //logCidEvent(account, 1, 945, 0, 0);

    error = flash0_read(temp, DF_LICTSVENC_OFFSET, 4);
    ts_licvence  = temp[3];
    ts_licvence += ((temp[2] << 8)  & 0x0000FF00);
    ts_licvence += ((temp[1] << 16) & 0x00FF0000);
    ts_licvence += ((temp[0] << 24) & 0xFF000000);

    if( ts_inicial <= ts_licvence)	{		//si hay licencia activa

        delta_ts = ts_licvence - ts_inicial;

        if( ts_nuevo > ts_licvence)	{		//jodete, hiciste que se venciera la licencia
            temp[0] = 0xA5;         //se acabo la licensia para el login
            error = flash0_write(1, temp, LOGIN_ENABLED, 1);
            RADAR_flags &= ~LIC_ENTER;
        } else	{
            ts_nuevolicvence = ts_nuevo + delta_ts;

            temp[0] = (ts_nuevolicvence >> 24) & 0x000000FF;
            temp[1] = (ts_nuevolicvence >> 16) & 0x000000FF;
            temp[2] = (ts_nuevolicvence >> 8) & 0x000000FF;
            temp[3] = (ts_nuevolicvence) & 0x000000FF;
            error = flash0_write(1, temp, DF_LICTSVENC_OFFSET, 4);		//guarde vencimiento de licensia
        }
    }






    //pgmsectime = mktime(&currtime);

    //if( (pgmsectime > (SEC_TIMER - 10*60)) && (pgmsectime < (SEC_TIMER + 10*60)) )
    //    return 1;


	//return 1;

	LLAVE_TX_OFF();
	POWER_TX_OFF();

	while(1);

}

int con_dumprtc(ConsoleState* state)
{
	struct tm mytime;
	time_t timestamp;



	timestamp = SEC_TIMER;
	gmtime((const time_t *) &(timestamp), &mytime);

	state->conio->puts(asctime(&mytime));
	state->conio->puts("\n\r");

	return 1;
}

int con_dumprtc2(ConsoleState* state)
{
	struct tm mytime;
	time_t timestamp;



	timestamp = SEC_TIMER;
	gmtime((const time_t *) &(timestamp), &mytime);

	state->conio->puts("Timestamp     ");
	state->conio->puts(asctime(&mytime));
	state->conio->puts("\n\r");

	mytime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
	mytime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
	mytime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
	mytime.tm_mon = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MONTH) - 1;
	mytime.tm_year = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
	mytime.tm_mday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
	mytime.tm_wday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK);

	state->conio->puts("Hard Clock    ");
	state->conio->puts(asctime(&mytime));
	state->conio->puts("\n\r");

	return 1;
}


int find_event_by_index( uint16_t fnd_index, uint32_t *eveaddr)
{
	uint32_t fnd_dfindex;
	uint16_t fnd_evflash_wrptr, read_index, count;
	uint8_t buffer_read[DF_EVELEN];
	OS_ERR os_err;

	if(evflash_wrptr != 0)	{
		fnd_evflash_wrptr = evflash_wrptr-1;
	} else	{
		fnd_evflash_wrptr = DF_MAXEVENTS - 1;
	}

	count = 0;

	while( count < DF_MAXEVENTS)	{
		fnd_dfindex = DF_EVENT0 + (fnd_evflash_wrptr * DF_EVELEN);
		flash0_read(buffer_read, fnd_dfindex, DF_EVELEN);
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
		if(buffer_read[0] == 0xFF)	{
			*eveaddr = 0;
			return FALSE;
		}

		read_index = ((buffer_read[1] << 8) & 0xFF00) + buffer_read[2];

		if(read_index == fnd_index)	{
			*eveaddr = fnd_dfindex;
			return TRUE;
		}

		if(fnd_evflash_wrptr == 0)
			fnd_evflash_wrptr = DF_MAXEVENTS - 1;
		else
			fnd_evflash_wrptr--;
		count++;
	}

	return FALSE;
}

int ReloadUnAckEvents25( int monid)
{
    uint16_t last_evflash_wrptr, count;
    int retval, qty,discard;

    uint8_t mask;
    EventRecord event;
    OS_ERR os_err;

    if(LogT_eventRec_count) {
        return 0;
    }

    mask = (1 << monid);

    if(evflash_wrptr > 24)  {
        last_evflash_wrptr = evflash_wrptr - 25;
    } else  {
        last_evflash_wrptr = DF_MAXEVENTS - (25 - evflash_wrptr);
    }

//    if(evflash_wrptr != 0)	{
//        last_evflash_wrptr = evflash_wrptr-1;
//    } else	{
//        last_evflash_wrptr = DF_MAXEVENTS - 1;
//    }

    qty = 0;
    for(count = 0; count < 25; count++) {
        retval = ReadEventFromFlash( last_evflash_wrptr, &event );
        WDT_Feed();
        if(retval == TRUE)  {
            if(!(event.ack_tag & mask) )   {
                discard = 0;
                //------------------------------------
                // Eventos que no quiero recargar
                //filtro las señalizacion de uso interno en zonas 189 a 192
                if((event.cid_zoneuser >= 0x189) && (event.cid_zoneuser <= 0x192))	{
                    discard++;
                }

                //filtro los eventos de zonas 5 a 8, de las evo, para los eventos entre 100 y 199
                if((event.cid_zoneuser >= ASALPBT_FIRSTZONE) && (event.cid_zoneuser <= ASALPBT_LASTZONE))	{
                    if((event.cid_eventcode >= 0x100) && (event.cid_eventcode <= 0x199))	{
                        if((event.cid_partition >= 0x70) && (event.cid_partition <= 0x88)) {
                            discard++;
                        }
                    }
                }
                //------------------------------------
                if( discard == 0) {
                    WriteEventToTxBuffer(monid, &event);
                    qty++;
                }
            }
        }

        if(last_evflash_wrptr == DF_MAXEVENTS - 1)
            last_evflash_wrptr = 0;
        else
            last_evflash_wrptr++;
        OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
    }
    return qty;
}

void ReloadUnAckEvents( int monid)
{
	uint16_t last_evflash_wrptr, count;
	int retval;

	uint8_t mask;
	EventRecord event;
	OS_ERR os_err;

	mask = (1 << monid);

	if(evflash_wrptr != 0)	{
		last_evflash_wrptr = evflash_wrptr-1;
	} else	{
		last_evflash_wrptr = DF_MAXEVENTS - 1;
	}

	count = 0;
	while( count < DF_MAXEVENTS)	{
        WDT_Feed();
        retval = ReadEventFromFlash( last_evflash_wrptr, &event );
		if( retval == FALSE)	{
			return;
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
		if(event.ack_tag & mask )	{
			break;
		}

		if(last_evflash_wrptr == 0)
			last_evflash_wrptr = DF_MAXEVENTS - 1;
		else
			last_evflash_wrptr--;
		count++;
	}
	if(count == DF_MAXEVENTS)	{
		return;
	} else
	if(count == 0)
		return;

	while( last_evflash_wrptr < (evflash_wrptr - 1) )	{
        WDT_Feed();
        retval = ReadEventFromFlash( last_evflash_wrptr, &event );
		if( retval == FALSE)	{
			return;
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
		WriteEventToTxBuffer(monid, &event);
        WDT_Feed();
		last_evflash_wrptr++;
	}

}
