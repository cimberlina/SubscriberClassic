/*
 * lan485.c
 *
 *  Created on: Mar 14, 2013
 *      Author: claudio
 */
#include "includes.h"

uint8_t         PTMSIGNAL_flag;
time_t          ptmsignal_timer;
uint8_t         ptmsignal_state;

uint8_t cid_ptm_index;

unsigned int lap485_count;
#define VALIDPTMSTATUSLAPS 10

OS_TCB		LAN485_Task_TCB;
CPU_STK		LAN485_Task_Stk[LAN485_Task_STK_SIZE];

uint16_t wdevo_event;

uint8_t  PT_estado_particion[5];
uint8_t  VolumetricRedundance[5];

uint8_t PartDec_group[10];

uint8_t hsbc_lock_partition;
int lan485errorpkt, lan485errorcurrent;
long accumulated_errors, totalpakets;
int howmuchdev;

PBT_device	pbt_dcb[MAXPBTPTM];

//estructura de datos para manejo de los ptm
PTM_device ptm_dcb[MAXQTYPTM];
uint8_t PTM_dev_status[MAXQTYPTM];
uint8_t PDX_dev_status[3];
uint8_t PDX_dev_alarm[3];

unsigned int dlyedptm_zone[MAXQTYPTM];
unsigned int dlyedevo_part;

uint8_t diag485[8];
uint8_t habi485[8];
uint8_t devfound[32];

uint16_t evecuenta;
uint8_t eveparticion;
uint8_t eveindex;

time_t TESO_timer;
time_t ASAL_timer;
time_t INCE_timer;
time_t ROTU_timer;
time_t TECHMAN_timer;
time_t TECHMAN9_timer;

unsigned char SysFlag2;
//#define	NORM_ASAL		0x01
//#define	NORM_INCE		0x02
//#define	NORM_TESO		0x04

uint8_t relestate[10], relepulse[10];

unsigned char rfdlybornera_teso_state;

#define BORRFDLY_ST_IDLE        0x10
#define BORRFDLY_ST_WAITTRIG    0x20
#define BORRFDLY_ST_TESOGAP     0x30


unsigned int IntToBCD( unsigned int value )
{
	unsigned int temp;
	unsigned char digit1, digit2, digit3, digit4;

	digit1 = value / 1000;
	temp = value - (digit1 * 1000);
	digit2 = temp / 100;
	temp = temp - (digit2 * 100);
	digit3 = temp / 10;
	temp = temp - (digit3 * 10);
	digit4 = temp;

	return ( (digit1 * 0x1000) + (digit2 * 0x100) + (digit3 * 0x10) + digit4 );
}

unsigned int BCDToInt( unsigned int value)
{
	unsigned int temp;

	temp = ((value >> 12)& 0x000F)*1000;
	temp += ((value >> 8)& 0x000F)*100;
	temp += ((value >> 4)& 0x000F)*10;
	temp += ( value & 0x000F);
	return temp;
}

unsigned int UCharToBCD( unsigned char value )
{
	unsigned int temp;
	unsigned char digit3, digit4;


	digit3 = value / 10;
	temp = value - (digit3 * 10);
	digit4 = temp;

	return ( (digit3 * 0x10) + digit4 );
}

void  LAN485_Task(void  *p_arg)
{

	OS_ERR	os_err;
	uint8_t rxbuffer[64], ptm_index, temp_index, temp_partition, temp_pt, databuffer[6];
        uint8_t temp[3];
	int i, nread, tslotms, tslotsec, retval,signalquality;
	uint32_t rnd_pwd;
	struct tm currtime;

	(void)p_arg;

	init_lan_cfgfile();
	ptm_index = 0;
	//myAlarmByte = 0x00;
	for( i = 0; i < 10; i++)
		relepulse[i] = 0x00;

	flash0_read(temp, DF_PTMPWD_OFFSET, 2);
    if(temp[1] == 0x5A) {
        ptm_pwd = temp[0] | 0x80;
    } else {
        new_ptm_pwd();
    }

	//------------------------------------------------------------------
	// Busco que dispositivos estan colgados de la red 485
	for( i = 0; i < 32; i++ )	{
		devfound[i] = 0x00;
	}
	howmuchdev = 0;
	for( i = 0; i <= 253; i++ )	{
		WDT_Feed();
		databuffer[0] = 0x0A;
		databuffer[1] = (uint8_t)i;
		databuffer[2] = 'P';
		databuffer[3] = ptm_pwd;
		databuffer[4] = 0x0D;
		LAN485_Send( databuffer, 5 );
		//device_poll( i );
		OSTimeDlyHMSM(0, 0, 0, 60, OS_OPT_TIME_HMSM_STRICT, &os_err);
		nread = ComGetBuff(COMM2, 10, rxbuffer, 64);
		if((nread == 7) || (nread == 15) || (nread == 26))	{
		    if(rxbuffer[2] == (uint8_t)i) {
                LAN485_Send( databuffer, 5 );
                OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
                nread = ComGetBuff(COMM2, 10, rxbuffer, 64);

                if((nread == 7) || (nread == 15) || (nread == 26)) {
                    if (rxbuffer[2] == (uint8_t) i) {
                        set_array_bit(i, devfound);
                        howmuchdev++;
                    }
                }

            }
		} else	{
			reset_array_bit(i, devfound);
		}
	}

	if( howmuchdev <= 4)	{
		tslotsec = 0;
		tslotms = 250;      //250
	} else
	if ( howmuchdev <= 12)	{
		tslotsec = 0;
		tslotms = 250;      //175
	} else	{
		tslotsec = 0;
		tslotms = 250;      //135
	}

	currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
	currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
	currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
#ifdef USAR_IRIDIUM
	//---------------------------------------------------
	// Inicializacion del iridium
	IRIDIUM_flag |= IRI_USEIRIDIUM_FLAG;
	if(IRIDIUM_flag & IRI_USEIRIDIUM_FLAG)	{
		sbdixInterval = ISBD_USB_SBDIX_INTERVAL;
		atTimeout = ISBD_DEFAULT_AT_TIMEOUT;
		sendReceiveTimeout = ISBD_DEFAULT_SENDRECEIVE_TIME;
		remainingMessages = -1;
		asleep = FALSE;
		reentrant = FALSE;
		//sleepPin(sleepPinNo),
		//ringPin(ringPinNo),
		msstmWorkaroundRequested = TRUE;
		//ringAlertsEnabled(ringPinNo != -1),
		ringAsserted = FALSE;
		lastPowerOnTime = (0UL);
		head = SBDRING;
		tail = SBDRING;
		nextChar = -1 ;

		IridiumSBD_begin();

		FSMIRI_state = FSMIRI_IDLE;
		hbiri_time = 47;
		HBIRI_timer = SEC_TIMER + hbiri_time;
		iphbiri_timer = SEC_TIMER + 15*60;

		retval = getSignalQuality(&signalquality);

	}
#endif
    //---------------------------------------------------
    PTMSIGNAL_flag = 0x00;
	lan485errorcurrent = 0;
	lan485errorpkt = 0;
	accumulated_errors = 0;
    totalpakets = 0;
    lap485_count = 0;

	while(DEF_ON)	{
		WDT_Feed();


#ifdef USAR_IRIDIUM
		//----------------------
		if(IRIDIUM_flag & IRI_USEIRIDIUM_FLAG)	{
			fsm_alrm_iridium();
		}
		//----------------------
#endif

		fsm_newptmpwd();
        //fsm_wdog_ip150();
        fsm_hsbclock();
		fsm_e401_volumetrica_p5();
		fsm_e401_volumetrica_p6();
		fsm_e401_volumetrica_p7();
		fsm_e401_volumetrica_p8();
		fsm_e401_volumetrica_p9();

        fsm_rfdlyptm();
        fsm_rfdlybornera_teso();
        fsm_ptmsignalling();

		currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
		currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
		currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
		//Escaneo la red una vez por hora para detectar cualquier dispositivo colgado no declarado
		if((currtime.tm_min == 59)  &&  (!(SystemFlag5 & SCAN485_DONE)))	{
			SystemFlag5 |= SCAN485_DONE;
			Modules485_Scan();
		}
		if((currtime.tm_min == 1) && (SystemFlag5 & SCAN485_DONE))	{
			SystemFlag5 &= ~SCAN485_DONE;
		}

		ProcessRestoreMainBoardEvents();
		ProcessRestoreByTimeout();

		OSTimeDlyHMSM(0, 0, tslotsec, tslotms, OS_OPT_TIME_HMSM_STRICT, &os_err);
		for(i = 0; i < 64; i++ )
			rxbuffer[i] = 0x00;
		if(ptm_index == MAXQTYPTM )	{
			retval = AnyPTM_KeyRejected();
			ptm_index = 0;
			if(lap485_count < VALIDPTMSTATUSLAPS + 2)
			    lap485_count++;

            lan485errorpkt = lan485errorcurrent;
            lan485errorcurrent = 0;

			currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
			currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
			currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
		}

		while( ptm_dcb[ptm_index].rtuaddr == 0x00 )	{	//busco el siguiente con direccion valida
			ptm_index++;
			temp_index++;
			if(ptm_index == MAXQTYPTM )	{
				retval = AnyPTM_KeyRejected();
				ptm_index = 0;
				if(lap485_count < VALIDPTMSTATUSLAPS + 2)
				    lap485_count++;
                lan485errorpkt = lan485errorcurrent;
                lan485errorcurrent = 0;
			}
			if( temp_index == MAXQTYPTM )
				break;
		}

		if( ptm_dcb[ptm_index].rtuaddr != 0x00)	{
			reset_array_bit(ptm_dcb[ptm_index].rtuaddr, devfound);

			device_poll( ptm_index );
			OSTimeDlyHMSM(0, 0, 0, 30, OS_OPT_TIME_HMSM_STRICT, &os_err);   //30ms
			nread = ComGetBuff(COMM2, 30, rxbuffer, 64);    //30ms
			rxbuffer[nread] = 0;

			PTm_process_answer( nread, rxbuffer, ptm_index );


			if(SystemFlag3 & WDOG_EVO_ENABLE)	{
				switch(ptm_dcb[ptm_index].rtuaddr)	{       //eveindex
                    case 240:
                        fsm_wdog_evo( 0, ptm_dcb[ptm_index].particion );
                        break;
                    case 241:
                        fsm_wdog_evo( 1, ptm_dcb[ptm_index].particion );
                        break;
                    case 242:
                        fsm_wdog_evo( 2, ptm_dcb[ptm_index].particion );
                        break;
                    default:
                        break;
				}
			}

			//----------------------------------------------------------------------------
			// FSM para la deteccion de interrupcion de 485
			temp_partition = ptm_dcb[ptm_index].particion;
			if((temp_partition >= 11) & (temp_partition < 20))	{
				temp_pt = temp_partition - 11;
				temp_partition = 0;
			} else
			if((temp_partition >= 21) & (temp_partition < 30))	{
				temp_pt = temp_partition - 21;
				temp_partition = 1;
			} else
			if((temp_partition >= 31) & (temp_partition < 40))	{
				temp_pt = temp_partition - 31;
				temp_partition = 2;
			} else
			if((temp_partition >= 41) & (temp_partition < 50))	{
				temp_pt = temp_partition - 41;
				temp_partition = 3;
			} else
			if((temp_partition >= 51) & (temp_partition < 60))	{
				temp_pt = temp_partition - 51;
				temp_partition = 4;
			} else
			if((temp_partition >= 61) & (temp_partition < 70))	{
				temp_pt = temp_partition - 61;
				temp_partition = 5;
			} else
			if((temp_partition >= 70) & (temp_partition <= 78))	{
				temp_pt = 0;
				temp_partition = 6;
				habi485[temp_partition] |= (1 << 5);
			} else
			if((temp_partition >= 80) & (temp_partition <= 88))	{
				temp_pt = 1;
				temp_partition = 6;
				habi485[temp_partition] |= (1 << 6);
			} else
			if((temp_partition >= 90) & (temp_partition <= 98))	{
				temp_pt = 2;
				temp_partition = 6;
				habi485[temp_partition] |= (1 << 7);
			}

			if( temp_pt < 8)	{
				habi485[temp_partition] |= (1 << (temp_pt));
			} else	{
				habi485[7] |= (1 << (temp_partition));
			}

            if(ptm_dcb[ptm_index].rtuaddr == 240 )  {
                cid_ptm_index = ptm_index;
            }

			switch(ptm_dcb[ptm_index].state485)		{
			case P485_IDLE :
				//- - - - - - - - - - - - - - - - - - - - -
				if( temp_pt < 8)	{
					diag485[temp_partition] &= ~(1 << (temp_pt));
				} else	{
					diag485[7] &= ~(1 << (temp_partition));
				}
				//- - - - - - - - - - - - - - - - - - - - -
				if( ((ptm_dcb[ptm_index].com_error_counter >= 90) && (ptm_dcb[ptm_index].particion < 240) ) || ((ptm_dcb[ptm_index].particion >= 240) && (ptm_dcb[ptm_index].particion <= 242) && (ptm_dcb[ptm_index].com_error_counter >= 90)) )	{
					switch(ptm_dcb[ptm_index].particion)    {
                        case 10:
                        case 55:
                        case 56:
                        case 57:
                        case 58:
                        case 59:
                        case 66:
                        case 67:
                        case 92:
                        case 93:
                        case 94:
                        case 95:
                        case 96:
                        case 97:
                        case 98:
                            GenerateCIDEventPTm(ptm_index, 'E', 386,0);
                            ptm_dcb[ptm_index].state485 = P485_NG;
                            PTM_dev_status[ptm_index] = 0x00;
                            break;
                        default:
                            if(ptm_dcb[ptm_index].rtuaddr == 240 )  {
                                cid_ptm_index = ptm_index;
                            }
                            SendProblem485(ptm_index, 'E');
                            PTM_dev_status[ptm_index] = 0x00;
                            ptm_dcb[ptm_index].state485 = P485_NG;
                            ptm_dcb[ptm_index].flags |= COMM_TROUBLE;
                            PTM485NG_HistoryWrite();

                            break;
                    }

					//- - - - - - - - - - - - - - - - - - - - -
					if( temp_pt < 8)	{
						diag485[temp_partition] |= (1 << (temp_pt));
					} else	{
						diag485[7] |= (1 << (temp_partition));
					}
					//- - - - - - - - - - - - - - - - - - - - -
				}
				break;
			case P485_NG :
				//- - - - - - - - - - - - - - - - - - - - -
				if( temp_pt < 8)	{
					diag485[temp_partition] |= (1 << (temp_pt));
				} else	{
					diag485[7] |= (1 << (temp_partition));
				}
				//- - - - - - - - - - - - - - - - - - - - -
				if( ptm_dcb[ptm_index].com_error_counter == 0 )	{
					//if(BaseAlarmPkt_alarm & bitpat[APER_bit])	{
                    if(SysFlag_AP_Apertura & AP_APR_VALID)  {
						ptm_dcb[ptm_index].timeout485 = 180;
					} else	{
						ptm_dcb[ptm_index].timeout485 = 10800;
					}
                    if( Rot485_flag & CIDRESET_FLAG )   {
                        Rot485_flag &= ~CIDRESET_FLAG;
                        ptm_dcb[ptm_index].timeout485 = 1;
                    }
					ptm_dcb[ptm_index].state485 = P485_WAIT;
					//- - - - - - - - - - - - - - - - - - - - -
					if( temp_pt < 8)	{
						diag485[temp_partition] &= ~(1 << (temp_pt));
					} else	{
						diag485[7] &= ~(1 << (temp_partition));
					}
					//- - - - - - - - - - - - - - - - - - - - -
				}
				break;
			case P485_WAIT :
				//- - - - - - - - - - - - - - - - - - - - -
				if( temp_pt < 8)	{
					diag485[temp_partition] |= (1 << (temp_pt));
				} else	{
					diag485[7] |= (1 << (temp_partition));
				}
				//- - - - - - - - - - - - - - - - - - - - -
                if((ptm_dcb[ptm_index].timeout485 > 180) && (SysFlag_AP_Apertura & AP_APR_VALID))   {
                    ptm_dcb[ptm_index].timeout485 = 179;
                }
				if(!(ptm_dcb[ptm_index].timeout485))	{
                    switch(ptm_dcb[ptm_index].particion)    {
                        case 10:
                        case 55:
                        case 56:
                        case 57:
                        case 58:
                        case 59:
                        case 66:
                        case 67:
                        case 92:
                        case 93:
                        case 94:
                        case 95:
                        case 96:
                        case 97:
                        case 98:
                            GenerateCIDEventPTm(ptm_index, 'R', 386,0);
                            ptm_dcb[ptm_index].state485 = P485_IDLE;
                            PTM_dev_status[ptm_index] = 0x00;
                            break;
                        default:

                            ptm_dcb[ptm_index].flags &= ~COMM_TROUBLE;
                            SendProblem485(ptm_index, 'R');
                            ptm_dcb[ptm_index].state485 = P485_IDLE;
                            PTM485NG_HistoryWrite();

                            break;
                    }
					//- - - - - - - - - - - - - - - - - - - - -
					if( temp_pt < 8)	{
						diag485[temp_partition] &= ~(1 << (temp_pt));
					} else	{
						diag485[7] &= ~(1 << (temp_partition));
					}
					//- - - - - - - - - - - - - - - - - - - - -
				} else
				if( ((ptm_dcb[ptm_index].com_error_counter >= 90) && (ptm_dcb[ptm_index].particion < 240) ) || ((ptm_dcb[ptm_index].particion >= 240) && (ptm_dcb[ptm_index].particion <= 242) && (ptm_dcb[ptm_index].com_error_counter >= 90)) )	{
                    switch(ptm_dcb[ptm_index].particion)    {
                        case 10:
                        case 55:
                        case 56:
                        case 57:
                        case 58:
                        case 59:
                        case 66:
                        case 67:
                        case 92:
                        case 93:
                        case 94:
                        case 95:
                        case 96:
                        case 97:
                        case 98:
                            //GenerateCIDEventPTm(ptm_index, 1, 386,0);
                            ptm_dcb[ptm_index].state485 = P485_NG;
                            PTM_dev_status[ptm_index] = 0x00;
                            break;
                        default:
                            ptm_dcb[ptm_index].state485 = P485_NG;
                            ptm_dcb[ptm_index].flags |= COMM_TROUBLE;
                            PTM485NG_HistoryWrite();
                            break;
                    }
					//- - - - - - - - - - - - - - - - - - - - -
					if( temp_pt < 8)	{
						diag485[temp_partition] |= (1 << (temp_pt));
					} else	{
						diag485[7] |= (1 << (temp_partition));
					}
					//- - - - - - - - - - - - - - - - - - - - -
				}
				break;
			default :
				ptm_dcb[ptm_index].state485 = P485_IDLE;
				ptm_dcb[ptm_index].flags &= ~COMM_TROUBLE;
				PTM485NG_HistoryWrite();
				//- - - - - - - - - - - - - - - - - - - - -
				if( temp_pt < 8)	{
					diag485[temp_partition] &= ~(1 << (temp_pt));
				} else	{
					diag485[7] &= ~(1 << (temp_partition));
				}
				//- - - - - - - - - - - - - - - - - - - - -
				break;
			}
		}

		ptm_index++;
	}

}

void SendProblem485(uint8_t ptm_index, uint8_t erevent)
{
	uint8_t rxbuffer[32];

	switch(erevent)	{
	case 'E':
	case 1 :
		Mem_Copy(rxbuffer, "0000 18 E350 00 000", 19);
		break;
	case 'R' :
	case 3 :
		Mem_Copy(rxbuffer, "0000 18 R350 00 000", 19);
		break;
	}

	rxbuffer[0] = ((ptm_dcb[0].cuenta >> 12) & 0x000F) + '0';
	rxbuffer[1] = ((ptm_dcb[0].cuenta >> 8) & 0x000F) + '0';
	rxbuffer[2] = ((ptm_dcb[0].cuenta >> 4) & 0x000F) + '0';
	rxbuffer[3] = ((ptm_dcb[0].cuenta) & 0x000F) + '0';

	if( (ptm_dcb[ptm_index].rtuaddr >= 240) && (ptm_dcb[ptm_index].rtuaddr < 250) )	{
		rxbuffer[13]= '0';
		rxbuffer[14]= '0';
		eveindex = ptm_index;
		ParsePtmCID_Event(rxbuffer);
	} else {
		rxbuffer[13]=  ((IntToBCD(ptm_dcb[ptm_index].particion) >> 4) & 0x0F) + '0';
		rxbuffer[14]=  (IntToBCD(ptm_dcb[ptm_index].particion) & 0x0F) + '0';
		eveindex = ptm_index;
		ParsePtmCID_Event(rxbuffer);
	}
}


void LAN485_Send( unsigned char sendbuffer[], int bufflen )
{
	OS_ERR os_err;
	CPU_INT08U ch, rxchar[16];
	CPU_INT08U err, i;
	time_t timeout;

	GPIO_SetValue(0, 1<<21);
	OSTimeDlyHMSM(0, 0, 0, 4, OS_OPT_TIME_HMSM_STRICT, &os_err);

	UART_Send(LPC_UART2, sendbuffer, bufflen, NONE_BLOCKING);

	timeout = MSEC_TIMER + 100;
	while(UART_CheckBusy(LPC_UART2))	{
		if( MSEC_TIMER > timeout)
			break;
	}

	i = 0;
	timeout = MSEC_TIMER + 100;
	while(CommIsEmpty(COMM2) != TRUE)	{
		ch = CommGetChar(COMM2, 1, &err);
		rxchar[i++] = ch;
		if( MSEC_TIMER > timeout)
			break;
		if(i >= 16)
            break;
	}
//	OSTimeDlyHMSM(0, 0, 0, 3, OS_OPT_TIME_HMSM_STRICT, &os_err);
//	ch = CommGetChar(COMM2, 1, &err);
//	rxchar[i++] = ch;


	for( i = 0; i < bufflen-1; i++)	{
		if(sendbuffer[i] != rxchar[i])	{
			Rot485_flag |= ROT491_FLAG;
			break;
		}
	}
	if(i == bufflen-1)	{
		Rot485_flag &= ~ROT491_FLAG;
	}

	OSTimeDlyHMSM(0, 0, 0, 3, OS_OPT_TIME_HMSM_STRICT, &os_err);
	CommInit(COMM2, &err);
	GPIO_ClearValue(0, 1<<21);

    if(DebugFlag & LAN485DBG_flag) {
        CommSendString(DEBUG_COMM, "POLL: ");
        printBuffByteRow(sendbuffer, bufflen);
    }

}

void device_poll( unsigned char index )
{
	unsigned char databuffer[16];
	uint8_t releindex;


	if((ptm_dcb[index].rtuaddr >= 220) && (ptm_dcb[index].rtuaddr <= 229))	{
		databuffer[0] = 0x0A;
		databuffer[1] = ptm_dcb[index].rtuaddr;
		
		if(SystemFlag7 & RHBNETOK_FLAG)
			databuffer[2] = 'V';

		if(SystemFlag7 & RHBBUZZON_FLAG)
			databuffer[2] = 'B';
		else if(!(SystemFlag7 & RHBNETOK_FLAG))
			databuffer[2] = 'b';
		
		databuffer[3] = ptm_pwd;
		databuffer[4] = 0x0D;
		LAN485_Send( databuffer, 5 );		//tiro sobre la linea, la encuesta
        totalpakets++;
	} else
	if((ptm_dcb[index].rtuaddr >= 230) && (ptm_dcb[index].rtuaddr <= 239))	{
		releindex = ptm_dcb[index].rtuaddr - 230;
		databuffer[0] = 0x0A;
		databuffer[1] = ptm_dcb[index].rtuaddr;
		databuffer[2] = 'R';
		relestate[releindex] &= (~relepulse[releindex]);
		databuffer[3] = relestate[releindex];
		databuffer[4] = relepulse[releindex];
		databuffer[5] = 0x0D;
		LAN485_Send( databuffer, 6 );
		ptm_dcb[index].flags &= ~RELESIGNAL;
        totalpakets++;
	} else
	if( ptm_dcb[index].rtuaddr )	{		//si el dispositivo existe su direccion debe ser distinta de cero
		if(ptm_dcb[index].flags & PTMCMD_ARM)   {
            databuffer[0] = 0x0A;
            databuffer[1] = ptm_dcb[index].rtuaddr;
            databuffer[2] = 'A';
            databuffer[3] = ptm_pwd;
            databuffer[4] = 0x0D;
            LAN485_Send(databuffer, 5);
            totalpakets++;
		} else {
            databuffer[0] = 0x0A;
            databuffer[1] = ptm_dcb[index].rtuaddr;
            databuffer[2] = 'P';
            databuffer[3] = ptm_pwd;
            databuffer[4] = 0x0D;
            LAN485_Send(databuffer, 5);        //tiro sobre la linea, la encuesta
            totalpakets++;
        }
	}
}

void SendPtmACK( unsigned char index )
{
	unsigned char databuffer[16];

	if( ptm_dcb[index].rtuaddr )	{		//si el dispositivo existe su direccion debe ser distinta de cero
		databuffer[0] = 0x0A;
		databuffer[1] = ptm_dcb[index].rtuaddr;
		databuffer[2] = '@';
		databuffer[3] = ptm_pwd;
		databuffer[4] = 0x0D;
		LAN485_Send( databuffer, 5 );		//tiro sobre la linea, la encuesta
	}
}


void PTm_process_answer( int nread, unsigned char * rxbuffer, unsigned char index )
{
	uint8_t i, error, chksum, releindex, all_ptm_armed, diff, restore, ptmstatus;
	static uint8_t ptmstatus0[MAXQTYPTM], ptmstatus1[MAXQTYPTM], ptmstatus2[MAXQTYPTM];

	error = 0;
	chksum = 0;

	//CommSendString(DEBUG_COMM,"nread: ");
	//printByte2(nread);
	//CommSendString(DEBUG_COMM,"\n\r");

	switch(nread)	{
		case 7 :
            if(DebugFlag & LAN485DBG_flag) {
                CommSendString(DEBUG_COMM, "ANSW: ");
                printBuffByteRow(rxbuffer, 7);
            }
			for( i = 1; i < 5; i++ )
				chksum += rxbuffer[i];
			if( rxbuffer[5] != chksum )	{
				error = ANS_CHK_NG;
				ptm_dcb[index].com_error_counter++;
				lan485errorcurrent++;
				accumulated_errors++;
			}
			else	{
				//-----------------------------------------------------------------------------------------------------------------------
				//procesamos respuesta de estado de los PTM
                if(ptm_dcb[index].flags & PTMCMD_ARM)   {
                    ptm_dcb[index].flags &= ~PTMCMD_ARM;
                }
				switch(rxbuffer[4])	{
					case 0x5A :
                        ptmstatus0[index] = ptmstatus1[index];
                        ptmstatus1[index] = ptmstatus2[index];
                        ptmstatus2[index] = rxbuffer[3];
                        ptmstatus = ptmstatus1[index] & ptmstatus2[index];

						PTM_dev_status[index] = (uint8_t)(rxbuffer[3] | 0x80);
						//diff = ptmstatus ^ rxbuffer[3];
                        diff = ptmstatus0[index] ^ ptmstatus;
						if((diff & 0x08) && (!(rxbuffer[3] & 0x08)) && (!(BaseAlarmPkt_alarm & bitpat[TESO_bit])) ) 	{
                            if(((ptm_dcb[index].particion < 26) || (ptm_dcb[index].particion > 28)) && (ptm_dcb[index].particion != 89)) {
                                if (ptm_dcb[index].event_alarm & EVEALRM_BURG_IMD) {
                                    GenerateCIDEventPTm(index, 'R', 130, 1);
                                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                                }
                            }
						}
						if((diff & 0x02) && (!(rxbuffer[3] & 0x02)) && (!(BaseAlarmPkt_alarm & bitpat[TESO_bit])) ) 	{
							if(ptm_dcb[index].event_alarm & EVEALRM_TAMPER)	{
								GenerateCIDEventPTm(index, 'R', 145, 1);
								ptm_dcb[index].event_alarm &= ~EVEALRM_TAMPER;
							}
						}
						if(lap485_count > VALIDPTMSTATUSLAPS)    {
						    if( diff & 0x40 )    {
						        if( rxbuffer[3] & 0x40 )    {   //colocaron jumper de Z24 en el PTM
						            GenerateCIDEventPTm(index, 'E', 813, 0);
						        } else  {                       //sacaron jumper de Z24 en el PTM
						            GenerateCIDEventPTm(index, 'E', 814, 0);
						            ptmstatus0[index] &= ~0x40;
						            ptmstatus1[index] &= ~0x40;
						        }
						    }
						}

						//-----------------------------------------------
						//sistema de puertas esclusas del HSBC
						if(ptm_dcb[index].rtuaddr == LOCKGATE_RTUADDR)	{
							hsbc_lock_partition = ptm_dcb[index].particion;
                            if(rxbuffer[3] & 0x04)
                                SystemFlag8 |= LOCK2_OPEN;
                            else
                                SystemFlag8 &= ~LOCK2_OPEN;

                            if(rxbuffer[3] & 0x08)
                                SystemFlag8 |= LOCK1_OPEN;
                            else
                                SystemFlag8 &= ~LOCK1_OPEN;
						}

						//-----------------------------------------------
                        if(ptm_dcb[index].rtuaddr == SKIMMING1_RTUADDR)	{
                            if(rxbuffer[3] & 0x04) {
								SystemFlag8 |= ASKIM1_ALR1;
								logCidEvent(account, 1, 144, 55, 2);
							}
                            else {
								SystemFlag8 &= ~ASKIM1_ALR1;
								logCidEvent(account, 3, 144, 55, 2);
							}
                            if(rxbuffer[3] & 0x08) {
								SystemFlag8 |= ASKIM1_ALR2;
								logCidEvent(account, 1, 144, 55, 1);
							}
                            else {
								SystemFlag8 &= ~ASKIM1_ALR2;
								logCidEvent(account, 3, 144, 55, 1);
							}
                        }
						//-----------------------------------------------
                        if(ptm_dcb[index].rtuaddr == SKIMMING2_RTUADDR)	{
                            if(rxbuffer[3] & 0x04) {
								SystemFlag8 |= ASKIM2_ALR1;
								logCidEvent(account, 1, 144, 56, 2);
							}
                            else {
								SystemFlag8 &= ~ASKIM2_ALR1;
								logCidEvent(account, 3, 144, 56, 2);
							}
                            if(rxbuffer[3] & 0x08) {
								SystemFlag8 |= ASKIM2_ALR2;
								logCidEvent(account, 1, 144, 56, 1);
							}
                            else {
								SystemFlag8 &= ~ASKIM2_ALR2;
								logCidEvent(account, 3, 144, 56, 1);
							}
                        }
						//-----------------------------------------------
                        if(ptm_dcb[index].rtuaddr == SKIMMING3_RTUADDR)	{
                            if(rxbuffer[3] & 0x04) {
								SystemFlag8 |= ASKIM3_ALR1;
								logCidEvent(account, 1, 144, 57, 2);
							}
                            else {
								SystemFlag8 &= ~ASKIM3_ALR1;
								logCidEvent(account, 3, 144, 57, 2);
							}
                            if(rxbuffer[3] & 0x08) {
								SystemFlag8 |= ASKIM3_ALR2;
								logCidEvent(account, 1, 144, 57, 1);
							}
                            else {
								SystemFlag8 &= ~ASKIM3_ALR2;
								logCidEvent(account, 3, 144, 57, 1);
							}
                        }


						break;
					case 0x5B :
						ptm_dcb[index].version = rxbuffer[3];
						break;
					default:
						PTM_dev_status[index] &= ~0x80;
						break;
				}
				//-----------------------------------------------------------------------------------------------------------------------
				if( (rxbuffer[0] == 0x0A) && (rxbuffer[1] == 'A') && (rxbuffer[2] == ptm_dcb[index].rtuaddr) && (rxbuffer[6] == 0x0D) )	{
					error = ANS_PKT_OK;
					ptm_dcb[index].com_error_counter = 0;
					if((ptm_dcb[index].rtuaddr >= 230) && (ptm_dcb[index].rtuaddr <= 239) && (!(ptm_dcb[index].flags & RELESIGNAL)))	{
						releindex = ptm_dcb[index].rtuaddr - 230;
						relepulse[releindex] = 0x00;
					}
				} else
				if( (rxbuffer[0] == 0x0A) && (rxbuffer[1] == 'a') && (rxbuffer[2] == ptm_dcb[index].rtuaddr) && (rxbuffer[6] == 0x0D) )	{
					error = ANS_PKT_OK;
					ptm_dcb[index].com_error_counter = 0;
					//ptm_dcb[index].curr_command = 0x00;
				}
			}
			break;
		case 26 :
            if(DebugFlag & LAN485DBG_flag) {
                CommSendString(DEBUG_COMM, "ANSW: ");
                printBuffByteRow(rxbuffer, 26);
            }
			for( i = 1; i < 24; i++ )
				chksum += rxbuffer[i];
			if( rxbuffer[24] != chksum )	{
				error = ANS_CHK_NG;
				ptm_dcb[index].com_error_counter++;
                lan485errorcurrent++;
                accumulated_errors++;
				//CommSendString(DEBUG_COMM,"E1\n\r");
			}
			else {
                if(ptm_dcb[index].flags & PTMCMD_ARM)   {
                    ptm_dcb[index].flags &= ~PTMCMD_ARM;
                }
				if( (rxbuffer[0] == 0x0A) && (rxbuffer[1] == 'A') && (rxbuffer[2] == ptm_dcb[index].rtuaddr) && (rxbuffer[25] == 0x0D) )	{
					error = ANS_PKT_OK;
					ptm_dcb[index].com_error_counter = 0;
					SendPtmACK( index );
					eveindex = index;
					ParsePtmCID_Event( &(rxbuffer[4]) );
					ProcessEvents( &(rxbuffer[4]), index );
					if((ptm_dcb[index].rtuaddr >= 230) && (ptm_dcb[index].rtuaddr <= 239) && (!(ptm_dcb[index].flags & RELESIGNAL)))	{
						releindex = ptm_dcb[index].rtuaddr - 230;
						relepulse[releindex] = 0x00;
					}

					//CommSendString(DEBUG_COMM,"E2\n\r");
				} else
				if( (rxbuffer[0] == 0x0A) && (rxbuffer[1] == 'a') && (rxbuffer[2] == ptm_dcb[index].rtuaddr) && (rxbuffer[25] == 0x0D) )	{
					error = ANS_PKT_OK;
					ptm_dcb[index].com_error_counter = 0;
					//ptm_dcb[index].curr_command = 0x00;
					SendPtmACK( index );
					eveindex = index;
					ParsePtmCID_Event( &(rxbuffer[4]) );
					ProcessEvents( &(rxbuffer[4]), index );
					//CommSendString(DEBUG_COMM,"E3\n\r");
				}
			}
			break;
		case 15:
            if(DebugFlag & LAN485DBG_flag) {
                CommSendString(DEBUG_COMM, "ANSW: ");
                printBuffByteRow(rxbuffer, 15);
            }
			//CommSendString(DEBUG_COMM,"E4\n\r");
			for( i = 1; i < 13; i++ )
				chksum += rxbuffer[i];
			if( rxbuffer[13] != chksum )	{
				error = ANS_CHK_NG;
				ptm_dcb[index].com_error_counter++;
                lan485errorcurrent++;
                accumulated_errors++;
			} else	{
                if(ptm_dcb[index].flags & PTMCMD_ARM)   {
                    ptm_dcb[index].flags &= ~PTMCMD_ARM;
                }
				if( (rxbuffer[0] == 0x0A) && (rxbuffer[1] == 'z') && (rxbuffer[2] == ptm_dcb[index].rtuaddr) && (rxbuffer[14] == 0x0D) )	{
					error = ANS_PKT_OK;
					ptm_dcb[index].com_error_counter = 0;
					//ParseZoneStatus( &(rxbuffer[4]), index );
				}
			}
			break;
		default :
			//CommSendString(DEBUG_COMM,"E5\n\r");
			//printBuffByteRow( rxbuffer, nread);
            lan485errorcurrent++;
            accumulated_errors++;
			if( ptm_dcb[index].com_error_counter < 91 )
                //if(ptm_dcb[index].rtuaddr != 240)
				    ptm_dcb[index].com_error_counter++;
			break;
	}
}

void GenerateCIDEventPTm( unsigned char index, unsigned char eventtype, unsigned int eventcode, unsigned int eventzone )
{
	uint16_t checksum, tempzone, temppart;
	unsigned char i;
	uint8_t *currentEventPtr;
	OS_ERR err;

    if((ptm_dcb[index].rtuaddr == LOCKGATE_RTUADDR) && (eventtype != 145) && (eventtype != 627) && (eventtype != 628))
        return;
    if((ptm_dcb[index].rtuaddr == SKIMMING1_RTUADDR) && (eventtype != 145) && (eventtype != 627) && (eventtype != 628))
        return;
    if((ptm_dcb[index].rtuaddr == SKIMMING2_RTUADDR) && (eventtype != 145) && (eventtype != 627) && (eventtype != 628))
        return;
    if((ptm_dcb[index].rtuaddr == SKIMMING3_RTUADDR) && (eventtype != 145) && (eventtype != 627) && (eventtype != 628))
        return;

//	currentEvent.index = eventIndex++;
//	if(eventIndex == 0xFFFF)
//		eventIndex = 0x0000;

	currentEvent.timestamp = SEC_TIMER;
	currentEvent.account = IntToBCD(ptm_dcb[index].cuenta);
	switch(eventtype)	{
		case 'E' :
			currentEvent.cid_qualifier = 1;
			break;
		case 'R' :
			currentEvent.cid_qualifier = 3;
			break;
	}
	currentEvent.cid_eventcode = IntToBCD(eventcode);


	if((ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr <= 243))  {
	    if(RFDLYBOR_flag & RFDLYBOR_EVOPART_FLAG)   {
            RFDLYBOR_flag &= ~RFDLYBOR_EVOPART_FLAG;
            currentEvent.cid_partition = IntToBCD(dlyedevo_part);
	    } else currentEvent.cid_partition = IntToBCD(ptm_dcb[index].particion);
	} else  currentEvent.cid_partition = IntToBCD(ptm_dcb[index].particion);



	if(((currentEvent.cid_eventcode >= 0x400)  && (currentEvent.cid_eventcode < 0x500)) || (currentEvent.cid_eventcode == 0x121))	{
		currentEvent.cid_zoneuser = IntToBCD(eventzone);
	} else
	if((currentEvent.cid_eventcode == 0x350) || (currentEvent.cid_eventcode == 0x145))	{
		currentEvent.cid_zoneuser = 0x000;
	} else
	//---------------------------------------------------------------------------
	if((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134)|| (currentEvent.cid_eventcode == 0x110) || (currentEvent.cid_eventcode == 0x120))	{
        if((ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr <= 243))  {
            currentEvent.cid_zoneuser = IntToBCD(eventzone);
        } else {
            if((eventzone != 909) && (eventzone != 908) && (eventzone != 907) && (eventzone != 0)) {
                temppart = ptm_dcb[index].particion % 10;
                if (temppart >= 5)
                    tempzone = 0x800;
                else
                    tempzone = 0x900;
                tempzone += (ptm_dcb[index].particion / 10) * 0x10;

                if (temppart >= 5)
                    tempzone += (eventzone + ((temppart - 5) * 2));
                else
                    tempzone += (eventzone + (temppart * 2));

                if (tempzone == 0x81A)
                    tempzone = 0x810;
                if (tempzone == 0x91A)
                    tempzone = 0x910;
                if (tempzone == 0x89A)
                    tempzone = 0x900;
                currentEvent.cid_zoneuser = tempzone;
            } else
            if(eventzone == 907)    {
                currentEvent.cid_zoneuser = 0x907;
            } else
            if(eventzone == 908)    {
                currentEvent.cid_zoneuser = 0x908;
            } else
            if(eventzone == 909)    {
                currentEvent.cid_zoneuser = 0x909;
            } else
            if(eventzone == 0)    {
                currentEvent.cid_zoneuser = 0x000;
            }
        }
	} else	{
		currentEvent.cid_zoneuser = IntToBCD(eventzone);
	}
	//--------------------------------------------------------------------------

	currentEventPtr = (uint8_t *)(&currentEvent);

	for(i = 0, checksum = 0; i < 14; i++)	{
		checksum += *(currentEventPtr + i);
	}
	checksum &= 0x00FF;

	currentEvent.checksum = (uint8_t)checksum;
	currentEvent.ack_tag = 0;

#ifdef DEBUG_CID_SERIAL
	//CMI DEBUG -------------------------------------------------
	buf_len = ProtEncoder_R3KCID(&(currentEvent), sendbuffer);
	for(i = 0; i < buf_len; i++)	{
		ComPutChar(DEBUG_COMM, sendbuffer[i]);
		delay_us(100);
	}
	//-------------------------------------------------
#endif

	WriteEventToFlash(&currentEvent);
#ifdef R3K_SERIAL
	WriteEventToR3KBuffer(&currentEvent);
//	switch(currentEvent.cid_eventcode)	{
//	case 0x674:
//	case 0x675:
//		WriteEventToR3KBuffer(&currentEvent);
//		break;
//	default:
//		break;
//	}
#endif


	for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
		if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )	{
			switch(Monitoreo[i].protocol)	{
			case AP_NTSEC4:
			case AP_NTSEC5:
			case AP_NTSEC6:
			case AP_NTSEC7:
				WriteEventToTxBuffer(i, &currentEvent);
				break;
			default:
				break;
			}
		}
	}
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
}

void ParsePtmCID_Event( unsigned char event_buffer[] )
{
	unsigned char i;
	uint8_t *currentEventPtr, biselpartition, temp2[8];
	unsigned int tempzone, tempint, temppart, temp;
	uint16_t checksum, biselcount;
	uint32_t len;
	int pbtzone, retval;


//	currentEvent.index = eventIndex++;
//	if(eventIndex == 0xFFFF)
//		eventIndex = 0x0000;

	currentEvent.timestamp = SEC_TIMER;


//	currentEvent.account = 0x1000*(event_buffer[0]-'0') + 0x0100*(event_buffer[1]-'0') + 0x0010*(event_buffer[2]-'0') + (event_buffer[3]-'0');
	currentEvent.account = IntToBCD(ptm_dcb[eveindex].cuenta);

	switch(event_buffer[8])	{
		case 'E' :
			currentEvent.cid_qualifier = 1;
			break;
		case 'R' :
			currentEvent.cid_qualifier = 3;
			break;
	}
	currentEvent.cid_eventcode = (event_buffer[9] - '0')*0x100 + (event_buffer[10] - '0')*0x10 + (event_buffer[11] - '0');
    if((ptm_dcb[eveindex].particion == 79) || (ptm_dcb[eveindex].particion == 89))  {
        if(currentEvent.cid_eventcode == 0x130)
            currentEvent.cid_eventcode = 0x110;
    }
	//--------------------------------------------------------------------------------------
	// Para los TAS el E130 se filtra
	if(ptm_dcb[eveindex].disparo == 'S')	{
		if((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134))	{
			return;
		}
	}

    // No procesamos nada que venga de los PT que manejan el sistema de exclusas del HSBC
    if((ptm_dcb[eveindex].rtuaddr == LOCKGATE_RTUADDR) && (currentEvent.cid_eventcode != 0x145)&& (currentEvent.cid_eventcode != 0x627) && (currentEvent.cid_eventcode != 0x628))
        return;
    if((ptm_dcb[eveindex].rtuaddr == SKIMMING1_RTUADDR) && (currentEvent.cid_eventcode != 0x145)&& (currentEvent.cid_eventcode != 0x627) && (currentEvent.cid_eventcode != 0x628))
        return;
    if((ptm_dcb[eveindex].rtuaddr == SKIMMING2_RTUADDR) && (currentEvent.cid_eventcode != 0x145)&& (currentEvent.cid_eventcode != 0x627) && (currentEvent.cid_eventcode != 0x628))
        return;
    if((ptm_dcb[eveindex].rtuaddr == SKIMMING3_RTUADDR) && (currentEvent.cid_eventcode != 0x145)&& (currentEvent.cid_eventcode != 0x627) && (currentEvent.cid_eventcode != 0x628))
        return;
	//--------------------------------------------------------------------------------------

	if( (ptm_dcb[eveindex].rtuaddr >= 240) && (ptm_dcb[eveindex].rtuaddr <= 243) )	{
		if(SystemFlag1 & BISEL_FLAG)	{
			biselcount = (event_buffer[0] -'0');
			biselpartition = (event_buffer[14] - '0');
			if(biselpartition != 0)	{
				event_buffer[14] = event_buffer[0];
			}
		}
        tempint = BCDToInt((unsigned int)((event_buffer[13] - '0')*10) + (event_buffer[14] - '0'));
        temp = (ptm_dcb[eveindex].particion >> 4) * 16;
        temp += (ptm_dcb[eveindex].particion & 0x0F);
        temppart = tempint + temp;
        tempint = IntToBCD(temppart);
        currentEvent.cid_partition = tempint;
//		tempint = BCDToInt((unsigned int)((event_buffer[13] - '0')*10) + (event_buffer[14] - '0'));
//		temp = (ptm_dcb[eveindex].particion >> 4) * 16;
//		temp += (ptm_dcb[eveindex].particion & 0x0F);
//        temppart = (event_buffer[13] - '0')*0x10;
//        temppart += (event_buffer[14] - '0');
//		currentEvent.cid_partition = temppart;       //tempint

	} else {
		//currentEvent.cid_partition = ((event_buffer[13] - '0') * 0x10) + (event_buffer[14] - '0');
		currentEvent.cid_partition = IntToBCD(ptm_dcb[eveindex].particion);
	}


	//-----------------------------------------------------------------------------
	// aca intercepto los PBT
	pbtzone = ((event_buffer[16] - '0') * 0x100) + ((event_buffer[17] - '0') * 0x10) + (event_buffer[18] - '0');
	retval = PBTinTable(ptm_dcb[eveindex].particion, pbtzone);
	if(retval != -1)	{

		logCidEvent(pbt_dcb[retval].cuenta, 1, REMOTEASALT_TRIGGER, 0, 0);
		return;
	}

	//-----------------------------------------------------------------------------


	if( (ptm_dcb[eveindex].rtuaddr >= 240) && (ptm_dcb[eveindex].rtuaddr < 250) )	{
		if(SystemFlag1 & BISEL_FLAG)	{
			if(!(((currentEvent.cid_eventcode >= 0x400)  && (currentEvent.cid_eventcode < 0x500)) || (currentEvent.cid_eventcode == 0x121)))	{
				if( (biselcount != biselpartition) && (biselpartition != 0))	{
					event_buffer[16] += 2;		//va a sumarle 200 al numero de zona
				}
			}
		}

		currentEvent.cid_zoneuser = ((event_buffer[16] - '0') * 0x100) + ((event_buffer[17] - '0') * 0x10) + (event_buffer[18] - '0');
	} else {
		if(((currentEvent.cid_eventcode >= 0x400)  && (currentEvent.cid_eventcode < 0x500)) || (currentEvent.cid_eventcode == 0x121))	{
			currentEvent.cid_zoneuser = ((event_buffer[16] - '0') * 0x100) + ((event_buffer[17] - '0') * 0x10) + (event_buffer[18] - '0');
		} else
		if((currentEvent.cid_eventcode == 0x350) || (currentEvent.cid_eventcode == 0x145))	{
			currentEvent.cid_zoneuser = 0x000;
		} else
		if (currentEvent.cid_eventcode == 0x776)  {
            ptm_dcb[eveindex].RFALRMDLY_flag |= PTMTERMICOTRIGG;
            ptm_dcb[eveindex].event_alarm |= EVEALRM_BURG_TERMIC;
            return;
		}  else
		if((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134)|| (currentEvent.cid_eventcode == 0x110) || (currentEvent.cid_eventcode == 0x120))	{
			temppart = ptm_dcb[eveindex].particion % 10;
			if(temppart >= 5)
				tempzone = 0x800;
			else
				tempzone = 0x900;
			tempzone += (ptm_dcb[eveindex].particion /10) * 0x10;

			if((event_buffer[18] - '0') != 0) {

                if (temppart >= 5)
                    tempzone += (((event_buffer[18] - '0') & 0x0F) + ((temppart - 5) * 2));
                else
                    tempzone += (((event_buffer[18] - '0') & 0x0F) + (temppart * 2));

                if (tempzone == 0x81A)
                    tempzone = 0x810;
                if (tempzone == 0x91A)
                    tempzone = 0x910;
                if (tempzone == 0x89A)
                    tempzone = 0x900;
                currentEvent.cid_zoneuser = tempzone;

                if((ptm_dcb[eveindex].particion >= 26) && (ptm_dcb[eveindex].particion <= 28) && (currentEvent.cid_eventcode == 0x130))    {
                    currentEvent.cid_eventcode = 0x120;
                }


                if (ptm_dcb[eveindex].RFALRMDLY_flag & PTMTERMICOTRIGG) {
                    ptm_dcb[eveindex].RFALRMDLY_flag &= ~PTMTERMICOTRIGG;
                    if(((currentEvent.cid_partition >= 0x11) && (currentEvent.cid_partition <= 0x19)) ||
                        ((currentEvent.cid_partition >= 0x41) && (currentEvent.cid_partition <= 0x49))) {
                        currentEvent.cid_zoneuser = 0x909;
                    } else
                    if(((currentEvent.cid_partition >= 0x21) && (currentEvent.cid_partition <= 0x25)) ||
                       ((currentEvent.cid_partition >= 0x31) && (currentEvent.cid_partition <= 0x39)))    {
                        currentEvent.cid_zoneuser = 0x908;
                    } else
                    if((currentEvent.cid_partition >= 0x50) && (currentEvent.cid_partition <= 0x54))    {
                        currentEvent.cid_zoneuser = 0x907;
                    }

                }
                ptm_dcb[eveindex].RFALRMDLY_flag &= ~PTMTERMICOTRIGG;
            } else  {
                currentEvent.cid_zoneuser = 0x000;
			}
		} else	{
			currentEvent.cid_zoneuser = ((event_buffer[16] - '0') * 0x100) + ((event_buffer[17] - '0') * 0x10) + (event_buffer[18] - '0');
		}
	}

	currentEventPtr = (uint8_t *)(&currentEvent);

	for(i = 0, checksum = 0; i < 14; i++)	{
		checksum += *(currentEventPtr + i);
	}
	checksum &= 0x00FF;

	currentEvent.checksum = (uint8_t)checksum;

	currentEvent.ack_tag = 0x00;

#ifdef DEBUG_CID_SERIAL
	//CMI DEBUG -------------------------------------------------
	buf_len = ProtEncoder_R3KCID(&(currentEvent), sendbuffer);
	for(i = 0; i < buf_len; i++)	{
		ComPutChar(DEBUG_COMM, sendbuffer[i]);
		delay_us(100);
	}
	//-------------------------------------------------
#endif


	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// replicacion del estado de apertura cierre de paticiones evo sobre pt clasico
	if((currentEvent.cid_eventcode == 0x403) || (currentEvent.cid_eventcode == 0x408) || (currentEvent.cid_eventcode == 0x401))	{
		if(currentEvent.cid_qualifier == 1)	{
			for(i = 0; i < 5; i++)	{
				if((PT_estado_particion[i] == currentEvent.cid_partition) && (PT_estado_particion[i] != 0x00))	{
					BaseAlarmPkt_estado_dispositivos &= ~(1 << (i+3));
					AlarmWriteHistory();
				}
			}
		} else
		if(currentEvent.cid_qualifier == 3)	{
			for(i = 0; i < 5; i++)	{
				if((PT_estado_particion[i] == currentEvent.cid_partition) && (PT_estado_particion[i] != 0x00))	{
					BaseAlarmPkt_estado_dispositivos |= (1 << (i+3));
					AlarmWriteHistory();
				}
			}
		}
	}
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    switch(ptm_dcb[eveindex].rtuaddr)	{
        case 240:
            EVOWD_Flag |= (1 << 0);
            break;
        case 241:
            EVOWD_Flag |= (1 << 1);
            break;
        case 242:
            EVOWD_Flag |= (1 << 2);
            break;
        default:
            break;
    }
	if(currentEvent.cid_eventcode == wdevo_event)	{

		return;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// intercepto el 401 que viene de los pt buzzer
	if( (ptm_dcb[eveindex].rtuaddr >= 220) && (ptm_dcb[eveindex].rtuaddr < 230) )	{
		if(currentEvent.cid_eventcode == 0x401)
			currentEvent.cid_eventcode = 0x801;
	}

	//intercepto el pulso de watch dog de vida del IP150
	if((currentEvent.cid_zoneuser == 0x004) && (currentEvent.cid_partition >= 0x070) && (currentEvent.cid_partition  <= 0x088))	{		//zona 4 de la EVO
		if((currentEvent.cid_eventcode >= 0x100) && (currentEvent.cid_eventcode < 0x200) && (currentEvent.cid_qualifier == 1)) {
			SystemFlag7 |= IP150_KICK;
			return;
		}
	}
    if((currentEvent.cid_zoneuser == 0x004) && (currentEvent.cid_partition >= 0x070) && (currentEvent.cid_partition  <= 0x088))	{		//zona 4 de la EVO
        if((currentEvent.cid_eventcode >= 0x100) && (currentEvent.cid_eventcode < 0x200) && (currentEvent.cid_qualifier == 3)) {
            return;
        }
    }

	//intercepto cualquier evento que llege de a zona 187,
	//y lo convierto en el evento 979 para indicar el estado
	//de la llave mecanica del electroiman de puerta
		if(currentEvent.cid_zoneuser == 0x187)	{
			currentEvent.cid_eventcode = 0x979;
			currentEvent.cid_zoneuser = 0x000;
			if(currentEvent.cid_qualifier == 1)	{
				STRIKE_Flag |= STRKLLAVE_STATE;
				temp2[0] = 0x5A;
				len = flash0_write(1, temp2, SLLAVE_STATE, 1);
			} else
			if(currentEvent.cid_qualifier == 3)	{
				STRIKE_Flag &= ~STRKLLAVE_STATE;
				temp2[0] = 0xA5;
				len = flash0_write(1, temp2, SLLAVE_STATE, 1);
			}
		}

		if(currentEvent.cid_zoneuser == 0x186)	{
			return;
		}

		if(currentEvent.cid_zoneuser == 0x188)	{
			if(currentEvent.cid_qualifier == 3)	{
				SIRENA_Flag |= DOOR1_OC_FLAG;
				temp2[0] = 0x5A;
				len = flash0_write(1, temp2, DOOR1_STATE, 1);
			} else
			if(currentEvent.cid_qualifier == 1)	{
				SIRENA_Flag &= ~DOOR1_OC_FLAG;
				temp2[0] = 0xA5;
				len = flash0_write(1, temp2, DOOR1_STATE, 1);
			}
			return;
		}


    //A peticion de Christian Marino, filtro los eventos de tamper de la particion 58
    if((currentEvent.cid_partition == 0x58) && (currentEvent.cid_eventcode == 0x145)) {
        return;
    }
	if((currentEvent.cid_partition == 0x66) && (currentEvent.cid_eventcode == 0x401)) {
        return;
    }
	if((currentEvent.cid_partition == 0x66) && (currentEvent.cid_eventcode == 0x403)) {
        return;
    }
	if((currentEvent.cid_partition == 0x66) && (currentEvent.cid_eventcode == 0x408)) {
        return;
    }
	if((currentEvent.cid_partition == 0x66) && (currentEvent.cid_eventcode == 0x145)) {
        return;
    }



	if((currentEvent.cid_partition == 0x70) && (currentEvent.cid_eventcode == 0x354)  && (currentEvent.cid_zoneuser == 0x000)) {
        return;
    }

    if((ptm_dcb[eveindex].particion >= 10) && (ptm_dcb[eveindex].particion < 55))   {
        tempzone = (event_buffer[18] - '0');
        if(((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134)) && (tempzone == 2))
            return;
    }
    if((currentEvent.cid_partition >= 0x71) && (currentEvent.cid_partition <= 0x78))   {
        if(((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134)) && (currentEvent.cid_qualifier == 1))
            if((currentEvent.cid_zoneuser == 0x189) || (currentEvent.cid_zoneuser == 0x190) || (currentEvent.cid_zoneuser == 0x191) || (currentEvent.cid_zoneuser == 0x192))
                return;
    }
    if((currentEvent.cid_partition >= 0x81) && (currentEvent.cid_partition <= 0x88))   {
        if(((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134)) && (currentEvent.cid_qualifier == 1))
            if((currentEvent.cid_zoneuser == 0x189) || (currentEvent.cid_zoneuser == 0x190) || (currentEvent.cid_zoneuser == 0x191) || (currentEvent.cid_zoneuser == 0x192))
                return;
    }
    if((currentEvent.cid_partition >= 0x91) && (currentEvent.cid_partition <= 0x98))   {
        if(((currentEvent.cid_eventcode == 0x130) || (currentEvent.cid_eventcode == 0x134)) && (currentEvent.cid_qualifier == 1))
            if((currentEvent.cid_zoneuser == 0x189) || (currentEvent.cid_zoneuser == 0x190) || (currentEvent.cid_zoneuser == 0x191) || (currentEvent.cid_zoneuser == 0x192))
                return;
    }

    if(currentEvent.cid_partition == 0x55)  {
        if((currentEvent.cid_eventcode >= 0x100) && (currentEvent.cid_eventcode <= 0x199))  {
            currentEvent.cid_eventcode = 0x931;
        } else
        if((currentEvent.cid_eventcode == 0x401) || (currentEvent.cid_eventcode == 0x408) || (currentEvent.cid_eventcode == 0x403))  {
            currentEvent.cid_eventcode = 0x801;
        }
    }

	WriteEventToFlash(&currentEvent);

	//filtro las señalizacion de uso interno en zonas 189 a 192
	if((currentEvent.cid_zoneuser >= 0x189) && (currentEvent.cid_zoneuser <= 0x192))	{
		return;
	}

	//filtro los eventos de zonas 5 a 8, de las evo, para los eventos entre 100 y 199
	if((currentEvent.cid_zoneuser >= ASALPBT_FIRSTZONE) && (currentEvent.cid_zoneuser <= ASALPBT_LASTZONE))	{
		if((currentEvent.cid_eventcode >= 0x100) && (currentEvent.cid_eventcode <= 0x199))	{
			if((currentEvent.cid_partition >= 0x70) && (currentEvent.cid_partition <= 0x88)) {
				return;
			}
		}
	}

	//señalizo los eventos asociados a volumetricas 05 y 71
	if((currentEvent.cid_eventcode == 0x401) || (currentEvent.cid_eventcode == 0x407) || (currentEvent.cid_eventcode == 0x408) || (currentEvent.cid_eventcode == 0x409))  {
	    if(currentEvent.cid_qualifier == 1) {   //eventos E
			if(currentEvent.cid_partition == 71) {
				recharge25min_alarm(ASAL_bit, 1);
			}
	        if(currentEvent.cid_partition == VolumetricRedundance[0])  {
                SystemFlag9 |= P5_E401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 1);
	        } else
			if(currentEvent.cid_partition == VolumetricRedundance[1])  {
				SystemFlag9 |= P6_E401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 1);
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[2])  {
				SystemFlag9 |= P7_E401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 1);
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[3])  {
				SystemFlag9 |= P8_E401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 1);
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[4])  {
				SystemFlag9 |= P9_E401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 1);
			}
            if(currentEvent.cid_partition == 0x05)  {
                SystemFlag9 |= P5_E401_05_RCVD;
				recharge25min_alarm(ASAL_bit, 1);
            } else
			if(currentEvent.cid_partition == 0x06)  {
				SystemFlag9 |= P6_E401_05_RCVD;
			} else
			if(currentEvent.cid_partition == 0x07)  {
				SystemFlag9 |= P7_E401_05_RCVD;
			} else
			if(currentEvent.cid_partition == 0x08)  {
				SystemFlag9 |= P8_E401_05_RCVD;
			} else
			if(currentEvent.cid_partition == 0x09)  {
				SystemFlag9 |= P9_E401_05_RCVD;
			}
	    } else  {                               //eventos R
			if(currentEvent.cid_partition == 71) {
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			}
			if(currentEvent.cid_partition == VolumetricRedundance[0])  {
				SystemFlag9 |= P5_R401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[1])  {
				SystemFlag9 |= P6_R401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[2])  {
				SystemFlag9 |= P7_R401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[3])  {
				SystemFlag9 |= P8_R401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			} else
			if(currentEvent.cid_partition == VolumetricRedundance[4])  {
				SystemFlag9 |= P9_R401_71_RCVD;
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			}
			if(currentEvent.cid_partition == 0x05)  {
				SystemFlag9 |= P5_R401_05_RCVD;
				recharge25min_alarm(ASAL_bit, 3);
				last_voluclose = SEC_TIMER;
			} else
			if(currentEvent.cid_partition == 0x06)  {
				SystemFlag9 |= P6_R401_05_RCVD;
			} else
			if(currentEvent.cid_partition == 0x07)  {
				SystemFlag9 |= P7_R401_05_RCVD;
			} else
			if(currentEvent.cid_partition == 0x08)  {
				SystemFlag9 |= P8_R401_05_RCVD;
			} else
			if(currentEvent.cid_partition == 0x09)  {
				SystemFlag9 |= P9_R401_05_RCVD;
			}
	    }
	}

#ifdef R3K_SERIAL
	WriteEventToR3KBuffer(&currentEvent);

#endif



	for( i = 0; i < CENTRALOFFICEMAX; i++ )	{
		if((Monitoreo[i].inuse == TRUE) && (!(SystemFlag11 & DONTSENDEVENTS)) )	{
			switch(Monitoreo[i].protocol)	{
			case AP_NTSEC4:
			case AP_NTSEC5:
			case AP_NTSEC6:
			case AP_NTSEC7:
				//while( SystemFlag2 & MONBUFFER_BUSY);
				WriteEventToTxBuffer(i, &currentEvent);
				break;
			default:
				break;
			}
		}
	}
}

int rfdly_time;

void init_lan_cfgfile( void )
{
	int error, i, tblindex;
	uint8_t buffer[64];
	uint32_t address, chksum;
	OS_ERR os_err;

	for( i = 0; i < MAXQTYPTM; i++ )	{
		ptm_dcb[i].rtuaddr = 0x00;
		ptm_dcb[i].com_error_counter = 0;
		ptm_dcb[i].particion = 0x00;
		ptm_dcb[i].cuenta = 0x0000;
		ptm_dcb[i].disparo = 0x00;
		//ptm_dcb[i].state485 = P485_IDLE;
		ptm_dcb[i].normstate = PTNORM_IDLE;
		PTM_dev_status[i] = 0x00;
		ptm_dcb[i].flags = 0x00;
		ptm_dcb[i].RFALRMDLY_flag = 0x00;
        ptm_dcb[i].rfalrmdly_state = PTRFDLY_ST_IDLE;
	}

	for( i = 0; i < MAXQTYPTM; i++ )	{
		address = PTM00DCB_E2P_ADDR + i*5;
		EepromReadBuffer(address, buffer, 5, &error);
		ptm_dcb[i].rtuaddr = buffer[0];
		ptm_dcb[i].particion = buffer[1];
		ptm_dcb[i].cuenta = buffer[2]*0x100 + buffer[3];
		ptm_dcb[i].disparo = buffer[4];
	}

	PTM485NG_HistoryRead();

	//inicializamos la tabla de pulsadores
	for( i = 0; i < MAXPBTPTM; i++ )	{
		pbt_dcb[i].particion = 0;
		pbt_dcb[i].zona = 0;
		pbt_dcb[i].cuenta = 0;
	}
	flash0_read(buffer, DF_PBTFIDU, 4);
	if( (buffer[0] != 0x55) || (buffer[1] != 0x5A) || (buffer[2] != 0xA5) || (buffer[3] != 0xAA) )	{
		for( tblindex = 0; tblindex < MAXPBTPTM; tblindex++ )	{
			buffer[0] = 0;
			buffer[1] = 0;
			buffer[2] = 0;
			buffer[3] = 0;
			buffer[4] = 0;

			chksum = 0;
			for(i = 0; i < 5; i++)
				chksum += buffer[i];
			buffer[5] = (uint8_t)(chksum & 0x00FF);

			address = DF_PBTTBL0 + tblindex*DF_PBTREGLEN;
			flash0_write(1, buffer, address, 6 );
			OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		}
		//le pongo la marca fiduciaria
        	buffer[0] = 0x55;
        	buffer[1] = 0x5A;
        	buffer[2] = 0xA5;
        	buffer[3] = 0xAA;
        	flash0_write(1, buffer, DF_PBTFIDU, 4);
        	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	} else	{
		for( tblindex = 0; tblindex < MAXPBTPTM; tblindex++ )	{
			address = DF_PBTTBL0 + tblindex*DF_PBTREGLEN;
        	flash0_read(buffer, address, DF_PBTREGLEN);

        	pbt_dcb[tblindex].particion = buffer[0];
        	pbt_dcb[tblindex].zona = buffer[1];
        	pbt_dcb[tblindex].zona <<= 8;
        	pbt_dcb[tblindex].zona |= (buffer[2] & 0x00FF);
        	pbt_dcb[tblindex].cuenta = buffer[3];
        	pbt_dcb[tblindex].cuenta <<= 8;
        	pbt_dcb[tblindex].cuenta |= (buffer[4] & 0x00FF);

		}
	}

}

void ptm_group_replica(void)
{
	int status_11_19, status_21_29, status_31_39, status_41_49, status_volumetricas;
	int i, j, k, l;
	int particion, part_group, part_uni;

	status_11_19 = 0x00;
	status_21_29 = 0x00;
	status_31_39 = 0x00;
	status_41_49 = 0x00;
	status_volumetricas = 0x00;

	for( i = 0; i < MAXQTYPTM; i++ )	{
		if( ptm_dcb[i].rtuaddr == 0x00)
			continue;
		//-----------------------------------------------------------------
		//aca replicacion de estados de ptm en borneras delanteras por RF
		if(PTM_dev_status[i] & 0x80)	{
			if( (ptm_dcb[i].particion >= 11) && (ptm_dcb[i].particion <= 19) )	{
				if(PTM_dev_status[i] & 0x01)	{
					status_11_19 |= 0x01;
				}
			}
			if( (ptm_dcb[i].particion >= 21) && (ptm_dcb[i].particion <= 29) )	{
				if(PTM_dev_status[i] & 0x01)	{
					status_21_29 |= 0x01;
				}	
			}
			if( (ptm_dcb[i].particion >= 31) && (ptm_dcb[i].particion <= 39) )	{
				if(PTM_dev_status[i] & 0x01)	{
					status_31_39 |= 0x01;
				}	
			}
			if( (ptm_dcb[i].particion >= 41) && (ptm_dcb[i].particion <= 49) )	{
				if(PTM_dev_status[i] & 0x01)	{
					status_41_49 |= 0x01;
				}	
			}
		}
		//-----------------------------------------------------------------
	}
	
	// replico en la bornera delantera
	if( PT_estado_particion[1] == 0x99 )	{
		if( status_21_29 & 0x01 )	{
			BaseAlarmPkt_estado_dispositivos &= ~0x10;
		} else	{
			BaseAlarmPkt_estado_dispositivos |= 0x10;
		}
	}
	if( PT_estado_particion[2] == 0x99 )	{
		if( status_31_39 & 0x01 )	{
			BaseAlarmPkt_estado_dispositivos &= ~0x20;
		} else	{
			BaseAlarmPkt_estado_dispositivos |= 0x20;
		}
	}
	if( PT_estado_particion[3] == 0x99 )	{
		if( status_41_49 & 0x01 )	{
			BaseAlarmPkt_estado_dispositivos &= ~0x40;
		} else	{
			BaseAlarmPkt_estado_dispositivos |= 0x40;
		}
	}
	if( PT_estado_particion[4] == 0x99 )	{
		if( status_11_19 & 0x01 )	{
			BaseAlarmPkt_estado_dispositivos &= ~0x80;
		} else	{
			BaseAlarmPkt_estado_dispositivos |= 0x80;
		}
	}

	//aca replicamos el grupo de la volumetricas
	for(i = 1; i < 10; i++)	{
		for(j = 0; j < 8; j++)	{
			if( PartDec_group[i] & (1 << j) )	{
				particion = i*10 + j + 1;
				part_group = i; part_uni = j;

				//primero busco en los ptm
				for( k = 0; k < MAXQTYPTM; k++ )	{
					if( ptm_dcb[k].rtuaddr == 0x00)
						continue;
					if( ptm_dcb[k].particion == particion )	{
						if(PTM_dev_status[k] & 0x01)	{
							status_volumetricas |= 0x01;
						}
					}
				}
				//ahora busco en las evo
				if( (particion > 70) && (particion < 79) )	{
					if( PDX_dev_status[0] & (1 << part_uni) )	{
						status_volumetricas |= 0x01;
					}
				} else
				if( (particion > 80) && (particion < 89) )	{
					if( PDX_dev_status[1] & (1 << part_uni) )	{
						status_volumetricas |= 0x01;
					}
				} else
				if( (particion > 90) && (particion < 99) )	{
					if( PDX_dev_status[2] & (1 << part_uni) )	{
						status_volumetricas |= 0x01;
					}
				}
			}
		}
	}

	if( PT_estado_particion[0] == 0x99 	)	{
		if( status_volumetricas & 0x01 )	{
			BaseAlarmPkt_estado_dispositivos &= ~0x08;
		} else	{
			BaseAlarmPkt_estado_dispositivos |= 0x08;
		}
	}

	if( status_volumetricas & 0x01 )	{
		SystemFlag7 |= RHBVOLU_FLAG;
	} else	{
		SystemFlag7 &= ~RHBVOLU_FLAG;
	}
}

void openptm_process( void )
{
	int i, statusptm;

	if( TasFlags & TASMODE_FLAG )
		return;
	if( !(SystemFlag5 & OPENPTM) )
		return;

	statusptm = 0x00;


	for( i = 0; i < MAXQTYPTM; i++ )	{
		if( ptm_dcb[i].rtuaddr == 0x00)
			continue;
		//-----------------------------------------------------------------
		//aca proceso sobre el rele de pgm
		if( (PTM_dev_status[i] & 0x01) && (PTM_dev_status[i] & 0x80) )	{
			statusptm |= 0x01;
		}

	}

	// replico en el pgm
	if( statusptm & 0x01 )	{
		GPIO_SetValue(0, 1<<27);
	} else	{
		GPIO_ClearValue(0, 1<<27);
	}	

}

void ProcessEvents( unsigned char event_buffer[], unsigned char index )
{

	unsigned int eventcode, eventpartition, eventzone, temp, subpartition;
	OS_ERR os_err;
	int retval, i;


	eventcode = (event_buffer[9] - '0')*100 + (event_buffer[10] - '0')*10 +  (event_buffer[11] - '0');
	eventpartition = (event_buffer[13] - '0')*10 + (event_buffer[14] - '0');
	eventzone = (event_buffer[16] - '0')*100 + (event_buffer[17] - '0')*10 + (event_buffer[18] - '0');

	subpartition = eventpartition % 10;

	//-----------------------------------------------------------------------------
	// aca intercepto los PBT
	retval = PBTinTable(ptm_dcb[index].particion, eventzone);
	if(retval != -1)	{
		return;
	}

    // No procesamos nada que venga de los PT que manejan el sistema de exclusas del HSBC
    if((ptm_dcb[index].rtuaddr == LOCKGATE_RTUADDR) && (eventcode != 0x145) && (eventcode != 0x627) && (eventcode != 0x628))
        return;
    if((ptm_dcb[index].rtuaddr == SKIMMING1_RTUADDR) && (eventcode != 0x145) && (eventcode != 0x627) && (eventcode != 0x628))
        return;
    if((ptm_dcb[index].rtuaddr == SKIMMING2_RTUADDR) && (eventcode != 0x145) && (eventcode != 0x627) && (eventcode != 0x628))
        return;
    if((ptm_dcb[index].rtuaddr == SKIMMING3_RTUADDR) && (eventcode != 0x145) && (eventcode != 0x627) && (eventcode != 0x628))
        return;

	if(event_buffer[8] == 'E')	{
		switch(eventcode)	{
			case 403:
			case 408:
			case 401:
                if( (ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr < 243) )	{
                    temp = ptm_dcb[index].rtuaddr - 240;
                    PDX_dev_status[temp] |= (1 << (subpartition - 1));
                    guardar_PDX_status();
                    ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_DISARM_FLAG;
                } else {
                    if (eventzone == 16) {
                        ptm_dcb[index].event_alarm |= EVEALRM_TECHMAN9;
                        TECHMAN9_timer = SEC_TIMER;
                    } else
                    if ((ptm_dcb[index].particion == CASTILLETEDVR_PARTITION) && (eventcode == 401)) {
                        SystemFlag7 &= ~CASTDVR_ACK;
                    } else
                    if (eventcode == 401) {
                        if ((eventpartition >= 10) && (eventpartition <= 99)) {
                            ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_DISARM_FLAG;
                        }
                    }
                }
				break;
			case 988:
				if( ptm_dcb[index].particion == CASTILLETEDVR_PARTITION )	{
					SystemFlag7 &= ~CASTDVR_ACK;
				}
				break;
			case 354:
				if(eventzone == 0)	{		//es la zona 0 para comunicador IP150
					SystemFlag7 &= ~IP150_ALIVE;
				}
				break;
			case 627:
				ptm_dcb[index].event_alarm |= EVEALRM_TECHMAN;
				TECHMAN_timer = SEC_TIMER;
				break;
            case 110:
                if(ptm_dcb[index].particion == 89)	{
                    SysFlag_AP_GenAlarm |= bitpat[INCE_bit];
                    recharge_alarm(INCE_bit);
                    ASAL_timer = SEC_TIMER;
                    //dlyedptm_zone[index] = eventzone;
                    //ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_ASAL_FLAG;

                    if(eventzone == 1)	{
                        if(!(ptm_dcb[index].event_alarm & EVEALRM_BURG_TERMIC)) {
                            ptm_dcb[index].event_alarm |= EVEALRM_BURG_IMD;
                            ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                            //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                        }
                    } else
                    if(eventzone == 2)	{
                        ptm_dcb[index].event_alarm |= EVEALRM_BURG_TMP;
                        ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                        //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                    }
                }
                break;
		    case 120:
                if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))	{
                    SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
                    recharge_alarm(ASAL_bit);
                    ASAL_timer = SEC_TIMER;
                    //dlyedptm_zone[index] = eventzone;
                    //ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_ASAL_FLAG;

                    if(eventzone == 1)	{
                        if(!(ptm_dcb[index].event_alarm & EVEALRM_BURG_TERMIC)) {
                            ptm_dcb[index].event_alarm |= EVEALRM_BURG_IMD;
                            ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                            //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                        }
                    } else
                    if(eventzone == 2)	{
                        ptm_dcb[index].event_alarm |= EVEALRM_BURG_TMP;
                        ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                        //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                    }
                }
		        break;
			case 121:		// Panic
				OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
				if((ptm_dcb[index].particion >= 11) && (ptm_dcb[index].particion < 55))	{
                    if((ptm_dcb[index].particion != 27) && (ptm_dcb[index].particion != 28) && (ptm_dcb[index].particion != 29) \
                        && (ptm_dcb[index].particion != 47) && (ptm_dcb[index].particion != 48) && (ptm_dcb[index].particion != 49)) {
                        ptm_dcb[index].RFALRMDLY_flag |= PTMSIG_OLDPANIC;
                        //SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
						//recharge_alarm(TESO_bit);
                        //TESO_timer = SEC_TIMER;
                    }
				} else
				if((ptm_dcb[index].particion >= 60) && (ptm_dcb[index].particion <= 62))	{
                    ptm_dcb[index].RFALRMDLY_flag |= PTMSIG_OLDPANIC;
					//SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
					//recharge_alarm(TESO_bit);
					//ASAL_timer = SEC_TIMER;
				} else
                /**if((ptm_dcb[index].particion >= 62) && (ptm_dcb[index].particion <= 65))	{
                    SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
                    recharge_alarm(ASAL_bit);
                    ASAL_timer = SEC_TIMER;
                } else **/
				if((ptm_dcb[index].particion == 79) || (ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 89))	{
					SysFlag_AP_GenAlarm |= bitpat[INCE_bit];
					recharge_alarm(INCE_bit);
					INCE_timer = SEC_TIMER;
				}
				switch(ptm_dcb[index].disparo)	{
                    case 'S':
                        TasFlags |= TASVANDAL_FLAG;
                        return;
                        break;
                    default:
                        break;
				}
				ptm_dcb[index].event_alarm |= EVEALRM_PANIC;
				break;
	        case 128:
                PTMSIGNAL_flag |= PTMSIG_PANIC;
                OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
                ptm_dcb[index].event_alarm |= EVEALRM_PANIC128;
                RFDLYBOR_flag |= RFDLYBOR_TDONEP_FLAG;
	            break;
		    case 776:
                OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
                if((ptm_dcb[index].particion >= 10) && (ptm_dcb[index].particion < 55)) {
                    if(!(((currentEvent.cid_partition >= 0x21) && (currentEvent.cid_partition <= 0x25)) ||
                       ((currentEvent.cid_partition >= 0x31) && (currentEvent.cid_partition <= 0x39)) ||
                       ((currentEvent.cid_partition >= 0x50) && (currentEvent.cid_partition <= 0x54))))    {
                        SysFlag1 |= SF220_flag;
                        SystemFlag3 |= NAPER_flag;
                        SystemFlag3 |= NAPER_F220V;
                        SystemFlag10 |= F220INDICATION1P;
                    }
                }
                break;
		    case 134:
			case 130:		// Burglary
				OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
				if((ptm_dcb[index].particion == 20) || (ptm_dcb[index].particion == 30) || (ptm_dcb[index].particion == 40) || (ptm_dcb[index].particion == 55))	{
					ptm_dcb[index].flags |= AUTONORMAL;
				} else
				if((ptm_dcb[index].particion == 68) || (ptm_dcb[index].particion == 69))	{
					GenerateCIDEventPTm(index, 'E', 771, eventzone);
					break;
				} else
				if( (ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr < 243) )	{
				    eventpartition += ptm_dcb[index].particion;
					if(eventzone == 189)	{
						SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
						ROTU_timer = SEC_TIMER;
					} else
					if(eventzone == 190)	{
						SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
						ASAL_timer = SEC_TIMER;
						recharge_alarm(ASAL_bit);
					} else
					if(eventzone == 191)	{
						SysFlag_AP_GenAlarm |= bitpat[INCE_bit];
						INCE_timer = SEC_TIMER;
						recharge_alarm(INCE_bit);
					} else
					if(eventzone == 192)	{
						//SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
						//TESO_timer = SEC_TIMER;
						//recharge_alarm(TESO_bit);

                        RFDLYBOR_flag |= RFDLYBOR_TESO_FLAG;
					} else if((eventzone >= FIRSTPBTZONE) && (eventzone <= LASTPBTZONE))	{
						retval = eventzone - FIRSTPBTZONE;
                        if((pbt_dcb[retval].cuenta > 0) && (pbt_dcb[retval].cuenta <= 9999))    {
                            logCidEvent(pbt_dcb[retval].cuenta, 1, REMOTEASALT_TRIGGER, 0, 0);
                        }
					} else

                    if(((eventpartition >= 71) && (eventpartition <= 78)) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188) && (eventzone != 4))	{
                        if(eventzone == 192) {
                            dlyedptm_zone[index] = eventzone;
                            dlyedevo_part = eventpartition;
                            RFDLYBOR_flag |= RFDLYBOR_EVOPART_FLAG;
                            ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                        }
                    } else
                    if((eventpartition >= 81) && (eventpartition <= 88) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188) && (eventzone != 4))	{
                        if(eventzone == 192) {
                            dlyedptm_zone[index] = eventzone;
                            dlyedevo_part = eventpartition;
                            RFDLYBOR_flag |= RFDLYBOR_EVOPART_FLAG;
                            ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                        }
                    } else
                    if((eventpartition >= 91) && (eventpartition <= 98) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188) && (eventzone != 4))	{
                        if(eventzone == 192) {
                            dlyedptm_zone[index] = eventzone;
                            dlyedevo_part = eventpartition;
                            RFDLYBOR_flag |= RFDLYBOR_EVOPART_FLAG;
                            ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                        }
                    } else
                    if((eventpartition == 79) || (eventpartition == 89) || (eventpartition == 89) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188))	{
                        SysFlag_AP_GenAlarm |= bitpat[INCE_bit];
                        recharge_alarm(INCE_bit);
                        INCE_timer = SEC_TIMER;

                        ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_INCE_FLAG;
                    }

				}
				else	{
					OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
					if((ptm_dcb[index].particion >= 10) && (ptm_dcb[index].particion < 55))	{
                        if((ptm_dcb[index].particion != 27) && (ptm_dcb[index].particion != 28) && (ptm_dcb[index].particion != 29) \
                        && (ptm_dcb[index].particion != 47) && (ptm_dcb[index].particion != 48) && (ptm_dcb[index].particion != 49)) {
                            //SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
							//recharge_alarm(TESO_bit);
                            //TESO_timer = SEC_TIMER;
                            if((eventzone == 1) || (eventzone == 0))  {
                                SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
                                recharge_alarm(TESO_bit);
                                TESO_timer = SEC_TIMER;
                                RFDLYBOR_flag |= RFDLYBOR_TDONEI_FLAG;
                            } else
                            if(eventzone == 2)  {
                                if(ptm_dcb[eveindex].RFALRMDLY_flag & PTMSIG_OLDPANIC)  {
                                    ptm_dcb[eveindex].RFALRMDLY_flag &= ~PTMSIG_OLDPANIC;
                                    SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
                                    recharge_alarm(TESO_bit);
                                    TESO_timer = SEC_TIMER;
                                    RFDLYBOR_flag |= RFDLYBOR_TDONEI_FLAG;
                                } else {
                                    dlyedptm_zone[index] = eventzone;
                                    ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                                }
                            }
                        }
					} else
					if((ptm_dcb[index].particion >= 60) && (ptm_dcb[index].particion <= 62))	{
						SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
						recharge_alarm(TESO_bit);
						ASAL_timer = SEC_TIMER;
                        //dlyedptm_zone[index] = eventzone;
                        //ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_ASAL_FLAG;
					} else
                    if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))	{
                        SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
                        recharge_alarm(ASAL_bit);
                        ASAL_timer = SEC_TIMER;
                        //dlyedptm_zone[index] = eventzone;
                        //ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_ASAL_FLAG;
                    } else
                    if((ptm_dcb[index].particion >= 71) && (ptm_dcb[index].particion <= 78) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188) && (eventzone != 4))	{
                        dlyedptm_zone[index] = eventzone;
                        ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                    } else
                    if((ptm_dcb[index].particion >= 81) && (ptm_dcb[index].particion <= 88) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188) && (eventzone != 4))	{
                        dlyedptm_zone[index] = eventzone;
                        ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                    } else
                    if((ptm_dcb[index].particion >= 91) && (ptm_dcb[index].particion <= 98) && (eventzone != 186) && (eventzone != 187) && (eventzone != 188) && (eventzone != 4))	{
                        dlyedptm_zone[index] = eventzone;
                        ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_TESO_FLAG;
                    } else
					if((ptm_dcb[index].particion == 79) || (ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 89))	{
						SysFlag_AP_GenAlarm |= bitpat[INCE_bit];
						recharge_alarm(INCE_bit);
						INCE_timer = SEC_TIMER;

                        ptm_dcb[index].RFALRMDLY_flag |= RFALRMDLY_INCE_FLAG;
					}
					switch(ptm_dcb[index].disparo)	{
                        case 'S':
                            TasFlags |= TASVANDAL_FLAG;
                            return;
                            break;
                        default:
                            break;
					}
				}
				if( (ptm_dcb[index].rtuaddr < 240) || (ptm_dcb[index].rtuaddr >= 243) )	{
					if(eventzone == 1)	{
					    if(!(ptm_dcb[index].event_alarm & EVEALRM_BURG_TERMIC)) {
                            ptm_dcb[index].event_alarm |= EVEALRM_BURG_IMD;
                            ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                            //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                        }
					} else
					if(eventzone == 2)	{
					    if(eventcode == 134)    {
                            ptm_dcb[index].event_alarm |= EVEALRM_ENTRYEXIT;
                            ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                            ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                            //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
					    } else  {
                            ptm_dcb[index].event_alarm |= EVEALRM_BURG_TMP;
                            ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                            //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
					    }
					}
				} else {
					ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
					ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                    //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
				}

				if((ptm_dcb[index].particion == 68) || (ptm_dcb[index].particion == 69))	{
					ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
					ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
				}
                fsm_rfdlyptm();
				break;
			case 145:		// Tamper
				OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
				if((ptm_dcb[index].particion >= 10) && (ptm_dcb[index].particion < 55))	{
                    if((ptm_dcb[index].particion != 27) && (ptm_dcb[index].particion != 28) && (ptm_dcb[index].particion != 29) \
                        && (ptm_dcb[index].particion != 47) && (ptm_dcb[index].particion != 48) && (ptm_dcb[index].particion != 49)) {
                        SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
                        TESO_timer = SEC_TIMER;
						recharge_alarm(TESO_bit);
                    }
				} else
				if((ptm_dcb[index].particion >= 60) && (ptm_dcb[index].particion <= 62))	{
					SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
					recharge_alarm(TESO_bit);
					ASAL_timer = SEC_TIMER;
				} else
                if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))	{
                    SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
                    recharge_alarm(ASAL_bit);
                    ASAL_timer = SEC_TIMER;
                } else
				if((ptm_dcb[index].particion == 79) || (ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 89))	{
					SysFlag_AP_GenAlarm |= bitpat[INCE_bit];
					INCE_timer = SEC_TIMER;
					recharge_alarm(INCE_bit);
				}
				switch(ptm_dcb[index].disparo)	{
                    case 'S':
                        TasFlags |= TASVANDAL_FLAG;
                        return;
                        break;
                    default:
                        break;
				}
				if(ptm_dcb[index].particion != 58)
					ptm_dcb[index].event_alarm |= EVEALRM_TAMPER;
				break;
			case 621:
				if(SEC_TIMER - TESO_timer > 5*60)	{
					OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
					if( ptm_dcb[index].event_alarm & EVEALRM_PANIC )	{
						GenerateCIDEventPTm(index, 'R', 121, 0);
						ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC;
					}
                    if( ptm_dcb[index].event_alarm & EVEALRM_PANIC128 )	{
                        GenerateCIDEventPTm(index, 'R', 130, 0);
                        GenerateCIDEventPTm(index, 'R', 128, 0);
                        ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC128;
                    }
					if( ptm_dcb[index].event_alarm & EVEALRM_BURG_IMD )	{
						GenerateCIDEventPTm(index, 'R', 130, 1);
						ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
					}
					if( ptm_dcb[index].event_alarm & EVEALRM_BURG_TMP )	{
						GenerateCIDEventPTm(index, 'R', 130, 2);
						ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
					}
                    if( ptm_dcb[index].event_alarm & EVEALRM_ENTRYEXIT )	{
                        GenerateCIDEventPTm(index, 'R', 134, 2);
                        ptm_dcb[index].event_alarm &= ~EVEALRM_ENTRYEXIT;
                    }
					if( ptm_dcb[index].event_alarm & EVEALRM_TAMPER )	{
						GenerateCIDEventPTm(index, 'R', 145, 1);
						ptm_dcb[index].event_alarm &= ~EVEALRM_TAMPER;
					}
				}
				break;
		}
	}
	if(event_buffer[8] == 'R')	{
		switch(eventcode)	{
			case 628:
				OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
				GenerateCIDEventPTm(index, 'R', 401, 9);
				ptm_dcb[index].event_alarm &= ~EVEALRM_TECHMAN;
                //-----------------------------
                for(i = 0; i < 5; i++)	{
                    if((PT_estado_particion[i] == IntToBCD(eventpartition)) && (PT_estado_particion[i] != 0x00))	{
                        BaseAlarmPkt_estado_dispositivos |= (1 << (i+3));
                        AlarmWriteHistory();
                    }
                }
                //-----------------------------
				break;
			case 403:
			case 408:
			case 401:
				if( (ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr < 243) )	{
					temp = ptm_dcb[index].rtuaddr - 240;
					PDX_dev_status[temp] &= ~(1 << (subpartition - 1));
					guardar_PDX_status();
                    ptm_dcb[index].RFALRMDLY_flag &= ~RFALRMDLY_DISARM_FLAG;
				} else
				if( (ptm_dcb[index].particion == CASTILLETEDVR_PARTITION) && (eventcode == 401) )	{
					SystemFlag7 |= CASTDVR_ACK;
				}
				break;
			case 988:
				if( ptm_dcb[index].particion == CASTILLETEDVR_PARTITION )	{
					SystemFlag7 |= CASTDVR_ACK;
				}
				break;
			case 354:
				if(eventzone == 0)	{			//es la zona 0 para comunicador IP150
					SystemFlag7 |= IP150_ALIVE;
				}
				break;
			case 145:
				if(ptm_dcb[index].particion != 58)
					ptm_dcb[index].event_alarm &= ~EVEALRM_TAMPER;
				break;
		}
	}
}

void ProcessRestoreMainBoardEvents( void )
{
	unsigned char index;
	OS_ERR os_err;

	if(SysFlag2 & NORM_ASAL)	{
		SysFlag2 &= ~NORM_ASAL;
		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
		for(index = 0; index < MAXQTYPTM; index++)	{
			if(ptm_dcb[index].rtuaddr == 0)
				continue;
			if( (ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr < 243) )
				continue;

			if( ptm_dcb[index].event_alarm & EVEALRM_PANIC )	{
				GenerateCIDEventPTm(index, 'R', 121, 0);
				ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC;
			}
            if( ptm_dcb[index].event_alarm & EVEALRM_PANIC128 )	{
                GenerateCIDEventPTm(index, 'R', 130, 0);
                GenerateCIDEventPTm(index, 'R', 128, 0);
                ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC128;
            }
			if( ptm_dcb[index].event_alarm & EVEALRM_BURG_IMD )	{
                if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))    {
                    GenerateCIDEventPTm(index, 'R', 120, 1);
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                }
//                else
//                if(ptm_dcb[index].particion == 99) {
//                    GenerateCIDEventPTm(index, 'R', 110, 1);
//                } else {
//                    GenerateCIDEventPTm(index, 'R', 130, 1);
//                }

			}
			if( ptm_dcb[index].event_alarm & EVEALRM_BURG_TMP )	{
                if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))    {
                    GenerateCIDEventPTm(index, 'R', 120, 2);
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                }
//                else
//                if(ptm_dcb[index].particion == 99) {
//                    GenerateCIDEventPTm(index, 'R', 110, 2);
//                } else {
//                    GenerateCIDEventPTm(index, 'R', 130, 2);
//                }

			}
			if( (ptm_dcb[index].event_alarm & EVEALRM_TAMPER) && (!(PTM_dev_status[index] & 0x02)) )	{
				GenerateCIDEventPTm(index, 'R', 145, 1);
				ptm_dcb[index].event_alarm &= ~EVEALRM_TAMPER;
			}
		}
	}

	if(SysFlag2 & NORM_TESO)	{
		SysFlag2 &= ~NORM_TESO;
		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
		for(index = 0; index < MAXQTYPTM; index++)	{
			if(ptm_dcb[index].rtuaddr == 0)
				continue;
			if( (ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr < 243) )
				continue;
//			if(ptm_dcb[index].disparo != 'T')
//				continue;
			if( ptm_dcb[index].event_alarm & EVEALRM_PANIC )	{
				GenerateCIDEventPTm(index, 'R', 121, 0);
				ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC;
                //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
			}
            if( ptm_dcb[index].event_alarm & EVEALRM_PANIC128 )	{
                GenerateCIDEventPTm(index, 'R', 130, 0);
                GenerateCIDEventPTm(index, 'R', 128, 0);
                ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC128;
            }
            if(((ptm_dcb[index].particion < 26) || (ptm_dcb[index].particion > 28)) && (ptm_dcb[index].particion != 89)) {
                if ((ptm_dcb[index].event_alarm & EVEALRM_BURG_IMD) && (!(PTM_dev_status[index] & 0x08))) {
                    GenerateCIDEventPTm(index, 'R', 130, 1);
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                    //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                }
                if (ptm_dcb[index].event_alarm & EVEALRM_BURG_TERMIC) {
                    if (((ptm_dcb[index].particion >= 21) && (ptm_dcb[index].particion <= 25)) ||
                        ((ptm_dcb[index].particion >= 31) && (ptm_dcb[index].particion <= 39))) {
                        GenerateCIDEventPTm(index, 'R', 130, 908);
                    } else if ((ptm_dcb[index].particion >= 50) && (ptm_dcb[index].particion <= 54)) {
                        GenerateCIDEventPTm(index, 'R', 130, 907);
                    } else {
                        GenerateCIDEventPTm(index, 'R', 130, 909);
                    }

                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                }
                if (ptm_dcb[index].event_alarm & EVEALRM_BURG_TMP) {
                    GenerateCIDEventPTm(index, 'R', 130, 2);
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                    //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                }
            }

            if( ptm_dcb[index].event_alarm & EVEALRM_ENTRYEXIT )	{
                GenerateCIDEventPTm(index, 'R', 134, 2);
                ptm_dcb[index].event_alarm &= ~EVEALRM_ENTRYEXIT;
                //ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
            }

			if( (ptm_dcb[index].event_alarm & EVEALRM_TAMPER) && (!(PTM_dev_status[index] & 0x02)) )	{
				GenerateCIDEventPTm(index, 'R', 145, 1);
				ptm_dcb[index].event_alarm &= ~EVEALRM_TAMPER;
			}
		}
	}

	if(SysFlag2 & NORM_INCE)	{
		SysFlag2 &= ~NORM_INCE;
		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
		for(index = 0; index < MAXQTYPTM; index++)	{
			if(ptm_dcb[index].rtuaddr == 0)
				continue;
			if( (ptm_dcb[index].rtuaddr >= 240) && (ptm_dcb[index].rtuaddr < 243) )
				continue;

//			if( ptm_dcb[index].event_alarm & EVEALRM_PANIC )	{
//				GenerateCIDEventPTm(index, 'R', 121, 0);
//				ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC;
//			}
//            if( ptm_dcb[index].event_alarm & EVEALRM_PANIC128 )	{
//                GenerateCIDEventPTm(index, 'R', 130, 0);
//                GenerateCIDEventPTm(index, 'R', 128, 0);
//                ptm_dcb[index].event_alarm &= ~EVEALRM_PANIC128;
//            }
            if( ptm_dcb[index].event_alarm & EVEALRM_BURG_IMD )	{
                if((ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 79)) {
                    GenerateCIDEventPTm(index, 'R', 110, 1);
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                }
//                else {
//                    GenerateCIDEventPTm(index, 'R', 130, 1);
//                }

            }
            if( ptm_dcb[index].event_alarm & EVEALRM_BURG_TMP )	{
                if((ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 79)) {
                    GenerateCIDEventPTm(index, 'R', 110, 2);
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                }
//                else {
//                    GenerateCIDEventPTm(index, 'R', 130, 2);
//                }

            }

		}
	}
}

void ProcessRestoreByTimeout(void)
{
   unsigned char index;
   int i;


	if(SEC_TIMER - TECHMAN_timer > 20*60)	{
		for(index = 0; index < MAXQTYPTM; index++)	{
			if((ptm_dcb[index].particion >= 70) && (ptm_dcb[index].particion <= 98))
				continue;
			if(ptm_dcb[index].rtuaddr == 0)
				continue;
			if( ptm_dcb[index].event_alarm & EVEALRM_TECHMAN )	{
				ptm_dcb[index].event_alarm &= ~EVEALRM_TECHMAN;
				GenerateCIDEventPTm(index, 'R', 401, 9);

                //-----------------------------
                for(i = 0; i < 5; i++)	{
                    if((PT_estado_particion[i] == currentEvent.cid_partition) && (PT_estado_particion[i] != 0x00))	{
                        BaseAlarmPkt_estado_dispositivos |= (1 << (i+3));
                        AlarmWriteHistory();
                    }
                }
                //-----------------------------
			}
		}
	}
	if(SEC_TIMER - TECHMAN9_timer > 20*60)	{
		for(index = 0; index < MAXQTYPTM; index++)	{
			if((ptm_dcb[index].particion >= 70) && (ptm_dcb[index].particion <= 98))
				continue;
			if(ptm_dcb[index].rtuaddr == 0)
				continue;
			if( ptm_dcb[index].event_alarm & EVEALRM_TECHMAN9 )	{
				ptm_dcb[index].event_alarm &= ~EVEALRM_TECHMAN9;
				GenerateCIDEventPTm(index, 'R', 401, 10);

                //-----------------------------
                for(i = 0; i < 5; i++)	{
                    if((PT_estado_particion[i] == currentEvent.cid_partition) && (PT_estado_particion[i] != 0x00))	{
                        BaseAlarmPkt_estado_dispositivos |= (1 << (i+3));
                        AlarmWriteHistory();
                    }
                }
                //-----------------------------
			}
		}
	}

	//maquina para la gestion de normalizacion de los ptmacro
	for(index = 0; index < MAXQTYPTM; index++)	{
		if((ptm_dcb[index].particion >= 70) && (ptm_dcb[index].particion <= 98) && (ptm_dcb[index].particion != 89) && (ptm_dcb[index].particion != 79))
			continue;
		switch(ptm_dcb[index].normstate)	{
		case PTNORM_IDLE:
			if(ptm_dcb[index].flags & AUTONORMAL)	{
				ptm_dcb[index].normstate = PTNORM_WAIT;
				ptm_dcb[index].flags &= ~AUTONORMAL;
				if(ptm_dcb[index].particion == 55)
				    ptm_dcb[index].normtimer = SEC_TIMER + 15;
				else    {
				    ptm_dcb[index].normtimer = SEC_TIMER + 15*60;
				}
			}
			break;
		case PTNORM_WAIT:
			if(ptm_dcb[index].normtimer <= SEC_TIMER)	{
				ptm_dcb[index].normstate = PTNORM_IDLE;
				if(ptm_dcb[index].particion == 55)  {
				    GenerateCIDEventPTm(index, 'R', 931, 0);
				    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
				    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
				    ptm_dcb[index].event_alarm &= ~EVEALRM_ENTRYEXIT;
				    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
				}
				if(ptm_dcb[index].event_alarm & EVEALRM_BURG_TMP)	{
					ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TMP;
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                    if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))    {
                        GenerateCIDEventPTm(index, 'R', 120, 2);
                    } else
                    if((ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 79)) {
                        GenerateCIDEventPTm(index, 'R', 110, 2);
                    } else
                    {
                        GenerateCIDEventPTm(index, 'R', 130, 2);
                    }
				}
                if(ptm_dcb[index].event_alarm & EVEALRM_ENTRYEXIT)	{
                    ptm_dcb[index].event_alarm &= ~EVEALRM_ENTRYEXIT;
                    GenerateCIDEventPTm(index, 'R', 134, 2);
                }
				if(ptm_dcb[index].event_alarm & EVEALRM_BURG_IMD)	{
					ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_IMD;
                    if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))    {
                        GenerateCIDEventPTm(index, 'R', 120, 1);
                    } else
                    if((ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 79)) {
                        GenerateCIDEventPTm(index, 'R', 110, 1);
                    } else
                    {
                        GenerateCIDEventPTm(index, 'R', 130, 1);
                    }

				}
                if( ptm_dcb[index].event_alarm & EVEALRM_BURG_TERMIC )	{
                    if((ptm_dcb[index].particion >= 26) && (ptm_dcb[index].particion <= 28))    {
                        GenerateCIDEventPTm(index, 'R', 120, 2);
                    }
                    if((ptm_dcb[index].particion == 89) || (ptm_dcb[index].particion == 79)) {
                        GenerateCIDEventPTm(index, 'R', 110, 2);
                    }
                    if(((ptm_dcb[index].particion >= 21) && (ptm_dcb[index].particion <= 25)) ||
                       ((ptm_dcb[index].particion >= 31) && (ptm_dcb[index].particion <= 39))) {
                        GenerateCIDEventPTm(index, 'R', 130, 908);
                    } else
                    if((ptm_dcb[index].particion >= 50) && (ptm_dcb[index].particion <= 54))    {
                        GenerateCIDEventPTm(index, 'R', 130, 907);
                    }   else  {
                        GenerateCIDEventPTm(index, 'R', 130, 909);
                    }
                    ptm_dcb[index].event_alarm &= ~EVEALRM_BURG_TERMIC;
                }
			}
			break;
		default:
			ptm_dcb[index].normstate = PTNORM_IDLE;
			break;
		}
	}
}

void guardar_PDX_status(void)
{
	uint32_t len;
	uint8_t mybuffer[8];

	mybuffer[0] = PDX_dev_status[0];
	mybuffer[1] = PDX_dev_status[1];
	mybuffer[2] = PDX_dev_status[2];
	mybuffer[3] = 0x5A;


	len = flash0_write(1, mybuffer, DF_PDXSTATUS_OFFSET, 4);
}

void recuperar_PDX_status(void)
{
	uint32_t len;
	uint8_t mybuffer[8];

	len = flash0_read(mybuffer, DF_PDXSTATUS_OFFSET, 4);

	if(mybuffer[3] == 0x5A)	{
		PDX_dev_status[0] = mybuffer[0];
		PDX_dev_status[1] = mybuffer[1];
		PDX_dev_status[2] = mybuffer[2];
	} else	{
		PDX_dev_status[0] = 0x00;
		PDX_dev_status[1] = 0x00;
		PDX_dev_status[2] = 0x00;
	}
}

void set_array_bit( uint8_t bit, uint8_t bitarray[])
{
	uint8_t row, column;

	row = bit / 8;
	column = bit % 8;

	bitarray[row] |= (1 << column);
}

void reset_array_bit( uint8_t bit, uint8_t bitarray[])
{
	uint8_t row, column;

	row = bit / 8;
	column = bit % 8;

	bitarray[row] &= ~(1 << column);
}

int get_array_bit( uint8_t bit, uint8_t bitarray[])
{
	uint8_t row, column;

	row = bit / 8;
	column = bit % 8;

	if(bitarray[row] & (1 << column))	{
		return 1;
	} else	{
		return 0;
	}
}


void Modules485_Scan( void )
{
	uint8_t ldatabuffer[6], lrxbuffer[64];
	int i, nread;
	OS_ERR	os_err;

	//------------------------------------------------------------------
	// Busco que dispositivos estan colgados de la red 485
	for( i = 0; i <= 253; i++ )	{
		WDT_Feed();
		ldatabuffer[0] = 0x0A;
		ldatabuffer[1] = (uint8_t)i;
		ldatabuffer[2] = 'P';
		ldatabuffer[3] = ptm_pwd;
		ldatabuffer[4] = 0x0D;
		LAN485_Send( ldatabuffer, 5 );
		//device_poll( i );
		OSTimeDlyHMSM(0, 0, 0, 30, OS_OPT_TIME_HMSM_STRICT, &os_err);
		nread = ComGetBuff(COMM2, 10, lrxbuffer, 64);
		if((nread == 7) || (nread == 15) || (nread == 26))	{
            if(lrxbuffer[2] == (uint8_t)i) {
                LAN485_Send( ldatabuffer, 5 );
                OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
                nread = ComGetBuff(COMM2, 10, lrxbuffer, 64);

                if((nread == 7) || (nread == 15) || (nread == 26)) {
                    if (lrxbuffer[2] == (uint8_t) i) {
                        if(!ModuleInTable(i))	{
                            logCidEvent(account, 1, 991, 0, i);		//Dispositivo detectado y no declarado en la 485
                            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
                        }
                    }
                }

            }
		}
	}
}

int PBTinTable( int particion, int zone )
{
	int i;

	for( i = 0; i < MAXPBTPTM; i++ )	{
		if(pbt_dcb[i].particion == particion)	{
			if(pbt_dcb[i].zona == zone)
				return i;
		}
	}
	return -1;
}

int ModuleInTable( int addr )
{
	int i;

	for( i = 0; i < MAXQTYPTM; i++ )	{
		if( ptm_dcb[i].rtuaddr == addr)	{
			return 1;
		}
	}

	return 0;
}

int AllPTM_KeyAccepted( void )
{

}

#define KEY_REJECTED 0x20

int AnyPTM_KeyRejected( void )
{

	int i, error;
	static int lap;

	if( (lap < 0) || (lap > 5) )
		lap = 0;

	for( i = 0, error = 0; i < MAXQTYPTM; i++ )	{
		if( (PTM_dev_status[i] & KEY_REJECTED) && (ptm_dcb[i].rtuaddr != 0x00))
			error++;
	}
	lap++;

	if( lap == 5)	{
		lap = 0;
		if(error)	{
			new_ptm_pwd();
			return 1;
		}
	}
	return 0;
}


uint8_t fsmnppwd;
#define FSMNPP_IDLE		0x10
#define FSMNPP_TRG		0x20

uint32_t fsmnpp_timer;

void fsm_newptmpwd( void )
{
	switch(fsmnppwd)	{
		case FSMNPP_IDLE:
			if(SystemFlag6 & TRGNPPWD_FLAG)	{
				fsmnpp_timer = 120*60;			//dos horas
				fsmnppwd = FSMNPP_TRG;
			}
			break;
		case FSMNPP_TRG:
			if(!fsmnpp_timer)	{
				new_ptm_pwd();
				fsmnppwd = FSMNPP_IDLE;
			}
			break;
		default:
			fsmnppwd = FSMNPP_IDLE;
			fsmnpp_timer = 0;
			break;
	}
}


uint16_t GetPTMZoneNumber( int particion, int zone12 )
{
    uint16_t temppart,tempzone;

    temppart = particion % 10;
    if(temppart >= 5)
        tempzone = 0x800;
    else
        tempzone = 0x900;
    tempzone += (particion /10) * 0x10;

    if(temppart >= 5)
        tempzone += ((zone12 & 0x0F) + ((temppart - 5) * 2));
    else
        tempzone += ((zone12 & 0x0F) + (temppart * 2));

    if(tempzone == 0x81A)
        tempzone = 0x810;
    if(tempzone == 0x91A)
        tempzone = 0x910;

    return tempzone;
}

uint8_t fsmhsbclock_state;
//#define FSMHBL_IDLE     0x10
//#define FSMHBL_LOCK1    0x20
//#define FSMHBL_LOCK2    0x30
//#define FSMHBL_LOCK12   0x40
//#define FSMHBL_LOCKALR  0x50

uint32_t fsmhlock_timer;

void fsm_hsbclock( void )
{
    switch(fsmhsbclock_state)   {
        case FSMHBL_IDLE:
            if(SystemFlag8 & LOCK1_OPEN)    {
                fsmhsbclock_state = FSMHBL_LOCK1;
                fsmhlock_timer = 60*1000;           //maximo permitido de puerta abierta 1 minuto
            } else
            if(SystemFlag8 & LOCK2_OPEN)    {
                fsmhsbclock_state = FSMHBL_LOCK2;
                fsmhlock_timer = 60*1000;           //maximo permitido de puerta abierta 1 minuto
            }
            break;
        case FSMHBL_LOCK1:
            if(SystemFlag8 & LOCK2_OPEN)    {
                fsmhsbclock_state = FSMHBL_LOCK12;
                fsmhlock_timer = 700;               //700 mseg de tolerancia para ambas puertas abiertas
            } else
            if(!(SystemFlag8 & LOCK1_OPEN)) {
                fsmhsbclock_state = FSMHBL_IDLE;
                fsmhlock_timer = 0;
            } else
            if(!fsmhlock_timer) {
                logCidEvent(account, 1, 962, hsbc_lock_partition, (uint16_t )GetPTMZoneNumber(hsbc_lock_partition, 1));
                fsmhsbclock_state = FSMHBL_LOCKLONG1;
            }
            break;
        case FSMHBL_LOCK2:
            if(SystemFlag8 & LOCK1_OPEN)    {
                fsmhsbclock_state = FSMHBL_LOCK12;
                fsmhlock_timer = 700;               //700 mseg de tolerancia para ambas puertas abiertas
            } else
            if(!(SystemFlag8 & LOCK2_OPEN)) {
                fsmhsbclock_state = FSMHBL_IDLE;
                fsmhlock_timer = 0;
            } else
            if(!fsmhlock_timer) {
                logCidEvent(account, 1, 962, hsbc_lock_partition, (uint16_t )GetPTMZoneNumber(hsbc_lock_partition, 2));
                fsmhsbclock_state = FSMHBL_LOCKLONG2;
            }
            break;
        case FSMHBL_LOCK12:
            if(!fsmhlock_timer) {                   //disparo de alarma
                SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
				recharge_alarm(ASAL_bit);
                ASAL_timer = SEC_TIMER;
                logCidEvent(account, 1, 961, hsbc_lock_partition, 0);
                fsmhsbclock_state = FSMHBL_LOCKALR;
            } else
            if(!(SystemFlag8 & LOCK1_OPEN)) {
                fsmhsbclock_state = FSMHBL_LOCK2;
                fsmhlock_timer = 60*1000;
            } else
            if(!(SystemFlag8 & LOCK2_OPEN)) {
                fsmhsbclock_state = FSMHBL_LOCK1;
                fsmhlock_timer = 60*1000;
            }
            break;
        case FSMHBL_LOCKLONG1:
            if((SystemFlag8 & LOCK1_OPEN) && (SystemFlag8 & LOCK2_OPEN))    {
                SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
				recharge_alarm(ASAL_bit);
                ASAL_timer = SEC_TIMER;
                logCidEvent(account, 1, 961, hsbc_lock_partition, 0);
                fsmhsbclock_state = FSMHBL_LOCKALR1;
            } else
            if((!(SystemFlag8 & LOCK1_OPEN)) && (!(SystemFlag8 & LOCK2_OPEN)))  {
                logCidEvent(account, 3, 962, hsbc_lock_partition, (uint16_t )GetPTMZoneNumber(hsbc_lock_partition, 1));
                fsmhsbclock_state = FSMHBL_IDLE;
                fsmhlock_timer = 0;
            }
            break;
		case FSMHBL_LOCKLONG2:
			if((SystemFlag8 & LOCK1_OPEN) && (SystemFlag8 & LOCK2_OPEN))    {
				SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
				recharge_alarm(ASAL_bit);
				ASAL_timer = SEC_TIMER;
				logCidEvent(account, 1, 961, hsbc_lock_partition, 0);
				fsmhsbclock_state = FSMHBL_LOCKALR2;
			} else
			if((!(SystemFlag8 & LOCK1_OPEN)) && (!(SystemFlag8 & LOCK2_OPEN)))  {
				logCidEvent(account, 3, 962, hsbc_lock_partition, (uint16_t )GetPTMZoneNumber(hsbc_lock_partition, 2));
				fsmhsbclock_state = FSMHBL_IDLE;
				fsmhlock_timer = 0;
			}
			break;
        case FSMHBL_LOCKALR:
            if((!(SystemFlag8 & LOCK1_OPEN)) && (!(SystemFlag8 & LOCK2_OPEN)))  {
                logCidEvent(account, 3, 961, hsbc_lock_partition, 0);
                fsmhsbclock_state = FSMHBL_IDLE;
                fsmhlock_timer = 0;
            }
            break;
		case FSMHBL_LOCKALR1:
			if((!(SystemFlag8 & LOCK1_OPEN)) && (!(SystemFlag8 & LOCK2_OPEN)))  {
				logCidEvent(account, 3, 961, hsbc_lock_partition, 0);
				logCidEvent(account, 3, 962, hsbc_lock_partition, (uint16_t )GetPTMZoneNumber(hsbc_lock_partition, 1));
				fsmhsbclock_state = FSMHBL_IDLE;
				fsmhlock_timer = 0;
			}
			break;
		case FSMHBL_LOCKALR2:
			if((!(SystemFlag8 & LOCK1_OPEN)) && (!(SystemFlag8 & LOCK2_OPEN)))  {
				logCidEvent(account, 3, 961, hsbc_lock_partition, 0);
				logCidEvent(account, 3, 962, hsbc_lock_partition, (uint16_t )GetPTMZoneNumber(hsbc_lock_partition, 2));
				fsmhsbclock_state = FSMHBL_IDLE;
				fsmhlock_timer = 0;
			}
			break;
        default:
            fsmhsbclock_state = FSMHBL_IDLE;
            fsmhlock_timer = 0;
            break;
    }


}

uint8_t fsme401volp5_state;
uint8_t fsme401volp6_state;
uint8_t fsme401volp7_state;
uint8_t fsme401volp8_state;
uint8_t fsme401volp9_state;
#define FSME401V_IDLE       0x10
#define FSME401V_WAIT_E05   0x20
#define FSME401V_WAIT_E71   0x30
#define FSME401V_WAIT_R05   0x40
#define FSME401V_WAIT_R71   0x50

time_t fsme401v_p5_timer;
time_t fsme401v_p6_timer;
time_t fsme401v_p7_timer;
time_t fsme401v_p8_timer;
time_t fsme401v_p9_timer;

void fsm_e401_volumetrica_p5(void)
{
	if(VolumetricRedundance[0] == 0)
		return;

    switch(fsme401volp5_state)    {
        case FSME401V_IDLE:
            if(SystemFlag9 & P5_E401_71_RCVD)  {
                SystemFlag9 &= ~P5_E401_71_RCVD;
                fsme401volp5_state = FSME401V_WAIT_E05;
            } else
            if(SystemFlag9 & P5_R401_71_RCVD)  {
                SystemFlag9 &= ~P5_R401_71_RCVD;
                fsme401volp5_state = FSME401V_WAIT_R05;
            } else
            if(SystemFlag9 & P5_E401_05_RCVD)  {
                SystemFlag9 &= ~P5_E401_05_RCVD;
                fsme401volp5_state = FSME401V_WAIT_E71;
            } else
            if(SystemFlag9 & P5_R401_05_RCVD)  {
                SystemFlag9 &= ~P5_R401_05_RCVD;
                fsme401volp5_state = FSME401V_WAIT_R71;
            }
            fsme401v_p5_timer = SEC_TIMER + 10*60;
            break;
        case FSME401V_WAIT_E05:
            if(SystemFlag9 & P5_E401_05_RCVD)  {
                SystemFlag9 &= ~P5_E401_05_RCVD;
                fsme401volp5_state = FSME401V_IDLE;
            } else
            if(fsme401v_p5_timer < SEC_TIMER)  {
                logCidEvent(account, 1, 812, 0x05, 1);
                logCidEvent(account, 1, 401, 0x05, 0);
                fsme401volp5_state = FSME401V_IDLE;
            }
            break;
        case FSME401V_WAIT_E71:
            if(SystemFlag9 & P5_E401_71_RCVD)  {
                SystemFlag9 &= ~P5_E401_71_RCVD;
                fsme401volp5_state = FSME401V_IDLE;
            } else
            if(fsme401v_p5_timer < SEC_TIMER)  {
                logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[0]), 1);
                logCidEvent(account, 1, 401, BCDToInt(VolumetricRedundance[0]), 0);
                fsme401volp5_state = FSME401V_IDLE;
            }
            break;
        case FSME401V_WAIT_R05:
            if(SystemFlag9 & P5_R401_05_RCVD)  {
                SystemFlag9 &= ~P5_R401_05_RCVD;
                fsme401volp5_state = FSME401V_IDLE;
            } else
            if(fsme401v_p5_timer < SEC_TIMER)  {
                logCidEvent(account, 1, 812, 0x05, 2);
                logCidEvent(account, 3, 401, 0x05, 0);
                fsme401volp5_state = FSME401V_IDLE;
            }
            break;
        case FSME401V_WAIT_R71:
            if(SystemFlag9 & P5_R401_71_RCVD)  {
                SystemFlag9 &= ~P5_R401_71_RCVD;
                fsme401volp5_state = FSME401V_IDLE;
            } else
            if(fsme401v_p5_timer < SEC_TIMER)  {
                logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[0]), 2);
                logCidEvent(account, 3, 401, BCDToInt(VolumetricRedundance[0]), 0);
                fsme401volp5_state = FSME401V_IDLE;
            }
            break;
        default:
            fsme401volp5_state = FSME401V_IDLE;
            break;
    }
}


void fsm_e401_volumetrica_p6(void)
{
	if(VolumetricRedundance[0] == 0)
		return;

	switch(fsme401volp6_state)    {
		case FSME401V_IDLE:
			if(SystemFlag9 & P6_E401_71_RCVD)  {
				SystemFlag9 &= ~P6_E401_71_RCVD;
				fsme401volp6_state = FSME401V_WAIT_E05;
			} else
			if(SystemFlag9 & P6_R401_71_RCVD)  {
				SystemFlag9 &= ~P6_R401_71_RCVD;
				fsme401volp6_state = FSME401V_WAIT_R05;
			} else
			if(SystemFlag9 & P6_E401_05_RCVD)  {
				SystemFlag9 &= ~P6_E401_05_RCVD;
				fsme401volp6_state = FSME401V_WAIT_E71;
			} else
			if(SystemFlag9 & P6_R401_05_RCVD)  {
				SystemFlag9 &= ~P6_R401_05_RCVD;
				fsme401volp6_state = FSME401V_WAIT_R71;
			}
			fsme401v_p6_timer = SEC_TIMER + 10*60;
			break;
		case FSME401V_WAIT_E05:
			if(SystemFlag9 & P6_E401_05_RCVD)  {
				SystemFlag9 &= ~P6_E401_05_RCVD;
				fsme401volp6_state = FSME401V_IDLE;
			} else
			if(fsme401v_p6_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x06, 1);
                logCidEvent(account, 1, 401, 0x06, 0);
				fsme401volp6_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_E71:
			if(SystemFlag9 & P6_E401_71_RCVD)  {
				SystemFlag9 &= ~P6_E401_71_RCVD;
				fsme401volp6_state = FSME401V_IDLE;
			} else
			if(fsme401v_p6_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[1]), 1);
                logCidEvent(account, 1, 401, BCDToInt(VolumetricRedundance[1]), 0);
				fsme401volp6_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R05:
			if(SystemFlag9 & P6_R401_05_RCVD)  {
				SystemFlag9 &= ~P6_R401_05_RCVD;
				fsme401volp6_state = FSME401V_IDLE;
			} else
			if(fsme401v_p6_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x06, 2);
                logCidEvent(account, 3, 401, 0x06, 0);
				fsme401volp6_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R71:
			if(SystemFlag9 & P6_R401_71_RCVD)  {
				SystemFlag9 &= ~P6_R401_71_RCVD;
				fsme401volp6_state = FSME401V_IDLE;
			} else
			if(fsme401v_p6_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[1]), 2);
                logCidEvent(account, 3, 401, BCDToInt(VolumetricRedundance[1]), 0);
				fsme401volp6_state = FSME401V_IDLE;
			}
			break;
		default:
			fsme401volp6_state = FSME401V_IDLE;
			break;
	}
}

void fsm_e401_volumetrica_p7(void)
{

	if(VolumetricRedundance[2] == 0)
		return;

	switch(fsme401volp7_state)    {
		case FSME401V_IDLE:
			if(SystemFlag9 & P7_E401_71_RCVD)  {
				SystemFlag9 &= ~P7_E401_71_RCVD;
				fsme401volp7_state = FSME401V_WAIT_E05;
			} else
			if(SystemFlag9 & P7_R401_71_RCVD)  {
				SystemFlag9 &= ~P7_R401_71_RCVD;
				fsme401volp7_state = FSME401V_WAIT_R05;
			} else
			if(SystemFlag9 & P7_E401_05_RCVD)  {
				SystemFlag9 &= ~P7_E401_05_RCVD;
				fsme401volp7_state = FSME401V_WAIT_E71;
			} else
			if(SystemFlag9 & P7_R401_05_RCVD)  {
				SystemFlag9 &= ~P7_R401_05_RCVD;
				fsme401volp7_state = FSME401V_WAIT_R71;
			}
			fsme401v_p7_timer = SEC_TIMER + 10*60;
			break;
		case FSME401V_WAIT_E05:
			if(SystemFlag9 & P7_E401_05_RCVD)  {
				SystemFlag9 &= ~P7_E401_05_RCVD;
				fsme401volp7_state = FSME401V_IDLE;
			} else
			if(fsme401v_p7_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x07, 1);
                logCidEvent(account, 1, 401, 0x07, 0);
				fsme401volp7_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_E71:
			if(SystemFlag9 & P7_E401_71_RCVD)  {
				SystemFlag9 &= ~P7_E401_71_RCVD;
				fsme401volp7_state = FSME401V_IDLE;
			} else
			if(fsme401v_p7_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[2]), 1);
                logCidEvent(account, 1, 401, BCDToInt(VolumetricRedundance[2]), 0);
				fsme401volp7_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R05:
			if(SystemFlag9 & P7_R401_05_RCVD)  {
				SystemFlag9 &= ~P7_R401_05_RCVD;
				fsme401volp7_state = FSME401V_IDLE;
			} else
			if(fsme401v_p7_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x07, 2);
                logCidEvent(account, 3, 401, 0x07, 0);
				fsme401volp7_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R71:
			if(SystemFlag9 & P7_R401_71_RCVD)  {
				SystemFlag9 &= ~P7_R401_71_RCVD;
				fsme401volp7_state = FSME401V_IDLE;
			} else
			if(fsme401v_p7_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[2]), 2);
                logCidEvent(account, 3, 401, BCDToInt(VolumetricRedundance[2]), 0);
				fsme401volp7_state = FSME401V_IDLE;
			}
			break;
		default:
			fsme401volp7_state = FSME401V_IDLE;
			break;
	}
}

void fsm_e401_volumetrica_p8(void)
{
	if(VolumetricRedundance[3] == 0)
		return;

	switch(fsme401volp8_state)    {
		case FSME401V_IDLE:
			if(SystemFlag9 & P8_E401_71_RCVD)  {
				SystemFlag9 &= ~P8_E401_71_RCVD;
				fsme401volp8_state = FSME401V_WAIT_E05;
			} else
			if(SystemFlag9 & P8_R401_71_RCVD)  {
				SystemFlag9 &= ~P8_R401_71_RCVD;
				fsme401volp8_state = FSME401V_WAIT_R05;
			} else
			if(SystemFlag9 & P8_E401_05_RCVD)  {
				SystemFlag9 &= ~P8_E401_05_RCVD;
				fsme401volp8_state = FSME401V_WAIT_E71;
			} else
			if(SystemFlag9 & P8_R401_05_RCVD)  {
				SystemFlag9 &= ~P8_R401_05_RCVD;
				fsme401volp8_state = FSME401V_WAIT_R71;
			}
			fsme401v_p8_timer = SEC_TIMER + 10*60;
			break;
		case FSME401V_WAIT_E05:
			if(SystemFlag9 & P8_E401_05_RCVD)  {
				SystemFlag9 &= ~P8_E401_05_RCVD;
				fsme401volp8_state = FSME401V_IDLE;
			} else
			if(fsme401v_p8_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x08, 1);
                logCidEvent(account, 1, 401, 0x08, 0);
				fsme401volp8_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_E71:
			if(SystemFlag9 & P8_E401_71_RCVD)  {
				SystemFlag9 &= ~P8_E401_71_RCVD;
				fsme401volp8_state = FSME401V_IDLE;
			} else
			if(fsme401v_p8_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[3]), 1);
                logCidEvent(account, 1, 401, BCDToInt(VolumetricRedundance[3]), 0);
				fsme401volp8_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R05:
			if(SystemFlag9 & P8_R401_05_RCVD)  {
				SystemFlag9 &= ~P8_R401_05_RCVD;
				fsme401volp8_state = FSME401V_IDLE;
			} else
			if(fsme401v_p8_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x08, 2);
                logCidEvent(account, 3, 401, 0x08, 0);
				fsme401volp8_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R71:
			if(SystemFlag9 & P8_R401_71_RCVD)  {
				SystemFlag9 &= ~P8_R401_71_RCVD;
				fsme401volp8_state = FSME401V_IDLE;
			} else
			if(fsme401v_p8_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[3]), 2);
                logCidEvent(account, 3, 401, BCDToInt(VolumetricRedundance[3]), 0);
				fsme401volp8_state = FSME401V_IDLE;
			}
			break;
		default:
			fsme401volp8_state = FSME401V_IDLE;
			break;
	}
}

void fsm_e401_volumetrica_p9(void)
{
	if(VolumetricRedundance[4] == 0)
		return;

	switch(fsme401volp9_state)    {
		case FSME401V_IDLE:
			if(SystemFlag9 & P9_E401_71_RCVD)  {
				SystemFlag9 &= ~P9_E401_71_RCVD;
				fsme401volp9_state = FSME401V_WAIT_E05;
			} else
			if(SystemFlag9 & P9_R401_71_RCVD)  {
				SystemFlag9 &= ~P9_R401_71_RCVD;
				fsme401volp9_state = FSME401V_WAIT_R05;
			} else
			if(SystemFlag9 & P9_E401_05_RCVD)  {
				SystemFlag9 &= ~P9_E401_05_RCVD;
				fsme401volp9_state = FSME401V_WAIT_E71;
			} else
			if(SystemFlag9 & P9_R401_05_RCVD)  {
				SystemFlag9 &= ~P9_R401_05_RCVD;
				fsme401volp9_state = FSME401V_WAIT_R71;
			}
			fsme401v_p9_timer = SEC_TIMER + 10*60;
			break;
		case FSME401V_WAIT_E05:
			if(SystemFlag9 & P9_E401_05_RCVD)  {
				SystemFlag9 &= ~P9_E401_05_RCVD;
				fsme401volp9_state = FSME401V_IDLE;
			} else
			if(fsme401v_p9_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x09, 1);
                logCidEvent(account, 1, 401, 0x09, 0);
				fsme401volp9_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_E71:
			if(SystemFlag9 & P9_E401_71_RCVD)  {
				SystemFlag9 &= ~P9_E401_71_RCVD;
				fsme401volp9_state = FSME401V_IDLE;
			} else
			if(fsme401v_p9_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[4]), 1);
                logCidEvent(account, 1, 401, BCDToInt(VolumetricRedundance[4]), 0);
				fsme401volp9_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R05:
			if(SystemFlag9 & P9_R401_05_RCVD)  {
				SystemFlag9 &= ~P9_R401_05_RCVD;
				fsme401volp9_state = FSME401V_IDLE;
			} else
			if(fsme401v_p9_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, 0x09, 2);
                logCidEvent(account, 3, 401, 0x09, 0);
				fsme401volp9_state = FSME401V_IDLE;
			}
			break;
		case FSME401V_WAIT_R71:
			if(SystemFlag9 & P9_R401_71_RCVD)  {
				SystemFlag9 &= ~P9_R401_71_RCVD;
				fsme401volp9_state = FSME401V_IDLE;
			} else
			if(fsme401v_p9_timer < SEC_TIMER)  {
				logCidEvent(account, 1, 812, BCDToInt(VolumetricRedundance[4]), 2);
                logCidEvent(account, 3, 401, BCDToInt(VolumetricRedundance[4]), 0);
				fsme401volp9_state = FSME401V_IDLE;
			}
			break;
		default:
			fsme401volp9_state = FSME401V_IDLE;
			break;
	}
}

void fsm_rfdlyptm( void )
{
    int this, i;


    for(this = 0; this < MAXQTYPTM; this++) {
        if( ptm_dcb[this].rtuaddr != 0x00)  {
            switch(ptm_dcb[this].rfalrmdly_state)   {
                case PTRFDLY_ST_IDLE:
                    if(ptm_dcb[this].RFALRMDLY_flag & RFALRMDLY_TESO_FLAG)  {
                        ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_DISARM_FLAG;

                        if((ptm_dcb[this].rtuaddr >= 240) && (ptm_dcb[this].rtuaddr < 243)) {
                            if(SystemFlag7 & RHBVOLU_FLAG) {
                                ptm_dcb[this].rfalrmdly_timer = SEC_TIMER;
                                ptm_dcb[this].RFALRMDLY_flag |= RFALRMDLY_EVOZ24_FLAG;
                            }
                            else {
                                if(rfdly_time < 20)
                                    ptm_dcb[this].rfalrmdly_timer = SEC_TIMER + 20;
                                else
                                    ptm_dcb[this].rfalrmdly_timer = SEC_TIMER + rfdly_time;
                                ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_EVOZ24_FLAG;
                            }
                        } else {
                            if(rfdly_time < 20)
                                ptm_dcb[this].rfalrmdly_timer = SEC_TIMER + 20;
                            else
                                ptm_dcb[this].rfalrmdly_timer = SEC_TIMER + rfdly_time;
                            ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_EVOZ24_FLAG;
                        }
                        ptm_dcb[this].rfalrmdly_state = PTRFDLY_ST_WAITTRIG;

                        RFDLYBOR_flag |= RFDLYBOR_LLOP_FLAG;
                        if(RFDLYBOR_flag & RFDLYBOR_E393HAB_FLAG) {
                            BaseAlarmPkt_alarm |= bitpat[LLOP_bit];
                            led_dcb[LLOP_led].led_cad = 255 * 0x100 + 0;
                            fsmAperWriteHistory();
                        }
                        if(RFDLYBOR_flag & RFDLYBOR_TESO_FLAG)  {
                            ptm_dcb[this].RFALRMDLY_flag |= RFALRMDLY_BORNERA_FLAG;
                        }
                    }

                    break;
                case PTRFDLY_ST_WAITTRIG:
                    if(RFDLYBOR_flag & RFDLYBOR_E393HAB_FLAG) {
                        BaseAlarmPkt_alarm |= bitpat[LLOP_bit];
                        led_dcb[LLOP_led].led_cad = 255 * 0x100 + 0;
                    }
                    if((ptm_dcb[this].RFALRMDLY_flag & RFALRMDLY_DISARM_FLAG) && (!(ptm_dcb[this].RFALRMDLY_flag & RFALRMDLY_EVOZ24_FLAG)))  {
                        ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_TESO_FLAG;
                        ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_ASAL_FLAG;
                        ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_DISARM_FLAG;
                        ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_BORNERA_FLAG;
                        ptm_dcb[this].rfalrmdly_state = PTRFDLY_ST_IDLE;
                        RFDLYBOR_flag &= ~RFDLYBOR_LLOP_FLAG;
                        if(RFDLYBOR_flag & RFDLYBOR_E393HAB_FLAG) {
                            BaseAlarmPkt_alarm &= ~bitpat[LLOP_bit];
                            led_dcb[LLOP_led].led_cad = 0;
                            led_dcb[LLOP_led].led_state = LED_IDLE;
                            fsmAperWriteHistory();
                        }
                        logCidEvent(account, 3, 393, 0, 0);
                        GenerateCIDEventPTm(this, 'E', 778, 0);
                        RFDLYBOR_flag |= RFDLYBOR_TDONE_FLAG;
                        //RFDLYBOR_flag &= ~RFDLYBOR_TDONE_FLAG;
                        RFDLYBOR_flag &= ~RFDLYBOR_TESO_FLAG;
                        RFDLYBOR_flag &= ~RFDLYBOR_TESOGAP_FLAG;
                        //RFDLYBOR_flag |= RFDLYBOR_TESOGAP_FLAG;
                        rfdlybornera_teso_state = BORRFDLY_ST_IDLE;
                    } else
                    if(SEC_TIMER >= ptm_dcb[this].rfalrmdly_timer)   {

                        GenerateCIDEventPTm(this, 'E', 130, dlyedptm_zone[this]);

                        if(ptm_dcb[this].RFALRMDLY_flag & RFALRMDLY_TESO_FLAG) {
                            ptm_dcb[this].RFALRMDLY_flag &= ~RFALRMDLY_TESO_FLAG;
                            ptm_dcb[this].rfalrmdly_state = PTRFDLY_ST_IDLE;
                            SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
                            recharge_alarm(TESO_bit);
                            TESO_timer = SEC_TIMER;
                            RFDLYBOR_flag &= ~RFDLYBOR_LLOP_FLAG;
                            if(RFDLYBOR_flag & RFDLYBOR_E393HAB_FLAG) {
                                //BaseAlarmPkt_alarm &= ~bitpat[LLOP_bit];
                                //led_dcb[LLOP_led].led_cad = 0;
                                //led_dcb[LLOP_led].led_state = LED_IDLE;
                                ptm_dcb[this].rfalrmdly_state = PTRFDLY_ST_WAITE393;
                                ptm_dcb[this].rfalrmdly_timer = SEC_TIMER + 3*60;
                            }
                            fsmAperWriteHistory();

                            //aca disparar por bornera si corresponde
                            if(ptm_dcb[this].RFALRMDLY_flag & RFALRMDLY_BORNERA_FLAG)   {
                                logCidEvent(account, 1, 130, 4, 0);
                                for(i = 0; i < MAXQTYPTM; i++)  {
                                    ptm_dcb[i].RFALRMDLY_flag &= ~RFALRMDLY_BORNERA_FLAG;
                                }
                            }

                        }

                        dlyedptm_zone[this] = 0;

                    }
                    break;
                case PTRFDLY_ST_WAITE393:
                    if(SEC_TIMER >= ptm_dcb[this].rfalrmdly_timer)  {
                        if(RFDLYBOR_flag & RFDLYBOR_E393HAB_FLAG) {
                            BaseAlarmPkt_alarm &= ~bitpat[LLOP_bit];
                            led_dcb[LLOP_led].led_cad = 0;
                            led_dcb[LLOP_led].led_state = LED_IDLE;
                            logCidEvent(account, 3, 393, 0, 0);
                        }
                        ptm_dcb[this].rfalrmdly_state = PTRFDLY_ST_IDLE;
                    }
                    break;
                default:
                    ptm_dcb[this].rfalrmdly_state = PTRFDLY_ST_IDLE;
                    break;
            }
        }
    }
}



time_t rfdlybor_teso_timer;
uint8_t DlyBor_time;

uint8_t RFDLYBOR_flag;


void fsm_rfdlybornera_teso( void )
{
    int index, ptm;

    switch(rfdlybornera_teso_state)  {
        case BORRFDLY_ST_IDLE:
            RFDLYBOR_flag &= ~RFDLYBOR_TDONE_FLAG;
            if(RFDLYBOR_flag & RFDLYBOR_TESOGAP_FLAG)   {
                RFDLYBOR_flag &= ~RFDLYBOR_TESOGAP_FLAG;
                rfdlybor_teso_timer = SEC_TIMER + 10;
                rfdlybornera_teso_state = BORRFDLY_ST_TESOGAP;
            } else
            if(RFDLYBOR_flag & RFDLYBOR_TESO_FLAG)  {
                rfdlybor_teso_timer = SEC_TIMER + DlyBor_time;
                rfdlybornera_teso_state = BORRFDLY_ST_WAITTRIG;
            }
            break;
        case BORRFDLY_ST_WAITTRIG:
            if(SEC_TIMER > rfdlybor_teso_timer)  {
                RFDLYBOR_flag &= ~RFDLYBOR_TESO_FLAG;
                ptm = 0;
                for(index = 0; index < MAXQTYPTM; index++)  {
                    if(ptm_dcb[index].RFALRMDLY_flag & RFALRMDLY_BORNERA_FLAG)  {
                        ptm++;
                    }
                }
                if(ptm == 0) {
                    //---------------------------------------------
                    //disparo la alarma de tesoro por bornera
                    BaseAlarmPkt_alarm |= bitpat[TESO_bit];
                    BaseAlarmPkt_memoria_dispositivos = OptoInputs;
                    AlarmWriteHistory();
                    SysFlag3 |= SEND_flag;
                    SysFlag3 |= SENDM_flag;
                    logCidEvent(account, 1, 130, 4, 0);
                    recharge_alarm(TESO_bit);
                    //---------------------------------------------
                }

                rfdlybornera_teso_state = BORRFDLY_ST_IDLE;
            } else
            if(RFDLYBOR_flag & RFDLYBOR_TDONE_FLAG) {
                RFDLYBOR_flag &= ~RFDLYBOR_TDONE_FLAG;
                RFDLYBOR_flag &= ~RFDLYBOR_TESO_FLAG;
                rfdlybornera_teso_state = BORRFDLY_ST_IDLE;
            } else
            if(RFDLYBOR_flag & RFDLYBOR_TDONEI_FLAG) {
                RFDLYBOR_flag &= ~RFDLYBOR_TDONEI_FLAG;
                RFDLYBOR_flag &= ~RFDLYBOR_TESO_FLAG;
                rfdlybornera_teso_state = BORRFDLY_ST_IDLE;
                logCidEvent(account, 1, 130, 4, 0);
            }
            break;
        case BORRFDLY_ST_TESOGAP:
            if(SEC_TIMER > rfdlybor_teso_timer) {
                RFDLYBOR_flag &= ~RFDLYBOR_TDONE_FLAG;
                RFDLYBOR_flag &= ~RFDLYBOR_TESO_FLAG;
                RFDLYBOR_flag &= ~RFDLYBOR_TESOGAP_FLAG;
                rfdlybornera_teso_state = BORRFDLY_ST_IDLE;
            }
            break;
        default:
            rfdlybornera_teso_state = BORRFDLY_ST_IDLE;
            break;
    }
}

//uint8_t         PTMSIGNAL_flag;

//time_t          ptmsignal_timer;
//uint8_t         ptmsignal_state;



void fsm_ptmsignalling( void )
{
    switch(ptmsignal_state) {
        case PTMSIGNAL_IDLE:
            if(PTMSIGNAL_flag & PTMSIG_PANIC)   {
                PTMSIGNAL_flag &= ~PTMSIG_PANIC;
                if(SysFlag_AP_Apertura & AP_APR_VALID)	{
                    rotu_autr_counter = 0;
                    rotu_autorst_timer_min = 2*60;
                    ptmsignal_timer = SEC_TIMER + 2*60;

                } else	{
                    if(paptslot == 0)	{
                        rotu_autr_counter = AUTORESET_POLL_COUNT;
                        rotu_autorst_timer_min = 6*60;
                        ptmsignal_timer = SEC_TIMER + 6*60;
                    } else	{
                        rotu_autr_counter = 0;
                        rotu_autorst_timer_min = 6*60;
                        ptmsignal_timer = SEC_TIMER + 6*60;
                    }
                }

                SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
                ptmsignal_state = PTMSIGNAL_PANIC_TRG;
            }
            break;
        case PTMSIGNAL_PANIC_TRG:
            if(SEC_TIMER > ptmsignal_timer) {
                rotu_state = AUTR_NORMAL;

                if( BaseAlarmPkt_alarm & bitpat[ROTU_bit] )	{
                    BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
                    AlarmWriteHistory();
                }
                BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
                rotu_autr_counter = 0;
                ptmsignal_state = PTMSIGNAL_IDLE;
            }
            break;
        default:
            ptmsignal_state = PTMSIGNAL_IDLE;
            PTMSIGNAL_flag = 0x00;
            break;
    }
}