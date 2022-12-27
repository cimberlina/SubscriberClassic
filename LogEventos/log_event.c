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



EventRecord currentEvent, LcurrentEvent;
uint16_t eventIndex;

// de la implementacion de buffer circular para flash de eventos
uint16_t evflash_wrptr;
uint16_t evflash_rdptr;
uint16_t audit_evflash_wrptr;
uint16_t audit_evflash_rdptr;


const uint8_t hexachar[16] = {'0', '1', '2', '3', '4','5','6','7','8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void RawCID_LogEvent( uint8_t *cidbuff)
{
	uint16_t checksum;
	int i, len;
	uint8_t *currentEventPtr;
	char strbuffer[256];

	currentEvent.index = eventIndex++;
	if(eventIndex == 0xFFFF)
		eventIndex = 0x0000;
	currentEvent.timestamp = SEC_TIMER;

	currentEvent.account = (cidbuff[0] * 0x1000) + (cidbuff[1] * 0x0100) + (cidbuff[2] * 0x0010) + cidbuff[3];
	currentEvent.cid_qualifier = cidbuff[6];
	currentEvent.cid_eventcode =  (cidbuff[7] * 0x0100) + (cidbuff[8] * 0x0010) + cidbuff[9];
	currentEvent.cid_partition = (cidbuff[10] * 0x0010) + cidbuff[11];
	currentEvent.cid_zoneuser = (cidbuff[12] * 0x0100) + (cidbuff[13] * 0x0010) + cidbuff[14];

	currentEventPtr = (uint8_t *)(&currentEvent);
	for(i = 0, checksum = 0; i < 14; i++)	{
		checksum += *(currentEventPtr + i);
	}
	checksum &= 0x00FF;

	currentEvent.checksum = (uint8_t)checksum;
	currentEvent.ack_tag = 0;

	len = BufPrintCidEvent( strbuffer, &currentEvent, 256 );

	CommSendString(COMM0, strbuffer);
	WriteEventToFlash(&currentEvent);


	for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
		if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )
			WriteEventToTxBuffer(i, &currentEvent);
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

	LcurrentEvent.index = eventIndex++;
	if(eventIndex == 0xFFFF)
		eventIndex = 0x0000;

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

	if(eventcode < 999)	{
		for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
			if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )	{
				switch(Monitoreo[i].protocol)	{
				case AP_NTSEC4:
				case AP_NTSEC5:
				case AP_NTSEC6:
				case AP_NTSEC7:
					WriteEventToTxBuffer(i, &LcurrentEvent);
					break;
				case AP_EYSE1:
					//heartbeat_EYSE1((uint8_t *)(&currentEvent) );
					WriteEventToTxBuffer(i, &LcurrentEvent);
					break;
				}
			}
		}
	}

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


void WriteEventToTxBuffer(int co_id, EventRecord *event)
{
	int wrptr;

	if(!((event->cid_qualifier ==  1) || (event->cid_qualifier ==  3) || (event->cid_qualifier ==  6)) )	{
		return;
	}
//	if((event->cid_eventcode == 0x000) || (event->cid_eventcode == 0xAAA) || (event->cid_eventcode >= 1000))	{
//		return;
//	}

	wrptr = Monitoreo[co_id].eventRec_writeptr++;
	Monitoreo[co_id].eventRec_count++;
	Mem_Copy( &(Monitoreo[co_id].eventRecord[wrptr]), event, sizeof(EventRecord));
	if(Monitoreo[co_id].eventRec_writeptr == TXEVENTBUFFERLEN)	{
		Monitoreo[co_id].eventRec_writeptr = 0;
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
		if( buffer[0] != 0xFF)
			count++;
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
	int error, i, chksum;
	uint8_t buffer[16];
	uint32_t dfindex;
	OS_ERR os_err;

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


	dfindex = DF_EVENT0 + (evflash_wrptr * DF_EVELEN);
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
		evflash_wrptr++;
		if( evflash_wrptr == DF_MAXEVENTS)
			evflash_wrptr = 0;
		Set_evwrptr(evflash_wrptr);
		return TRUE;
	} else return FALSE;

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


int EventCodeToInt(uint16_t eventcode)
{
	int retval;

	retval = (((eventcode >> 12) & 0x0F)*1000) + (((eventcode >> 8) & 0x0F)*100) + (((eventcode >> 4) & 0x0F)*10) + (eventcode & 0x0F);
	return retval;
}

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
	for( i = 0; i < DF_MAXEVENTS; i++)	{
		if(ReadEventFromFlash( i, &event ))	{
			if((event.timestamp >= time1) && (event.timestamp <= currmax))	{
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

int con_DumpEvMemory(ConsoleState* state)
{
	int index, index1, index2, i;
	uint32_t dfindex;
	uint8_t buffer[DF_EVELEN];
	char tmpbuffer[16];
	char tsndbuffer[132];


	for( i = 0; i < 132; i++ )	{
		tsndbuffer[i] = 0;
	}

	switch(state->numparams)	{
	case 2:
		index1 = atoi(con_getparam(state->command, 1));
		index2 = index1;
		break;
	case 3:
		index1 = atoi(con_getparam(state->command, 1));
		index2 = atoi(con_getparam(state->command, 2));
		break;
	}



	if( (index1 >= DF_MAXEVENTS) || (index2 >= DF_MAXEVENTS))	{
		state->conio->puts("Indice espcificado fuera de rango\n\r");
		return -1;
	}

	for( index = index1; index <= index2; index++)	{
		dfindex = DF_EVENT0 + (index * DF_EVELEN);
		flash0_read(buffer, dfindex, DF_EVELEN);
		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)itoa(index));
		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)";\t");
		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)itoa(dfindex));
		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)":\t ");


		for( i = 0; i < DF_EVELEN; i++)	{
			BuffPutHex( tmpbuffer, buffer[i]);
			Str_Cat(tmpbuffer, " ");
			Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)tmpbuffer);

		}
		Str_Cat((CPU_CHAR  *)tsndbuffer, (const CPU_CHAR  *)"\r\n");
		state->conio->puts(tsndbuffer);

		tsndbuffer[0] = 0;

	}
	state->conio->puts("\r\n");
	return 1;
}

int con_DumpEvIndex(ConsoleState* state)
{
	int index, len;
	EventRecord curr_event;
	char buffer[128];

	index = atoi(con_getparam(state->command, 1));

	if( index >= DF_MAXEVENTS)	{
		state->conio->puts("Indice especificado fuera de rango\n\r");
		return -1;
	}

	len = 0;
	if( ReadEventFromFlash( index, &curr_event ) )	{
		len = BufPrintCidEvent( buffer, &curr_event, 128 );
	}
	if(len)	{
		state->conio->puts(buffer);
		return 1;
	}

	state->conio->puts("Error de lectura de evento\n\r");
	return -1;
}

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

int con_DumpEventByTime(ConsoleState* state)
{
	uint16_t indexT1, indexT2, i;
	char buffer[128];
	EventRecord thisevent;
	time_t t1, t2;

	switch(state->numparams)	{
	case 1:
		i = HowManyEvents();
		if(i > evflash_wrptr)	{
			indexT1 = evflash_wrptr;
			indexT2 = evflash_wrptr -1;
		} else	{
			indexT1 = 0;
			indexT2 = evflash_wrptr -1;
		}
		break;
	case 2:
		indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
		indexT2 = evflash_wrptr - 1;
		break;
	case 3:
		t1 = ConvTimestamp(con_getparam(state->command, 1));
		t2 = ConvTimestamp(con_getparam(state->command, 2));
		if( t1 > t2)	{
			state->conio->puts("Invalid parameters\n\r");
			return -1;

		} else if( t2 > t1)	{
			indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
			indexT2 = GetTimestampIndexEv_LE(con_getparam(state->command, 2));
		} else	{
			indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
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
				BufPrintCidEvent( buffer, &thisevent, 128 );
				state->conio->puts(buffer);
			} else	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
	} else	{
		for( i = indexT1; i < DF_MAXEVENTS; i++)	{
			if(ReadEventFromFlash( i, &thisevent ))	{
				BufPrintCidEvent( buffer, &thisevent, 128 );
				state->conio->puts(buffer);
			} else	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		for( i = 0; i <= indexT2; i++)	{
			if(ReadEventFromFlash( i, &thisevent ))	{
				BufPrintCidEvent( buffer, &thisevent, 128 );
				state->conio->puts(buffer);
			} else	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
	}
	state->conio->puts("\n\r");
	return 1;
}


int con_DumpEventTypeByTime(ConsoleState* state)
{
    uint16_t indexT1, indexT2, i;
    char buffer[128];
    EventRecord thisevent;
    time_t t1, t2;

    switch(state->numparams)	{
        case 1:
            i = HowManyEvents();
            if(i > evflash_wrptr)	{
                indexT1 = evflash_wrptr;
                indexT2 = evflash_wrptr -1;
            } else	{
                indexT1 = 0;
                indexT2 = evflash_wrptr -1;
            }
            break;
        case 2:
            indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
            indexT2 = evflash_wrptr - 1;
            break;
        case 3:
            t1 = ConvTimestamp(con_getparam(state->command, 1));
            t2 = ConvTimestamp(con_getparam(state->command, 2));
            if( t1 > t2)	{
                state->conio->puts("Invalid parameters\n\r");
                return -1;

            } else if( t2 > t1)	{
                indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
                indexT2 = GetTimestampIndexEv_LE(con_getparam(state->command, 2));
            } else	{
                indexT1 = GetTimestampIndexEv_GE(con_getparam(state->command, 1));
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
                BufPrintCidEvent( buffer, &thisevent, 128 );
                state->conio->puts(buffer);
            } else	{
                state->conio->puts("\n\r");
                return 1;
            }
        }
    } else	{
        for( i = indexT1; i < DF_MAXEVENTS; i++)	{
            if(ReadEventFromFlash( i, &thisevent ))	{
                BufPrintCidEvent( buffer, &thisevent, 128 );
                state->conio->puts(buffer);
            } else	{
                state->conio->puts("\n\r");
                return 1;
            }
        }
        for( i = 0; i <= indexT2; i++)	{
            if(ReadEventFromFlash( i, &thisevent ))	{
                BufPrintCidEvent( buffer, &thisevent, 128 );
                state->conio->puts(buffer);
            } else	{
                state->conio->puts("\n\r");
                return 1;
            }
        }
    }
    state->conio->puts("\n\r");
    return 1;
}


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
