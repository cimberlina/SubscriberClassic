/*
 * protocols.c
 *
 *  Created on: Apr 25, 2012
 *      Author: ironman
 */
#include "includes.h"

uint8_t numabo;
uint8_t numcen;
//char nomsuc[50];
//char myipaddrstr[16];
uint8_t eyseAlarma;
uint8_t eyseEstados;
uint8_t eyseDisparos;

int ProtocolEncoder(int coid, int account, AlarmProtocols aprotocol, EventRecord *event, uint8_t *outputstr)
{
	int err;

	switch(aprotocol)	{
	case AP_NTSEC7:
		err = ProtEncoder_NTSEC7(account, event, outputstr, Monitoreo[coid].sec);
		break;
    case AP_NTSEC4:
    case AP_NTSEC5:
	case AP_NTSEC6:
		err = ProtEncoder_NTSEC6(account, event, outputstr, Monitoreo[coid].sec);
		break;
//	case AP_NTSEC6E:
//		err = ProtEncoder_NTSEC6E(account, event, outputstr, Monitoreo[coid].sec);
//		break;
//	case AP_NTSEC4:
//		err = ProtEncoder_NTSEC4(account, event, outputstr);
//		break;
//	case AP_NTSEC5:
//		err = ProtEncoder_NTSEC5(account, event, outputstr, Monitoreo[coid].sec);
//		break;
//	case AP_INDAV1:
//		err = ProtEncoder_INDAV1(account, event, outputstr);
//		break;
	}
	return err;
}

//int ProtEncoder_raw_EYSE1(EventRecord *event, uint8_t *outputstr)
//{
//	Mem_Copy( outputstr, event, 16);
//	return 16;
//}
//
//int ProtEncoder_EYSE1(EventRecord *event, uint8_t *outputstr)
//{
//	EventRecord *eveptr;
//	struct tm time;
//	uint8_t EyseAlarma;
//	uint8_t EyseEstados;
//	uint8_t EyseDisparos;
//	char *chbuf;
//	int checksum, i;
//
//	eveptr = event;
//
//	outputstr[0] = 0x00;
//	outputstr[1] = 0xD0;
//	outputstr[2] = numcen;
//	outputstr[3] = numabo;
//
//	ConvertEventToEYSEbytes( eveptr, &EyseAlarma, &EyseEstados, &EyseDisparos);
//	outputstr[4] = EyseAlarma;
//
//	gmtime((const time_t *) &(event->timestamp), &time);
//
//	chbuf = itoa(time.tm_hour);
//	if( time.tm_hour < 10 )	{
//		outputstr[5] = 0x30;
//		outputstr[6] = chbuf[0];
//	} else {
//		outputstr[5] = chbuf[0];
//		outputstr[6] = chbuf[1];
//	}
//
//	chbuf = itoa( time.tm_min );
//	if( time.tm_min < 10 )	{
//		outputstr[7] = 0x30;
//		outputstr[8] = chbuf[0];
//	} else {
//		outputstr[7] = chbuf[0];
//		outputstr[8] = chbuf[1];
//	}
//
//	chbuf = itoa( time.tm_mday );
//	if( time.tm_mday < 10 )	{
//		outputstr[9] = 0x30;
//		outputstr[10] = chbuf[0];
//	} else {
//		outputstr[9] = chbuf[0];
//		outputstr[10] = chbuf[1];
//	}
//
//	chbuf = itoa( time.tm_mon );
//	if( time.tm_mon < 10 )	{
//		outputstr[11] = 0x30;
//		outputstr[12] = chbuf[0];
//	} else {
//		outputstr[11] = chbuf[0];
//		outputstr[12] = chbuf[1];
//	}
//
//	outputstr[13] = EyseEstados;
//	outputstr[14] = EyseDisparos;
//
//	checksum = 0;
//	for( i = 0; i < 15; i++ )	{
//		checksum += outputstr[i];
//	}
//	outputstr[15] = (uint8_t)(checksum & 0x7F);
//
//	return 16;
//}

//void ConvertEventToEYSEbytes( EventRecord *event, uint8_t *EyseAlarma, uint8_t *EyseEstados, uint8_t *EyseDisparos)
//{
//	static uint8_t eyse_alarma, eyse_estados, eyse_disparos;
//
//	int eventocompuesto;
//
//	eventocompuesto = (event->cid_qualifier * 1000) + BCD_Word_to_int(event->cid_eventcode);
//
//	eyse_alarma = eyseAlarma;
//
//	switch(eventocompuesto)	{
//	case 1120:
//		eyse_alarma |= EYSE_ASAL_bit;
//		break;
//	case 3120:
//		eyse_alarma &= ~EYSE_ASAL_bit;
//		break;
//	case 1110:
//		eyse_alarma |= EYSE_INCE_bit;
//		break;
//	case 3110:
//		eyse_alarma &= ~EYSE_INCE_bit;
//		break;
//	case 1130:
//    case 1134:
//		eyse_alarma |= EYSE_TESO_bit;
//		switch(event->cid_zoneuser)	{
//		case CEYSE_ZI_PT1:
//			eyse_disparos |= EYSE_IPT1_bit;
//			break;
//		case CEYSE_ZT_PT1:
//			eyse_disparos |= EYSE_TPT1_bit;
//			break;
//		case CEYSE_ZI_PT2:
//			eyse_disparos |= EYSE_IPT2_bit;
//			break;
//		case CEYSE_ZT_PT2:
//			eyse_disparos |= EYSE_TPT2_bit;
//			break;
//		case CEYSE_ZI_PT3:
//			eyse_disparos |= EYSE_IPT3_bit;
//			break;
//		case CEYSE_ZT_PT3:
//			eyse_disparos |= EYSE_TPT3_bit;
//			break;
//		case CEYSE_ZI_PT4:
//			eyse_disparos |= EYSE_IPT4_bit;
//			break;
//		case CEYSE_ZT_PT4:
//			eyse_disparos |= EYSE_TPT4_bit;
//			break;
//		}
//		break;
//	case 3130:
//    case 3134:
//		eyse_alarma &= ~EYSE_TESO_bit;
//		eyse_disparos = 0x00;
//		break;
//	case 1380:
//		eyse_alarma |= EYSE_ROTU_bit;
//		break;
//	case 3380:
//		eyse_alarma &= ~EYSE_ROTU_bit;
//		break;
//	case 1137:
//		eyse_alarma |= EYSE_APER_bit;
//		break;
//	case 3137:
//		eyse_alarma &= ~EYSE_APER_bit;
//		break;
//	case 1393:
//		eyse_alarma |= EYSE_LLOP_bit;
//		break;
//	case 3393:
//		eyse_alarma &= ~EYSE_LLOP_bit;
//		break;
//	case 1301:
//		eyse_alarma |= EYSE_F220_bit;
//		break;
//	case 3301:
//		eyse_alarma &= ~EYSE_F220_bit;
//		break;
//	case 1401:
//		switch(event->cid_partition)	{
//		case CEYSE_PART_DSC:
//			eyse_estados |= EYSE_EDSC_bit;
//			break;
//		case CEYSE_PART_PT1:
//			eyse_estados |= EYSE_EPT1_bit;
//			break;
//		case CEYSE_PART_PT2:
//			eyse_estados |= EYSE_EPT2_bit;
//			break;
//		case CEYSE_PART_PT3:
//			eyse_estados |= EYSE_EPT3_bit;
//			break;
//		case CEYSE_PART_PT4:
//			eyse_estados |= EYSE_EPT4_bit;
//			break;
//		}
//		break;
//	case 3401:
//		switch(event->cid_partition)	{
//		case CEYSE_PART_DSC:
//			eyse_estados &= ~EYSE_EDSC_bit;
//			break;
//		case CEYSE_PART_PT1:
//			eyse_estados &= ~EYSE_EPT1_bit;
//			break;
//		case CEYSE_PART_PT2:
//			eyse_estados &= ~EYSE_EPT2_bit;
//			break;
//		case CEYSE_PART_PT3:
//			eyse_estados &= ~EYSE_EPT3_bit;
//			break;
//		case CEYSE_PART_PT4:
//			eyse_estados &= ~EYSE_EPT4_bit;
//			break;
//		}
//		break;
//	}
//
//	if(eyse_alarma > 0x80)	{
//		if( eyse_alarma != 0xAA )
//			eyse_alarma &= ~0x80;
//		*EyseAlarma   = eyse_alarma;
//	} else
//	if((eyse_alarma == 0x00) || (eyse_alarma == 0x80))	{
//		eyse_alarma = 0x80;
//		*EyseAlarma = 0x81;
//	} else	{
//		*EyseAlarma   = eyse_alarma;
//	}
//
//
//	*EyseEstados  = eyseEstados;	//eyse_estados;
//	*EyseDisparos = eyseDisparos;	//eyse_disparos;
//
//	if((eventocompuesto == 1356) && (BaseAlarmPkt_alarm == 0x80) )	{
//		*EyseAlarma = 0xAA;
//	}
//
//	if(eventocompuesto == 3356)	{
//		*EyseAlarma = BaseAlarmPkt_alarm;
//	}

//	eyseAlarma = *EyseAlarma;
//	eyseEstados = *EyseEstados;
//	eyseDisparos = *EyseDisparos;

//}
//int
//heartbeat_NTSEC4(int hbaccount, uint8_t *outputstr)
//{
//	char *strptr;
//	int i, j, len, checksum, error;
//	char buffer[16];
//
//	for(i = 0; i < 23; i++)	{
//		outputstr[i] = 0x00;
//	}
//
//	outputstr[0] = 0x40;
//	outputstr[1] = 0xD4;
//
//	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
//	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
//	outputstr[2] |= (SerialNumData[1] & 0x0F);
//	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
//	outputstr[3] |= (SerialNumData[3] & 0x0F);
//	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
//	outputstr[4] |= (SerialNumData[5] & 0x0F);
//	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
//	outputstr[5] |= (SerialNumData[7] & 0x0F);
//
//	strptr = itoa(hbaccount);
//	len = Str_Len((const  CPU_CHAR  *)strptr);
//	for(i = len-1, j = 9; i >= 0; i--, j--)	{
//		outputstr[j] = strptr[i] - '0';
//	}
//
//	outputstr[10] = 0x0A;
//	outputstr[11] = 0x0A;
//
//	outputstr[12] = 0x00;
//	outputstr[13] = 0x04;
//
//	outputstr[15] = Monitoreo[0].flags & 0xC0;
//
//    outputstr[17] = BaseAlarmPkt_numabo;
//    outputstr[18] = BaseAlarmPkt_alarm;
//    outputstr[19] = BaseAlarmPkt_estado_dispositivos;
//    outputstr[20] = BaseAlarmPkt_memoria_dispositivos;
//
//	checksum = 0;
//	for( i = 6; i < 21; i++ )	{
//		if(outputstr[i] == 0)
//			if(i < 12)
//				outputstr[i] = 0x0A;
//		checksum += outputstr[i];
//	}
//	while( checksum > 0)
//		checksum -= 15;
//	checksum += 15;
//
//	outputstr[21] = (uint8_t)checksum;
//
//	checksum = 0;
//	for( i = 1; i < 22; i++)
//		checksum ^= outputstr[i];
//
//	outputstr[22] = (uint8_t)checksum;
//
//	return 23;
//
//}
//
//int heartbeat_NTSEC5(int hbaccount, uint8_t *outputstr, uint8_t sec)
//{
//	char *strptr;
//	int i, j, len, checksum, error;
//	char buffer[16];
//
//	for(i = 0; i < 26; i++)	{
//		outputstr[i] = 0x00;
//	}
//
//	outputstr[0] = 0x40;
//	outputstr[1] = 0xD7;
//
//	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
//	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
//	outputstr[2] |= (SerialNumData[1] & 0x0F);
//	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
//	outputstr[3] |= (SerialNumData[3] & 0x0F);
//	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
//	outputstr[4] |= (SerialNumData[5] & 0x0F);
//	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
//	outputstr[5] |= (SerialNumData[7] & 0x0F);
//
//	strptr = itoa(hbaccount);
//	len = Str_Len((const  CPU_CHAR  *)strptr);
//	for(i = len-1, j = 9; i >= 0; i--, j--)	{
//		outputstr[j] = strptr[i] - '0';
//	}
//
//	outputstr[10] = 0x0A;
//	outputstr[11] = 0x0A;
//
//	outputstr[12] = 0x00;
//	outputstr[13] = 0x05;
//
//	outputstr[15] = Monitoreo[0].flags & 0xC0;
//
//	outputstr[17] = BaseAlarmPkt_numabo;
//	outputstr[18] = BaseAlarmPkt_alarm;
//	outputstr[19] = BaseAlarmPkt_estado_dispositivos;
//	outputstr[20] = BaseAlarmPkt_memoria_dispositivos;
//
//	checksum = 0;
//	for( i = 6; i < 21; i++ )	{
//		if(outputstr[i] == 0)
//			if(i < 12)
//				outputstr[i] = 0x0A;
//		checksum += outputstr[i];
//	}
//	while( checksum > 0)
//		checksum -= 15;
//	checksum += 15;
//
//	outputstr[21] = (uint8_t)checksum;
//
//
//	outputstr[22] = sec;
//	outputstr[23] = 0x00;
//	outputstr[24] = 0x00;
//
//	checksum = 0;
//	for( i = 1; i < 25; i++)
//		checksum ^= outputstr[i];
//
//	outputstr[25] = (uint8_t)checksum;
//
//	return 26;
//
//}

int heartbeat_NTSEC6(int hbaccount, uint8_t *outputstr, uint8_t sec)
{
	char *strptr;
	int i, j, len, checksum, error;
	struct tm time;
	time_t timestamp;
	char *chbuf;
	char buffer[16];

	for(i = 0; i < 43; i++)	{
		outputstr[i] = 0x00;
	}

	timestamp = SEC_TIMER;
	gmtime((const time_t *) &(timestamp), &time);

	outputstr[0] = 0x40;
	outputstr[1] = 0xE8;

	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
	outputstr[2] |= (SerialNumData[1] & 0x0F);
	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
	outputstr[3] |= (SerialNumData[3] & 0x0F);
	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
	outputstr[4] |= (SerialNumData[5] & 0x0F);
	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
	outputstr[5] |= (SerialNumData[7] & 0x0F);

	strptr = itoa(hbaccount);
	len = Str_Len((const  CPU_CHAR  *)strptr);
	for(i = len-1, j = 9; i >= 0; i--, j--)	{
		outputstr[j] = strptr[i] - '0';
	}

	outputstr[10] = 0x0A;
	outputstr[11] = 0x0A;

	outputstr[12] = 0x00;
	outputstr[13] = 0x06;

	outputstr[15] = Monitoreo[0].flags & 0xC0;

	outputstr[17] = BaseAlarmPkt_numabo;
	outputstr[18] = BaseAlarmPkt_alarm;
	outputstr[19] = BaseAlarmPkt_estado_dispositivos;
	outputstr[20] = BaseAlarmPkt_memoria_dispositivos;

	checksum = 0;
	for( i = 6; i < 21; i++ )	{
		if(outputstr[i] == 0)
			if(i < 12)
				outputstr[i] = 0x0A;
		checksum += outputstr[i];
	}
	while( checksum > 0)
		checksum -= 15;
	checksum += 15;

	outputstr[21] = (uint8_t)checksum;


	outputstr[22] = sec;
	outputstr[23] = 0x00;
	outputstr[24] = 0x51;

	//---------------------------------------------------------
	//agrego el timestamp
	chbuf = itoa(time.tm_hour);
	if( time.tm_hour < 10 )	{
		outputstr[28] = 0x30;
		outputstr[29] = chbuf[0];
	} else {
		outputstr[28] = chbuf[0];
		outputstr[29] = chbuf[1];
	}
	chbuf = itoa( time.tm_min );
	if( time.tm_min < 10 )	{
		outputstr[30] = 0x30;
		outputstr[31] = chbuf[0];
	} else {
		outputstr[30] = chbuf[0];
		outputstr[31] = chbuf[1];
	}
	chbuf = itoa( time.tm_sec );
	if( time.tm_sec < 10 )	{
		outputstr[32] = 0x30;
		outputstr[33] = chbuf[0];
	} else {
		outputstr[32] = chbuf[0];
		outputstr[33] = chbuf[1];
	}
	chbuf = itoa( time.tm_mon + 1 );
	if( (time.tm_mon + 1) < 10 )	{
		outputstr[36] = 0x30;
		outputstr[37] = chbuf[0];
	} else {
		outputstr[36] = chbuf[0];
		outputstr[37] = chbuf[1];
	}
	chbuf = itoa( time.tm_mday );
	if( time.tm_mday < 10 )	{
		outputstr[34] = 0x30;
		outputstr[35] = chbuf[0];
	} else {
		outputstr[34] = chbuf[0];
		outputstr[35] = chbuf[1];
	}
	chbuf = itoa( time.tm_year + YEAR0 );
	outputstr[38] = chbuf[0];
	outputstr[39] = chbuf[1];
	outputstr[40] = chbuf[2];
	outputstr[41] = chbuf[3];
	//-------------------------------------------------------

	checksum = 0;
	for( i = 1; i < 42; i++)
		checksum ^= outputstr[i];

	outputstr[42] = (uint8_t)checksum;

	return 43;
}

int heartbeat_NTSEC7(int hbaccount, uint8_t *outputstr, uint8_t sec)
{
	char *strptr;
	int i, j, len, checksum, error;
	struct tm time;
	time_t timestamp;
	char *chbuf;
	char buffer[16];

	for(i = 0; i < 95; i++)	{
		outputstr[i] = 0x00;
	}

	timestamp = SEC_TIMER;
	gmtime((const time_t *) &(timestamp), &time);

	outputstr[0] = 0x40;
	outputstr[1] = 0xF8;

	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
	outputstr[2] |= (SerialNumData[1] & 0x0F);
	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
	outputstr[3] |= (SerialNumData[3] & 0x0F);
	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
	outputstr[4] |= (SerialNumData[5] & 0x0F);
	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
	outputstr[5] |= (SerialNumData[7] & 0x0F);

	strptr = itoa(hbaccount);
	len = Str_Len((const  CPU_CHAR  *)strptr);
	for(i = len-1, j = 9; i >= 0; i--, j--)	{
		outputstr[j] = strptr[i] - '0';
	}

	outputstr[10] = 0x0A;
	outputstr[11] = 0x0A;

	outputstr[12] = 0x00;
	outputstr[13] = 0x07;

	outputstr[15] = Monitoreo[0].flags & 0xC0;

	outputstr[17] = BaseAlarmPkt_numabo;
	outputstr[18] = BaseAlarmPkt_alarm;
	outputstr[19] = BaseAlarmPkt_estado_dispositivos;
	outputstr[20] = BaseAlarmPkt_memoria_dispositivos;

	checksum = 0;
	for( i = 6; i < 21; i++ )	{
		if(outputstr[i] == 0)
			if(i < 12)
				outputstr[i] = 0x0A;
		checksum += outputstr[i];
	}
	while( checksum > 0)
		checksum -= 15;
	checksum += 15;

	outputstr[21] = (uint8_t)checksum;


	outputstr[22] = sec;
	outputstr[23] = 0x00;
	outputstr[24] = 0x51;

	//---------------------------------------------------------
	//agrego el timestamp
	chbuf = itoa(time.tm_hour);
	if( time.tm_hour < 10 )	{
		outputstr[28] = 0x30;
		outputstr[29] = chbuf[0];
	} else {
		outputstr[28] = chbuf[0];
		outputstr[29] = chbuf[1];
	}
	chbuf = itoa( time.tm_min );
	if( time.tm_min < 10 )	{
		outputstr[30] = 0x30;
		outputstr[31] = chbuf[0];
	} else {
		outputstr[30] = chbuf[0];
		outputstr[31] = chbuf[1];
	}
	chbuf = itoa( time.tm_sec );
	if( time.tm_sec < 10 )	{
		outputstr[32] = 0x30;
		outputstr[33] = chbuf[0];
	} else {
		outputstr[32] = chbuf[0];
		outputstr[33] = chbuf[1];
	}
	chbuf = itoa( time.tm_mon + 1 );
	if( (time.tm_mon + 1) < 10 )	{
		outputstr[36] = 0x30;
		outputstr[37] = chbuf[0];
	} else {
		outputstr[36] = chbuf[0];
		outputstr[37] = chbuf[1];
	}
	chbuf = itoa( time.tm_mday );
	if( time.tm_mday < 10 )	{
		outputstr[34] = 0x30;
		outputstr[35] = chbuf[0];
	} else {
		outputstr[34] = chbuf[0];
		outputstr[35] = chbuf[1];
	}
	chbuf = itoa( time.tm_year + YEAR0 );
	outputstr[38] = chbuf[0];
	outputstr[39] = chbuf[1];
	outputstr[40] = chbuf[2];
	outputstr[41] = chbuf[3];
	//-------------------------------------------------------
	// aca va la info de estado de particiones
	for( i = 0; i < MAXQTYPTM; i++)	{
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x01))	{
			if( (ptm_dcb[i].rtuaddr >= 240) && (ptm_dcb[i].rtuaddr <= 242))	{
				switch(ptm_dcb[i].rtuaddr)	{
					case 240:
						for(j = 0; j < 8; j++)	{
							if( PDX_dev_status[0] & (1 << j) )
								partition_bit_set( (outputstr + 42),  (ptm_dcb[i].particion + j + 1), 1);
						}
						break;
					case 241:
						for(j = 0; j < 8; j++)	{
							if( PDX_dev_status[1] & (1 << j) )
								partition_bit_set( (outputstr + 42),  (ptm_dcb[i].particion + j + 1), 1);
						}
						break;
					case 242:
						for(j = 0; j < 8; j++)	{
							if( PDX_dev_status[2] & (1 << j) )
								partition_bit_set( (outputstr + 42),  (ptm_dcb[i].particion + j + 1), 1);
						}
						break;
					default:
						break;
				}

			} else	{	
				partition_bit_set( (outputstr + 42),  ptm_dcb[i].particion, 1);
			}
		}
		//ahora va el tamper
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x02))	{
			if(ptm_dcb[i].rtuaddr < 100)	{
				partition_bit_set( (outputstr + 55),  ptm_dcb[i].particion, 1);
			}
		}
		//ahora va la inmediata
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x08))	{
			if(ptm_dcb[i].rtuaddr < 100)	{
				partition_bit_set( (outputstr + 68),  ptm_dcb[i].particion, 1);
			}
		}
		//ahora va la temporizada
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x04))	{
			if(ptm_dcb[i].rtuaddr < 100)	{
				partition_bit_set( (outputstr + 81),  ptm_dcb[i].particion, 1);
			}
		}
	}


	checksum = 0;
	for( i = 1; i < 94; i++)
		checksum ^= outputstr[i];

	outputstr[94] = (uint8_t)checksum;

	return 95;
}


//int heartbeat_EYSE1( uint8_t *outputstr)
//{
//
//	struct tm time;
//	uint8_t EyseAlarma;
//	uint8_t EyseEstados;
//	uint8_t EyseDisparos;
//	char *chbuf;
//	int checksum, i;
//	time_t timestamp;
//
//
//
//	outputstr[0] = 0x00;
//	outputstr[1] = 0xD0;
//	outputstr[2] = numcen;
//	outputstr[3] = numabo;
//
//	//ConvertEventToEYSEbytes( eveptr, &EyseAlarma, &EyseEstados, &EyseDisparos);
//	if( eyseAlarma == 0x80)	{
//		outputstr[4] = 0x81;
//	} else	{
//		outputstr[4] = eyseAlarma;
//	}
//
//	timestamp = SEC_TIMER;
//	gmtime((const time_t *) &(timestamp), &time);
//
//	chbuf = itoa(time.tm_hour);
//	if( time.tm_hour < 10 )	{
//		outputstr[5] = 0x30;
//		outputstr[6] = chbuf[0];
//	} else {
//		outputstr[5] = chbuf[0];
//		outputstr[6] = chbuf[1];
//	}
//
//	chbuf = itoa( time.tm_min );
//	if( time.tm_min < 10 )	{
//		outputstr[7] = 0x30;
//		outputstr[8] = chbuf[0];
//	} else {
//		outputstr[7] = chbuf[0];
//		outputstr[8] = chbuf[1];
//	}
//
//	chbuf = itoa( time.tm_mday );
//	if( time.tm_mday < 10 )	{
//		outputstr[9] = 0x30;
//		outputstr[10] = chbuf[0];
//	} else {
//		outputstr[9] = chbuf[0];
//		outputstr[10] = chbuf[1];
//	}
//
//	chbuf = itoa( time.tm_mon + 1 );
//	if( time.tm_mon < 10 )	{
//		outputstr[11] = 0x30;
//		outputstr[12] = chbuf[0];
//	} else {
//		outputstr[11] = chbuf[0];
//		outputstr[12] = chbuf[1];
//	}
//
//	outputstr[13] = eyseEstados;
//	outputstr[14] = eyseDisparos;
//
//	checksum = 0;
//	for( i = 0; i < 15; i++ )	{
//		checksum += outputstr[i];
//	}
//	outputstr[15] = (uint8_t)(checksum & 0x7F);
//
//	return 16;
//}
//
//
//int ProtEncoder_NTSEC4(int paccount, EventRecord *event, uint8_t *outputstr)
//{
//
//	char *strptr;
//	int i, j, len, checksum, error;
//	char buffer[16];
//
//	for(i = 0; i < 23; i++)	{
//		outputstr[i] = 0x00;
//	}
//
//	outputstr[0] = 0x40;
//	outputstr[1] = 0xD4;
//
//	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
//	//flash0_read(buffer, DF_SERIALNUM_OFFSET, 8);
//	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
//	outputstr[2] |= (SerialNumData[1] & 0x0F);
//	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
//	outputstr[3] |= (SerialNumData[3] & 0x0F);
//	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
//	outputstr[4] |= (SerialNumData[5] & 0x0F);
//	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
//	outputstr[5] |= (SerialNumData[7] & 0x0F);
//
//	if(paccount != 0)		{
//		if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
//			strptr = itoa(event->account);
//		} else 	{
//			strptr = itoa(paccount);
//		}
//		len = Str_Len((const  CPU_CHAR  *)strptr);
//		for(i = len-1, j = 9; i >= 0; i--, j--)	{
//			outputstr[j] = strptr[i] - '0';
//		}
//	} else {
//		outputstr[6] = ((event->account >> 12) & 0x0F);
//		outputstr[7] = ((event->account >> 8) & 0x0F);
//		outputstr[8] = ((event->account >> 4) & 0x0F);
//		outputstr[9] = (event->account & 0x0F);
//	}
//
//
//	outputstr[10] = 0x01;
//	outputstr[11] = 0x08;
//
//	outputstr[12] = event->cid_qualifier;
//
////	strptr = itoa(event->cid_eventcode);
////	len = Str_Len((const  CPU_CHAR  *)strptr);
////	for(i = len-1, j = 15; i >= 0; i--, j--)	{
////		outputstr[j] = strptr[i] - '0';
////	}
//
//	outputstr[13] = ((event->cid_eventcode >> 8) & 0x0F);
//	outputstr[14] = ((event->cid_eventcode >> 4) & 0x0F);
//	outputstr[15] = (event->cid_eventcode & 0x0F);
//
////	strptr = itoa(event->cid_partition);
////	len = Str_Len((const  CPU_CHAR  *)strptr);
////	for(i = len-1, j = 17; i >= 0; i--, j--)	{
////		outputstr[j] = strptr[i] - '0';
////	}
//
//	outputstr[16] = ((event->cid_partition >> 4) & 0x0F);
//	outputstr[17] = (event->cid_partition & 0x0F);
//
////	strptr = itoa(event->cid_zoneuser);
////	len = Str_Len((const  CPU_CHAR  *)strptr);
////	for(i = len-1, j = 20; i >= 0; i--, j--)	{
////		outputstr[j] = strptr[i] - '0';
////	}
//
//	outputstr[18] = ((event->cid_zoneuser >> 8) & 0x0F);
//	outputstr[19] = ((event->cid_zoneuser >> 4) & 0x0F);
//	outputstr[20] = (event->cid_zoneuser & 0x0F);
//
//	checksum = 0;
//	for( i = 6; i < 21; i++ )	{
//		if(outputstr[i] == 0)
//			outputstr[i] = 0x0A;
//		checksum += outputstr[i];
//	}
//	while( checksum > 0)
//		checksum -= 15;
//	checksum += 15;
//
//	outputstr[21] = (uint8_t)checksum;
//
//	checksum = 0;
//	for( i = 1; i < 22; i++)
//		checksum ^= outputstr[i];
//
//	outputstr[22] = (uint8_t)checksum;
//
//	return 23;
//}
//
//int ProtEncoder_NTSEC5(int paccount, EventRecord *event, uint8_t *outputstr, uint8_t sec)
//{
//	char *strptr;
//	int i, j, len, checksum, error;
//	char buffer[16];
//
//	for(i = 0; i < 26; i++)	{
//		outputstr[i] = 0x00;
//	}
//
//	outputstr[0] = 0x40;
//	outputstr[1] = 0xD7;
//
//	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
//	//flash0_read(buffer, DF_SERIALNUM_OFFSET, 8);
//	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
//	outputstr[2] |= (SerialNumData[1] & 0x0F);
//	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
//	outputstr[3] |= (SerialNumData[3] & 0x0F);
//	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
//	outputstr[4] |= (SerialNumData[5] & 0x0F);
//	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
//	outputstr[5] |= (SerialNumData[7] & 0x0F);
//
//	if(paccount != 0)		{
//		if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
//			strptr = itoa(event->account);
//		} else 	{
//			strptr = itoa(paccount);
//		}
//		len = Str_Len((const  CPU_CHAR  *)strptr);
//		for(i = len-1, j = 9; i >= 0; i--, j--)	{
//			outputstr[j] = strptr[i] - '0';
//		}
//	} else {
//		outputstr[6] = ((event->account >> 12) & 0x0F);
//		outputstr[7] = ((event->account >> 8) & 0x0F);
//		outputstr[8] = ((event->account >> 4) & 0x0F);
//		outputstr[9] = (event->account & 0x0F);
//	}
//
//
//	outputstr[10] = 0x01;
//	outputstr[11] = 0x08;
//
//	outputstr[12] = event->cid_qualifier;
//
//
//	outputstr[13] = ((event->cid_eventcode >> 8) & 0x0F);
//	outputstr[14] = ((event->cid_eventcode >> 4) & 0x0F);
//	outputstr[15] = (event->cid_eventcode & 0x0F);
//
//
//
//	outputstr[16] = ((event->cid_partition >> 4) & 0x0F);
//	outputstr[17] = (event->cid_partition & 0x0F);
//
//
//
//	outputstr[18] = ((event->cid_zoneuser >> 8) & 0x0F);
//	outputstr[19] = ((event->cid_zoneuser >> 4) & 0x0F);
//	outputstr[20] = (event->cid_zoneuser & 0x0F);
//
//	checksum = 0;
//	for( i = 6; i < 21; i++ )	{
//		if(outputstr[i] == 0)
//			outputstr[i] = 0x0A;
//		checksum += outputstr[i];
//	}
//	while( checksum > 0)
//		checksum -= 15;
//	checksum += 15;
//
//	outputstr[21] = (uint8_t)checksum;
//
//	outputstr[22] = sec;
//	outputstr[23] = 0x00;
//	outputstr[24] = 0x00;
//
//	checksum = 0;
//	for( i = 1; i < 25; i++)
//		checksum ^= outputstr[i];
//
//	outputstr[25] = (uint8_t)checksum;
//
//	return 26;
//
//}

int ProtEncoder_NTSEC6(int paccount, EventRecord *event, uint8_t *outputstr, uint8_t sec)
{
	char *strptr;
	int i, j, len, checksum, error;
	struct tm time;
	time_t timestamp;
	char *chbuf;
	char buffer[16];

	for(i = 0; i < 43; i++)	{
		outputstr[i] = 0x00;
	}

	outputstr[0] = 0x40;
	outputstr[1] = 0xE8;

	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
	//flash0_read(buffer, DF_SERIALNUM_OFFSET, 8);
	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
	outputstr[2] |= (SerialNumData[1] & 0x0F);
	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
	outputstr[3] |= (SerialNumData[3] & 0x0F);
	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
	outputstr[4] |= (SerialNumData[5] & 0x0F);
	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
	outputstr[5] |= (SerialNumData[7] & 0x0F);

	if(paccount != 0)		{
		if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
			strptr = itoa(event->account);
		} else 	{
			strptr = itoa(paccount);
		}

		len = Str_Len((const  CPU_CHAR  *)strptr);
		for(i = len-1, j = 9; i >= 0; i--, j--)	{
			outputstr[j] = strptr[i] - '0';
		}
	} else {
		outputstr[6] = ((event->account >> 12) & 0x0F);
		outputstr[7] = ((event->account >> 8) & 0x0F);
		outputstr[8] = ((event->account >> 4) & 0x0F);
		outputstr[9] = (event->account & 0x0F);
	}

	outputstr[10] = 0x01;
	outputstr[11] = 0x08;

	outputstr[12] = event->cid_qualifier;


	outputstr[13] = ((event->cid_eventcode >> 8) & 0x0F);
	outputstr[14] = ((event->cid_eventcode >> 4) & 0x0F);
	outputstr[15] = (event->cid_eventcode & 0x0F);



	outputstr[16] = ((event->cid_partition >> 4) & 0x0F);
	outputstr[17] = (event->cid_partition & 0x0F);



	outputstr[18] = ((event->cid_zoneuser >> 8) & 0x0F);
	outputstr[19] = ((event->cid_zoneuser >> 4) & 0x0F);
	outputstr[20] = (event->cid_zoneuser & 0x0F);

	checksum = 0;
	for( i = 6; i < 21; i++ )	{
		if(outputstr[i] == 0)
			outputstr[i] = 0x0A;
		checksum += outputstr[i];
	}
	while( checksum > 0)
		checksum -= 15;
	checksum += 15;

	outputstr[21] = (uint8_t)checksum;

	outputstr[22] = sec;
	outputstr[23] = 0x00;
	outputstr[24] = 0x51;

	//---------------------------------------------------------
	//agrego el timestamp
	timestamp = event->timestamp;
	gmtime((const time_t *) &(timestamp), &time);

	chbuf = itoa(time.tm_hour);
	if( time.tm_hour < 10 )	{
		outputstr[28] = 0x30;
		outputstr[29] = chbuf[0];
	} else {
		outputstr[28] = chbuf[0];
		outputstr[29] = chbuf[1];
	}
	chbuf = itoa( time.tm_min );
	if( time.tm_min < 10 )	{
		outputstr[30] = 0x30;
		outputstr[31] = chbuf[0];
	} else {
		outputstr[30] = chbuf[0];
		outputstr[31] = chbuf[1];
	}
	chbuf = itoa( time.tm_sec );
	if( time.tm_sec < 10 )	{
		outputstr[32] = 0x30;
		outputstr[33] = chbuf[0];
	} else {
		outputstr[32] = chbuf[0];
		outputstr[33] = chbuf[1];
	}
	chbuf = itoa( time.tm_mon + 1 );
	if( (time.tm_mon + 1) < 10 )	{
		outputstr[36] = 0x30;
		outputstr[37] = chbuf[0];
	} else {
		outputstr[36] = chbuf[0];
		outputstr[37] = chbuf[1];
	}
	chbuf = itoa( time.tm_mday );
	if( time.tm_mday < 10 )	{
		outputstr[34] = 0x30;
		outputstr[35] = chbuf[0];
	} else {
		outputstr[34] = chbuf[0];
		outputstr[35] = chbuf[1];
	}
	chbuf = itoa( time.tm_year + YEAR0 );
	outputstr[38] = chbuf[0];
	outputstr[39] = chbuf[1];
	outputstr[40] = chbuf[2];
	outputstr[41] = chbuf[3];
	//-------------------------------------------------------


	checksum = 0;
	for( i = 1; i < 42; i++)
		checksum ^= outputstr[i];

	outputstr[42] = (uint8_t)checksum;

	return 43;

}

//int ProtEncoder_NTSEC6E(int paccount, EventRecord *event, uint8_t *outputstr, uint8_t sec)
//{
//	char *strptr;
//	int i, j, len, checksum, error;
//	struct tm time;
//	time_t timestamp;
//	char *chbuf;
//	uint8_t buffer[16];
//
//	for(i = 0; i < 43; i++)	{
//		outputstr[i] = 0x00;
//	}
//
//	outputstr[0] = 0x42;
//	outputstr[1] = 0xE8;
//
//	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
//	//flash0_read(buffer, DF_SERIALNUM_OFFSET, 8);
//	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
//	outputstr[2] |= (SerialNumData[1] & 0x0F);
//	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
//	outputstr[3] |= (SerialNumData[3] & 0x0F);
//	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
//	outputstr[4] |= (SerialNumData[5] & 0x0F);
//	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
//	outputstr[5] |= (SerialNumData[7] & 0x0F);
//
//
//
//	//------------------------------------------------------------------------------
//	// cargo el evento CID en la forma clasica
//	if(paccount != 0)		{
//		if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
//			strptr = itoa(event->account);
//		} else 	{
//			strptr = itoa(paccount);
//		}
//		len = Str_Len((const  CPU_CHAR  *)strptr);
//		for(i = len-1, j = 9; i >= 0; i--, j--)	{
//			outputstr[j] = strptr[i] - '0';
//		}
//	} else {
//		outputstr[6] = ((event->account >> 12) & 0x0F);
//		outputstr[7] = ((event->account >> 8) & 0x0F);
//		outputstr[8] = ((event->account >> 4) & 0x0F);
//		outputstr[9] = (event->account & 0x0F);
//	}
//
//	outputstr[10] = 0x01;
//	outputstr[11] = 0x08;
//
//	outputstr[12] = event->cid_qualifier;
//
//
//	outputstr[13] = ((event->cid_eventcode >> 8) & 0x0F);
//	outputstr[14] = ((event->cid_eventcode >> 4) & 0x0F);
//	outputstr[15] = (event->cid_eventcode & 0x0F);
//
//
//
//	outputstr[16] = ((event->cid_partition >> 4) & 0x0F);
//	outputstr[17] = (event->cid_partition & 0x0F);
//
//
//
//	outputstr[18] = ((event->cid_zoneuser >> 8) & 0x0F);
//	outputstr[19] = ((event->cid_zoneuser >> 4) & 0x0F);
//	outputstr[20] = (event->cid_zoneuser & 0x0F);
//
//	checksum = 0;
//	for( i = 6; i < 21; i++ )	{
//		if(outputstr[i] == 0)
//			outputstr[i] = 0x0A;
//		checksum += outputstr[i];
//	}
//	while( checksum > 0)
//		checksum -= 15;
//	checksum += 15;
//
//	outputstr[21] = (uint8_t)checksum;
//
//	//-------------------------------------------------------------------------------------
//	// encripto el evento y lo vuelvo a cargar
//	for(i = 0; i < 16; i++)	{
//		buffer[i] = outputstr[i + 6];
//	}
//
//	aes_encrypt( buffer, (unsigned char *)AES_key);
//
//	for(i = 0; i < 16; i++)	{
//		outputstr[i + 6] = buffer[i];
//	}
//
//	//---------------------------------------------------------------------------------
//
//	outputstr[22] = sec;
//	outputstr[23] = 0x00;
//	outputstr[24] = 0x51;
//
//	//---------------------------------------------------------
//	//agrego el timestamp
//	timestamp = event->timestamp;
//	gmtime((const time_t *) &(timestamp), &time);
//
//	chbuf = itoa(time.tm_hour);
//	if( time.tm_hour < 10 )	{
//		outputstr[28] = 0x30;
//		outputstr[29] = chbuf[0];
//	} else {
//		outputstr[28] = chbuf[0];
//		outputstr[29] = chbuf[1];
//	}
//	chbuf = itoa( time.tm_min );
//	if( time.tm_min < 10 )	{
//		outputstr[30] = 0x30;
//		outputstr[31] = chbuf[0];
//	} else {
//		outputstr[30] = chbuf[0];
//		outputstr[31] = chbuf[1];
//	}
//	chbuf = itoa( time.tm_sec );
//	if( time.tm_sec < 10 )	{
//		outputstr[32] = 0x30;
//		outputstr[33] = chbuf[0];
//	} else {
//		outputstr[32] = chbuf[0];
//		outputstr[33] = chbuf[1];
//	}
//	chbuf = itoa( time.tm_mon + 1 );
//	if( (time.tm_mon + 1) < 10 )	{
//		outputstr[36] = 0x30;
//		outputstr[37] = chbuf[0];
//	} else {
//		outputstr[36] = chbuf[0];
//		outputstr[37] = chbuf[1];
//	}
//	chbuf = itoa( time.tm_mday );
//	if( time.tm_mday < 10 )	{
//		outputstr[34] = 0x30;
//		outputstr[35] = chbuf[0];
//	} else {
//		outputstr[34] = chbuf[0];
//		outputstr[35] = chbuf[1];
//	}
//	chbuf = itoa( time.tm_year + YEAR0 );
//	outputstr[38] = chbuf[0];
//	outputstr[39] = chbuf[1];
//	outputstr[40] = chbuf[2];
//	outputstr[41] = chbuf[3];
//	//-------------------------------------------------------
//
//
//	checksum = 0;
//	for( i = 1; i < 42; i++)
//		checksum ^= outputstr[i];
//
//	outputstr[42] = (uint8_t)checksum;
//
//	return 43;
//
//}


int ProtEncoder_NTSEC7(int paccount, EventRecord *event, uint8_t *outputstr, uint8_t sec)
{
	char *strptr;
	int i, j, len, checksum, error;
	struct tm time;
	time_t timestamp;
	char *chbuf;
	char buffer[16];

	for(i = 0; i < 95; i++)	{
		outputstr[i] = 0x00;
	}

	outputstr[0] = 0x40;
	outputstr[1] = 0xF8;

	//EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
	//flash0_read(buffer, DF_SERIALNUM_OFFSET, 8);
	outputstr[2] = ((SerialNumData[0] & 0x0F) << 4);
	outputstr[2] |= (SerialNumData[1] & 0x0F);
	outputstr[3] = ((SerialNumData[2] & 0x0F) << 4);
	outputstr[3] |= (SerialNumData[3] & 0x0F);
	outputstr[4] = ((SerialNumData[4] & 0x0F) << 4);
	outputstr[4] |= (SerialNumData[5] & 0x0F);
	outputstr[5] = ((SerialNumData[6] & 0x0F) << 4);
	outputstr[5] |= (SerialNumData[7] & 0x0F);

	if(paccount != 0)		{
		if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
			strptr = itoa(event->account);
		} else 	{
			strptr = itoa(paccount);
		}
		len = Str_Len((const  CPU_CHAR  *)strptr);
		for(i = len-1, j = 9; i >= 0; i--, j--)	{
			outputstr[j] = strptr[i] - '0';
		}
	} else {
		outputstr[6] = ((event->account >> 12) & 0x0F);
		outputstr[7] = ((event->account >> 8) & 0x0F);
		outputstr[8] = ((event->account >> 4) & 0x0F);
		outputstr[9] = (event->account & 0x0F);
	}


	outputstr[10] = 0x01;
	outputstr[11] = 0x08;

	outputstr[12] = event->cid_qualifier;


	outputstr[13] = ((event->cid_eventcode >> 8) & 0x0F);
	outputstr[14] = ((event->cid_eventcode >> 4) & 0x0F);
	outputstr[15] = (event->cid_eventcode & 0x0F);



	outputstr[16] = ((event->cid_partition >> 4) & 0x0F);
	outputstr[17] = (event->cid_partition & 0x0F);



	outputstr[18] = ((event->cid_zoneuser >> 8) & 0x0F);
	outputstr[19] = ((event->cid_zoneuser >> 4) & 0x0F);
	outputstr[20] = (event->cid_zoneuser & 0x0F);

	checksum = 0;
	for( i = 6; i < 21; i++ )	{
		if(outputstr[i] == 0)
			outputstr[i] = 0x0A;
		checksum += outputstr[i];
	}
	while( checksum > 0)
		checksum -= 15;
	checksum += 15;

	outputstr[21] = (uint8_t)checksum;

	outputstr[22] = sec;
	outputstr[23] = 0x00;
	outputstr[24] = 0x51;

	//---------------------------------------------------------
	//agrego el timestamp
	timestamp = event->timestamp;
	gmtime((const time_t *) &(timestamp), &time);

	chbuf = itoa(time.tm_hour);
	if( time.tm_hour < 10 )	{
		outputstr[28] = 0x30;
		outputstr[29] = chbuf[0];
	} else {
		outputstr[28] = chbuf[0];
		outputstr[29] = chbuf[1];
	}
	chbuf = itoa( time.tm_min );
	if( time.tm_min < 10 )	{
		outputstr[30] = 0x30;
		outputstr[31] = chbuf[0];
	} else {
		outputstr[30] = chbuf[0];
		outputstr[31] = chbuf[1];
	}
	chbuf = itoa( time.tm_sec );
	if( time.tm_sec < 10 )	{
		outputstr[32] = 0x30;
		outputstr[33] = chbuf[0];
	} else {
		outputstr[32] = chbuf[0];
		outputstr[33] = chbuf[1];
	}
	chbuf = itoa( time.tm_mon + 1 );
	if( (time.tm_mon + 1) < 10 )	{
		outputstr[36] = 0x30;
		outputstr[37] = chbuf[0];
	} else {
		outputstr[36] = chbuf[0];
		outputstr[37] = chbuf[1];
	}
	chbuf = itoa( time.tm_mday );
	if( time.tm_mday < 10 )	{
		outputstr[34] = 0x30;
		outputstr[35] = chbuf[0];
	} else {
		outputstr[34] = chbuf[0];
		outputstr[35] = chbuf[1];
	}
	chbuf = itoa( time.tm_year + YEAR0 );
	outputstr[38] = chbuf[0];
	outputstr[39] = chbuf[1];
	outputstr[40] = chbuf[2];
	outputstr[41] = chbuf[3];

	//-------------------------------------------------------
	// aca va la info de estado de particiones
	for( i = 0; i < MAXQTYPTM; i++)	{
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x01))	{
			if( (ptm_dcb[i].rtuaddr >= 240) && (ptm_dcb[i].rtuaddr <= 242))	{
				switch(ptm_dcb[i].rtuaddr)	{
					case 240:
						for(j = 0; j < 8; j++)	{
							if( PDX_dev_status[0] & (1 << j) )
								partition_bit_set( (outputstr + 42),  (ptm_dcb[i].particion + j + 1), 1);
						}
						break;
					case 241:
						for(j = 0; j < 8; j++)	{
							if( PDX_dev_status[1] & (1 << j) )
								partition_bit_set( (outputstr + 42),  (ptm_dcb[i].particion + j + 1), 1);
						}
						break;
					case 242:
						for(j = 0; j < 8; j++)	{
							if( PDX_dev_status[2] & (1 << j) )
								partition_bit_set( (outputstr + 42),  (ptm_dcb[i].particion + j + 1), 1);
						}
						break;
					default:
						break;
				}

			} else	{	
				partition_bit_set( (outputstr + 42),  ptm_dcb[i].particion, 1);
			}
		}
		//ahora va el tamper
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x02))	{
			if(ptm_dcb[i].rtuaddr < 100)	{
				partition_bit_set( (outputstr + 55),  ptm_dcb[i].particion, 1);
			}
		}
		//ahora va la inmediata
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x08))	{
			if(ptm_dcb[i].rtuaddr < 100)	{
				partition_bit_set( (outputstr + 68),  ptm_dcb[i].particion, 1);
			}
		}
		//ahora va la temporizada
		if((ptm_dcb[i].rtuaddr != 0x00) && (PTM_dev_status[i] & 0x04))	{
			if(ptm_dcb[i].rtuaddr < 100)	{
				partition_bit_set( (outputstr + 81),  ptm_dcb[i].particion, 1);
			}
		}
	}
	//-------------------------------------------------------

	checksum = 0;
	for( i = 1; i < 94; i++)
		checksum ^= outputstr[i];

	outputstr[94] = (uint8_t)checksum;

	return 95;

}

//int ProtEncoder_INDAV1(int paccount, EventRecord *event, uint8_t *outputstr)
//{
//
//	return 1;
//}

int sendEyseId(uint8_t *outputstr)
{
	Str_Copy(outputstr, "@@0001");
	strcat(outputstr, itoa(numcen));
	strcat(outputstr, "#");
	strcat(outputstr, itoa(numabo));
	strcat(outputstr, "#:V2.0: ");
	//strcat(outputstr, nomsuc);
	strcat(outputstr, " IP:");
//	strcat(outputstr, myipaddrstr);
	//strcat(outputstr, "192.168.3.215");
	strcat(outputstr, "@#");
	return Str_Len(outputstr);
}

int ProtEncoder_R3KCID(EventRecord *event, uint8_t *outputstr)
{
	int i, j, len;
	uint16_t lr3kaccount;
	char *strptr;


	for(i = 0; i < 24; i++)	{
		outputstr[i] = 0x00;
	}

	outputstr[0] = 0x35;
	outputstr[1] = 0x30;
	outputstr[2] = 0x35;
	outputstr[3] = 0x31;
	outputstr[4] = 0x20;
	outputstr[5] = 0x31;
	outputstr[6] = 0x38;


	if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
		strptr = itoa(event->account);
		len = Str_Len((const  CPU_CHAR  *)strptr);
		outputstr[7] = '0'; outputstr[8] = '0'; outputstr[9] = '0'; outputstr[10] = '0';
		for(i = len-1, j = 10; i >= 0; i--, j--)	{
			outputstr[j] = strptr[i];
		}
	} else 	{
		lr3kaccount = AccountToDigits(r3kaccount);
		outputstr[7] = ((lr3kaccount >> 12) & 0x0F) +'0';
		outputstr[8] = ((lr3kaccount >> 8) & 0x0F) +'0';
		outputstr[9] = ((lr3kaccount >> 4) & 0x0F) +'0';
		outputstr[10] = (lr3kaccount & 0x0F) +'0';
	}

	if(event->cid_qualifier == 1)
		outputstr[11] = 'E';
	else
		outputstr[11] = 'R';


	outputstr[12] = ((event->cid_eventcode >> 8) & 0x0F) +'0';
	outputstr[13] = ((event->cid_eventcode >> 4) & 0x0F) +'0';
	outputstr[14] = (event->cid_eventcode & 0x0F) +'0';

	outputstr[15] = ((event->cid_partition >> 4) & 0x0F) +'0';
	outputstr[16] = (event->cid_partition & 0x0F) +'0';

	outputstr[17] = ((event->cid_zoneuser >> 8) & 0x0F) +'0';
	outputstr[18] = ((event->cid_zoneuser >> 4) & 0x0F) +'0';
	outputstr[19] = (event->cid_zoneuser & 0x0F) +'0';



	outputstr[20] = 0x14;
	outputstr[21] = 0x0A;
	outputstr[22] = 0x0D;

	return 23;
}

int ProtEncoder_R3KCID_T(EventRecord *event, uint8_t *outputstr)
{
	int i, j, len;
	uint16_t lr3kaccount;
	char *strptr;

	struct tm time;
	time_t timestamp;
	char *chbuf;

	for(i = 0; i < 38; i++)	{
		outputstr[i] = 0x00;
	}

	outputstr[0] = 0x35;
	outputstr[1] = 0x30;
	outputstr[2] = 0x35;
	outputstr[3] = 0x31;
	outputstr[4] = 0x20;
	outputstr[5] = 0x31;
	outputstr[6] = 0x38;

	if( event->cid_eventcode == EventCodeToDigits(REMOTEASALT_TRIGGER))	{
		strptr = itoa(event->account);
		len = Str_Len((const  CPU_CHAR  *)strptr);
		outputstr[7] = '0'; outputstr[8] = '0'; outputstr[9] = '0'; outputstr[10] = '0';
		for(i = len-1, j = 10; i >= 0; i--, j--)	{
			outputstr[j] = strptr[i];
		}
	} else 	{
		lr3kaccount = AccountToDigits(r3kaccount);
		outputstr[7] = ((lr3kaccount >> 12) & 0x0F) +'0';
		outputstr[8] = ((lr3kaccount >> 8) & 0x0F) +'0';
		outputstr[9] = ((lr3kaccount >> 4) & 0x0F) +'0';
		outputstr[10] = (lr3kaccount & 0x0F) +'0';
	}


//	outputstr[7] = ((event->account >> 12) & 0x0F) +'0';
//	outputstr[8] = ((event->account >> 8) & 0x0F) +'0';
//	outputstr[9] = ((event->account >> 4) & 0x0F) +'0';
//	outputstr[10] = (event->account & 0x0F) +'0';

	if(event->cid_qualifier == 1)
		outputstr[11] = 'E';
	else
		outputstr[11] = 'R';


	outputstr[12] = ((event->cid_eventcode >> 8) & 0x0F) +'0';
	outputstr[13] = ((event->cid_eventcode >> 4) & 0x0F) +'0';
	outputstr[14] = (event->cid_eventcode & 0x0F) +'0';

	outputstr[15] = ((event->cid_partition >> 4) & 0x0F) +'0';
	outputstr[16] = (event->cid_partition & 0x0F) +'0';

	outputstr[17] = ((event->cid_zoneuser >> 8) & 0x0F) +'0';
	outputstr[18] = ((event->cid_zoneuser >> 4) & 0x0F) +'0';
	outputstr[19] = (event->cid_zoneuser & 0x0F) +'0';

	outputstr[20] = 0x20;
	//---------------------------------------------------------
	//agrego el timestamp
	timestamp = event->timestamp;
	gmtime((const time_t *) &(timestamp), &time);

	chbuf = itoa(time.tm_hour);
	if( time.tm_hour < 10 )	{
		outputstr[21] = 0x30;
		outputstr[22] = chbuf[0];
	} else {
		outputstr[21] = chbuf[0];
		outputstr[22] = chbuf[1];
	}
	chbuf = itoa( time.tm_min );
	if( time.tm_min < 10 )	{
		outputstr[23] = 0x30;
		outputstr[24] = chbuf[0];
	} else {
		outputstr[23] = chbuf[0];
		outputstr[24] = chbuf[1];
	}
	chbuf = itoa( time.tm_sec );
	if( time.tm_sec < 10 )	{
		outputstr[25] = 0x30;
		outputstr[26] = chbuf[0];
	} else {
		outputstr[25] = chbuf[0];
		outputstr[26] = chbuf[1];
	}
	chbuf = itoa( time.tm_mon + 1 );
	if( (time.tm_mon + 1) < 10 )	{
		outputstr[27] = 0x30;
		outputstr[28] = chbuf[0];
	} else {
		outputstr[27] = chbuf[0];
		outputstr[28] = chbuf[1];
	}
	chbuf = itoa( time.tm_mday );
	if( time.tm_mday < 10 )	{
		outputstr[29] = 0x30;
		outputstr[30] = chbuf[0];
	} else {
		outputstr[29] = chbuf[0];
		outputstr[30] = chbuf[1];
	}
	chbuf = itoa( time.tm_year + YEAR0 );
	outputstr[31] = chbuf[0];
	outputstr[32] = chbuf[1];
	outputstr[33] = chbuf[2];
	outputstr[34] = chbuf[3];
	//-------------------------------------------------------


	outputstr[35] = 0x14;
	outputstr[36] = 0x0A;
	outputstr[37] = 0x0D;

	return 38;
}

//uint8_t hex2int( uint8_t mydata)
//{
//	uint8_t temp;
//
//	temp = (mydata/10) * 0x10;
//	temp += mydata % 10;
//
//	return temp;
//}


void partition_bit_set( uint8_t *databuffer, uint8_t this_partition, uint8_t this_state)
{
	int row, column;

	if(this_partition > 99)
		return;

	row = this_partition / 8;
	column = 7 - (this_partition % 8);

	if(row > 13)
		return;

	if(this_state == 1)	{
		databuffer[row] |= (1 << column);
	} else	{
		databuffer[row] &= ~(1 << column);
	}

}