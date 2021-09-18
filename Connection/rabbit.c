/*
 * rabbit.c
 *
 *  Created on: Jul 24, 2012
 *      Author: ironman
 */

#include "includes.h"

OS_TCB		RabbitTask_TCB;
CPU_STK		RabbitTask_Stk[RabbitTask_STK_SIZE];

uint8_t NumAbo = 17;
uint8_t NumCen = 13;
//char NomSuc[32] = {"EYSE Pruebas"};
//char Server1[16];
//char Server2[16];

//char LocalIP[16];
//char Netmask[16];
//char Gateway[16];


//int inUse[CENTRALOFFICEMAX] = { TRUE, TRUE };

//uint16_t hbaccount[CENTRALOFFICEMAX] = { 666, 777 };
//uint16_t sockPort[CENTRALOFFICEMAX] = { 6023, 7023};
//int sockProtocol[CENTRALOFFICEMAX] = { IPPROTO_UDP, IPPROTO_UDP};
//int sockType[CENTRALOFFICEMAX] = { SOCK_DGRAM, SOCK_DGRAM };
//const char *serverName[CENTRALOFFICEMAX] = {
//		Server1,
//		Server2
//};
//AlarmProtocols coProtocol[CENTRALOFFICEMAX] = { AP_NTSEC4, AP_NTSEC4 };
//int heartBeatTime[CENTRALOFFICEMAX] = { 60, 60  };


MonitorConnectionStruct Monitoreo[CENTRALOFFICEMAX];

void GetServerAdd( void )
{
    uint8_t e2p_data[16], df_data[16], temp[8];
    uint8_t e2p_chksum, df_chksum;
    int error, i, len;
    uint16_t chksum1, chksum2;
    OS_ERR os_err;
    NET_ERR      err;

    //------------------------------------------------------------------------
    //	Para el server1

    //leo la data de la data_flash
    flash0_read(df_data, DF_SVR1ADD_OFFSET, 16);
    flash0_read(temp, DF_SR1CHK_OFFSET, 1);
    df_chksum = temp[0];
    chksum1 = 0;
    for( i = 0; i < 16, df_data[i] != 0; i++)
        chksum1 += df_data[i];
    chksum1 &= 0x00FF;

    //leo la data de la e2prom
    EepromReadBuffer(SERVER1_E2P_ADDR, e2p_data, 16, &error); 
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
    e2p_chksum = EepromReadByte(SRV1ACHK_E2P_ADDR, &error);  
    chksum2 = 0;
    for( i = 0; i < 16, e2p_data[i] != 0; i++)
        chksum2 += e2p_data[i];
    chksum2 &= 0x00FF;

    if( chksum1 == chksum2) {
        if( chksum2 == df_chksum )    {
            //los checksum dieron todos bien !!!
            len = strlen(df_data);
			Str_Copy_N(Monitoreo[0].ServerName, df_data, len);
        } else
        if( chksum1 == df_chksum )   {
            len = strlen(df_data);
			Str_Copy_N(Monitoreo[0].ServerName, df_data, len);
        }
    } else {
        if( chksum1 == df_chksum )   {
            len = strlen(df_data);
			Str_Copy_N(Monitoreo[0].ServerName, df_data, len);
        }
        else
        if( chksum2 == e2p_chksum ) {
            len = strlen(e2p_data);
			Str_Copy_N(Monitoreo[0].ServerName, e2p_data, len);
        }
    }
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    //  Para el server 2

    //leo la data de la data_flash
    flash0_read(df_data, DF_SVR2ADD_OFFSET, 16);
    flash0_read(temp, DF_SR2CHK_OFFSET, 1);
    df_chksum = temp[0];
    chksum1 = 0;
    for( i = 0; i < 16, df_data[i] != 0; i++)
        chksum1 += df_data[i];
    chksum1 &= 0x00FF;

    //leo la data de la e2prom
    EepromReadBuffer(SERVER2_E2P_ADDR, e2p_data, 16, &error); 
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
    e2p_chksum = EepromReadByte(SRV2ACHK_E2P_ADDR, &error);  
    chksum2 = 0;
    for( i = 0; i < 16, e2p_data[i] != 0; i++)
        chksum2 += e2p_data[i];
    chksum2 &= 0x00FF;

    if( chksum1 == chksum2) {
        if( chksum2 == df_chksum )    {
            //los checksum dieron todos bien !!!
            len = strlen(df_data);
			Str_Copy_N(Monitoreo[1].ServerName, df_data, len);
        } else
        if( chksum1 == df_chksum )   {
            len = strlen(df_data);
			Str_Copy_N(Monitoreo[1].ServerName, df_data, len);
        }
    } else {
        if( chksum1 == df_chksum )   {
            len = strlen(df_data);
			Str_Copy_N(Monitoreo[1].ServerName, df_data, len);
        }
        else
        if( chksum2 == e2p_chksum ) {
            len = strlen(e2p_data);
			Str_Copy_N(Monitoreo[1].ServerName, e2p_data, len);
        }
    }
    //------------------------------------------------------------------------

    return;
}

void GetPortNumber(void)
{
	uint8_t port1buffer[4], port2buffer[4];
	uint16_t e2p_port1, e2p_port2, df_port1, df_port2;
	int error;

	e2p_port1 = EepromReadWord(PORT1_E2P_ADDR, &error);
	e2p_port2 = EepromReadWord(PORT2_E2P_ADDR, &error);

	flash0_read(port1buffer, DF_PORT1_OFFSET, 2);
	flash0_read(port2buffer, DF_PORT2_OFFSET, 2);

	df_port1 = port1buffer[0]*0x100 + port1buffer[1];
	df_port2 = port2buffer[0]*0x100 + port2buffer[1];

	if( e2p_port1 == df_port1 )	{
		Monitoreo[0].monport = e2p_port1;
	} else 	{
		Monitoreo[0].monport = df_port1;
	}
	if( e2p_port2 == df_port2 )	{
		Monitoreo[1].monport = e2p_port2;
	} else 	{
		Monitoreo[1].monport = df_port2;
	}
}


void InitMonitoreoStruct(void)
{
	int i, error, len;
	uint8_t retval[2];
	char buffer[16];

	numabo = NumAbo;
	numcen = NumCen;
	//Str_Copy(nomsuc, NomSuc);
	eyseAlarma = 0x81;
	eyseEstados = 0x00;
	eyseDisparos = 0x00;

    Monitoreo[0].inuse = EepromReadByte(INUSE1_E2P_ADDR, &error);
    Monitoreo[1].inuse = EepromReadByte(INUSE2_E2P_ADDR, &error);

	for( i = 0; i < CENTRALOFFICEMAX; i++)	{
		Monitoreo[i].state = SM_HOME;
		Monitoreo[i].timer = 0;
		//if( SystemFlag & (PORRESET_FLAG | RESETRESET_FLAG | BODRESET_FLAG))	{
			Monitoreo[i].eventRec_count = 0;
			Monitoreo[i].eventRec_readptr = 0;
			Monitoreo[i].eventRec_writeptr = 0;
		//}
		Monitoreo[i].retries = 0;
		Monitoreo[i].flags = 0x0000;
		Monitoreo[i].rxsm_state = RXSM_IDLE;
		Monitoreo[i].rxsm_flags = 0x0000;
		Monitoreo[i].rxsm_timer = 0;
		Monitoreo[i].reconnect = 0;
		if(Monitoreo[i].inuse == TRUE)
		    ReloadUnAckEvents(i);
	}
		//la siguiente inicializacion se debe cargar desde EEPROM de inicializacion
		Monitoreo[0].HBaccount = EepromReadWord(CIDACCOUNT1_E2P_ADDR, &error);
		Monitoreo[1].HBaccount = EepromReadWord(CIDACCOUNT2_E2P_ADDR, &error);


		GetPortNumber();
/*		Monitoreo[0].monport = EepromReadWord(PORT1_E2P_ADDR, &error);
		Monitoreo[1].monport = EepromReadWord(PORT2_E2P_ADDR, &error);*/

		Monitoreo[0].wdogstate = WR3K_IDLE;
		Monitoreo[1].wdogstate = WR3K_IDLE;

		retval[0] = EepromReadByte(PROTCOL1_E2P_ADDR, &error);
		retval[1] = EepromReadByte(PROTCOL2_E2P_ADDR, &error);
		for( i = 0; i < 2; i++)	{
			switch(retval[i])	{
				case 1:
					Monitoreo[i].protocol = AP_NTSEC4;
//					sockProtocol[i] = IPPROTO_UDP;
//					sockType[i] = SOCK_DGRAM;
					break;
				case 2:
					Monitoreo[i].protocol = AP_NTSEC5;
//					sockProtocol[i] = IPPROTO_UDP;
//					sockType[i] = SOCK_DGRAM;
					break;
				case 3:
					Monitoreo[i].protocol = AP_NTSEC6;
//					sockProtocol[i] = IPPROTO_TCP;
//					sockType[i] = SOCK_STREAM;
					break;
				case 4:
					Monitoreo[i].protocol = AP_NTSEC7;
//					sockProtocol[i] = IPPROTO_TCP;
//					sockType[i] = SOCK_STREAM;
					break;
				default:
					Monitoreo[i].protocol = AP_NTSEC4;
					break;
				}
		}


		GetServerAdd();

/*		EepromReadBuffer(SERVER1_E2P_ADDR, buffer, 16, &error);
		len = strlen(buffer);
		Str_Copy_N(Monitoreo[0].ServerName, buffer, len);
		EepromReadBuffer(SERVER2_E2P_ADDR, buffer, 16, &error);
		len = strlen(buffer);
		Str_Copy_N(Monitoreo[1].ServerName, buffer, len);*/


		Monitoreo[0].HeartBeatTime =  EepromReadWord(HBTTIME1_E2P_ADDR, &error);
		Monitoreo[1].HeartBeatTime =  EepromReadWord(HBTTIME2_E2P_ADDR, &error);
		if( Monitoreo[0].HeartBeatTime > 3600)	{
			Monitoreo[0].HeartBeatTime = 90;
		}
		if( Monitoreo[1].HeartBeatTime > 3600)	{
			Monitoreo[1].HeartBeatTime = 90;
		}

//		Monitoreo[0].inuse = EepromReadByte(INUSE1_E2P_ADDR, &error);
//		Monitoreo[1].inuse = EepromReadByte(INUSE2_E2P_ADDR, &error);

		Monitoreo[0].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[0].HeartBeatTime);
		Monitoreo[1].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[1].HeartBeatTime);
//		Monitoreo[0].flags &= ~ACKWDG_FLAG;
//		Monitoreo[1].flags &= ~ACKWDG_FLAG;
		Monitoreo[0].flags |= ACKWDG_FLAG;
		Monitoreo[1].flags |= ACKWDG_FLAG;
		Monitoreo[0].sec = 1;
		Monitoreo[1].sec = 1;
}


void  RabbitTask(void  *p_arg)
{
	OS_ERR	os_err;
	NET_ERR err;

	EventRecord thisEvent;
	uint8_t	sendbuffer[128];
	uint16_t buf_len;
	uint8_t	recvbuffer[64];
	uint8_t temp[16];
	NET_SOCK_RTN_CODE	retval;
	NET_SOCK_ADDR_LEN ipsize;
	CPU_INT32U taskinterval;

	time_t linktesttimer;

	int monid;
	uint8_t aboactual[3];
	int i;




	(void)p_arg;

	
	OSTimeDlyHMSM(0, 0, TIME_STARTUP_R3K, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//InitMonitoreoStruct();
	linktesttimer = SEC_TIMER;
	taskinterval = 1000;

	if(valid_license())    {
	    SystemFlag10 |= UDPLICOK_FLAG;
	} else  {
	    SystemFlag10 &= ~UDPLICOK_FLAG;
	}

	while (DEF_ON) {
		WDT_Feed();
		OSTimeDlyHMSM(0, 0, 0, taskinterval, OS_OPT_TIME_HMSM_STRICT, &os_err);

		if(!(SystemFlag5 & SERIALNUM_OK))	{
			flash0_read(temp, DF_SERIALNUM_OFFSET, 8);
			for(i = 0; i < 8; i++)  {               //detecto que no sea igual a cero
        		if( temp[i] != '0' )  {
            		SystemFlag5 |= SERIALNUM_OK;
        		}
    		}
    		for(i = 0; i < 8; i++)  {               //detecto si todos los digitos son numeros validos
        		if((temp[i] < '0') || (temp[i] > '9'))
            		SystemFlag5 &= ~SERIALNUM_OK;
    		}

    		flash0_read(temp, DF_BLOCKCODE_OFFSET, 1);
    		if(temp[0] == 0x5A) {
        		SystemFlag5 |= SERIALNUM_OK;
    		}

    		if(!(SystemFlag5 & SERIALNUM_OK))
				continue;
		}

		if(SEC_TIMER >= (linktesttimer + 5))	{
			linktesttimer = SEC_TIMER;
			//-------------------------------------------------------------------
			//Chequeo el estado de la red para ver si es necesario reinicializar
			retval = NetNIC_PhyLinkState();
			switch( ethlink_state )	{
			case ETHLNK_CONNECTED :
				if(retval == DEF_NO)	{
					ethlink_state = ETHLNK_DISCONNECTED;
					for( monid = 0; monid < CENTRALOFFICEMAX; monid++) {
						if(Monitoreo[monid].inuse == FALSE)
							continue;
						NetSock_Close(Monitoreo[monid].monsock, &err);
						Monitoreo[monid].state = SM_HOME;
					}
				}
				break;
			case ETHLNK_DISCONNECTED :
				if(retval == DEF_YES)	{
					ethlink_state = ETHLNK_CONNECTED;
					NetNIC_Init(&err);

				}
				break;
			}
			//-------------------------------------------------------------------
		}


		if( ethlink_state == ETHLNK_CONNECTED )	{
		for( monid = 0; monid < CENTRALOFFICEMAX; monid++) {
			if(Monitoreo[monid].inuse == FALSE)
				continue;
			//fsm_wdog_r3k(monid);
			switch(Monitoreo[monid].state)	{
			//********************************************************************************************************************
			//* Estableciendo la conexion
			case SM_HOME:
				taskinterval = 5000;
				switch(Monitoreo[monid].protocol)	{
				case AP_NTSEC4:
				case AP_NTSEC5:
				case AP_NTSEC6:
				case AP_NTSEC7:
					//------------------------------------------------------------------------------------------------------------
					//conexion por UDP
					Monitoreo[monid].monsock = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4, NET_SOCK_TYPE_DATAGRAM, NET_SOCK_PROTOCOL_UDP, &err);

					if (err != NET_SOCK_ERR_NONE) {
						switch(err)	{
						case NET_SOCK_ERR_NONE_AVAIL:
							NetSock_Close(Monitoreo[0].monsock, &err);
							NetSock_Close(Monitoreo[1].monsock, &err);
							OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
							ethlink_state = ETHLNK_CONNECTED;
							NetNIC_Init(&err);
							OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
							//InitMonitoreoStruct();
							break;
						}
						break;
					}	else 	{
						Monitoreo[monid].server_ip_addr = NetASCII_Str_to_IP(Monitoreo[monid].ServerName, &err);
						if( err != NET_ASCII_ERR_NONE)	{
							//NetSock_Close(Monitoreo[monid].monsock, &err);
							Monitoreo[monid].state = SM_WAIT_CLOSE;
							Monitoreo[monid].timer = SEC_TIMER;
							break;
						}

						Monitoreo[monid].server_sock_addr_ip_size = sizeof(Monitoreo[monid].server_sock_addr_ip);
						Mem_Clr((void *)&(Monitoreo[monid].server_sock_addr_ip), (CPU_SIZE_T) Monitoreo[monid].server_sock_addr_ip_size);
						Monitoreo[monid].server_sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
						Monitoreo[monid].server_sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(Monitoreo[monid].server_ip_addr);
						Monitoreo[monid].server_sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(Monitoreo[monid].monport);

						Monitoreo[monid].timer = SEC_TIMER - Monitoreo[monid].HeartBeatTime;
						Monitoreo[monid].state = SM_SOCK_READY;
						Monitoreo[monid].flags |= SNDHBT_FLAG;
						//ReloadUnAckEvents(monid);
						
					}
					//--------------------------------------------------------------------------------------------------------------
					break;
				}
			//********************************************************************************************************************
			//* Ya estamos conectados
			case SM_SOCK_READY:
				taskinterval = 250;     //2000
				if((Monitoreo[monid].eventRec_count) && (!(Monitoreo[monid].flags & SNDHBT_FLAG)) && (SystemFlag10 & UDPLICOK_FLAG))	{
					//-----------------------------------------------
					retval = 1;
					while(retval)	{
						retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
													(void *)				recvbuffer,
													(CPU_INT16S)			64,
													(CPU_INT16S)			NET_SOCK_FLAG_NONE,
													(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
													(NET_SOCK_ADDR_LEN *)	&ipsize,
													(void *)				0,
													(CPU_INT08U)			0,
													(CPU_INT08U *)			0,
													(NET_ERR *)				&err );
						if(retval < 0)	{
							break;
						}
					}
					retval = 0;
					OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
					//-----------------------------------------------
					preReadEvent(monid, &thisEvent);

					//--------------------------------------------------------------
					// inserto el envio del paquete encriptado
/*					buf_len = ProtocolEncoder(monid, Monitoreo[monid].HBaccount, AP_NTSEC6E, &thisEvent, sendbuffer);
					retval = NetSock_TxDataTo(	(NET_SOCK_ID)			Monitoreo[monid].monsock,
							(void *)				sendbuffer,
							(CPU_INT16S)			buf_len,
							(CPU_INT16S)			NET_SOCK_FLAG_NONE,
							(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
							(NET_SOCK_ADDR_LEN)		sizeof(Monitoreo[monid].server_sock_addr_ip),
							(NET_ERR *)				&err );
					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);*/
					//--------------------------------------------------------------
					
					buf_len = ProtocolEncoder(monid, Monitoreo[monid].HBaccount, Monitoreo[monid].protocol, &thisEvent, sendbuffer);

					retval = NetSock_TxDataTo(	(NET_SOCK_ID)			Monitoreo[monid].monsock,
												(void *)				sendbuffer,
												(CPU_INT16S)			buf_len,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
												(NET_SOCK_ADDR_LEN)		sizeof(Monitoreo[monid].server_sock_addr_ip),
												(NET_ERR *)				&err );
					if( err != NET_SOCK_ERR_NONE)	{
						//NetSock_Close(Monitoreo[monid].monsock, &err);
						Monitoreo[monid].state = SM_WAIT_CLOSE;
						Monitoreo[monid].timer = SEC_TIMER;
						break;
					}
					Monitoreo[monid].retries = 1;
					Monitoreo[monid].state = SM_SOCK_WAITACK;
					Monitoreo[monid].flags |= EVESND_FLAG;
					Monitoreo[monid].timer = SEC_TIMER;
					aboactual[0] = sendbuffer[4];
					aboactual[1] = sendbuffer[13];
					aboactual[2] = sendbuffer[14];

					ScreenLedBlink(8, (7 - monid), 7, 3, 1);
					//ScreenLedBlink(8, 7, 3, 3, 1);
					if(OptoInputs == 0x81)	{
						Buzzer_dcb.led_cad = 2*0x100 + 255;
						Buzzer_dcb.led_state = LED_IDLE;
						Buzzer_dcb.led_blink = 1;
					}

				} else
				    if(SEC_TIMER >= (Monitoreo[monid].timer + Monitoreo[monid].HeartBeatTime))	{
					//-----------------------------------------------
					retval = 1;
					while(retval)	{
						retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
													(void *)				recvbuffer,
													(CPU_INT16S)			64,
													(CPU_INT16S)			NET_SOCK_FLAG_NONE,
													(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
													(NET_SOCK_ADDR_LEN *)	&ipsize,
													(void *)				0,
													(CPU_INT08U)			0,
													(CPU_INT08U *)			0,
													(NET_ERR *)				&err );
						if(retval < 0)	{
							break;
						}
					}
					retval = 0;
					OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
					//-----------------------------------------------
					buf_len = heartbeat(monid, sendbuffer);
					retval = NetSock_TxDataTo(	(NET_SOCK_ID)			Monitoreo[monid].monsock,
												(void *)				sendbuffer,
												(CPU_INT16S)			buf_len,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
												(NET_SOCK_ADDR_LEN)		sizeof(Monitoreo[monid].server_sock_addr_ip),
												(NET_ERR *)				&err
							);
					if( err != NET_SOCK_ERR_NONE)	{
						//NetSock_Close(Monitoreo[monid].monsock, &err);
						Monitoreo[monid].state = SM_WAIT_CLOSE;
						Monitoreo[monid].timer = SEC_TIMER;
						break;
					}
					Monitoreo[monid].retries = 1;
					Monitoreo[monid].state = SM_SOCK_WAITACK;
					Monitoreo[monid].flags |= HBTSND_FLAG;
					Monitoreo[monid].timer = SEC_TIMER;

					ScreenLedBlink(8, (7 - monid), 7, 3, 1);
					//ScreenLedBlink(8, 7, 3, 3, 1);
					if(OptoInputs == 0x81)	{
						Buzzer_dcb.led_cad = 2*0x100 + 255;
						Buzzer_dcb.led_state = LED_IDLE;
						Buzzer_dcb.led_blink = 1;
					}
				}
				break;
			//********************************************************************************************************************
			//* Ahora esperamos el ACK
			case SM_SOCK_WAITACK:
				taskinterval = 250;    //2000
				// veo si recibi algo
				ipsize = sizeof(Monitoreo[monid].server_sock_addr_ip);
				retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
											(void *)				recvbuffer,
											(CPU_INT16S)			64,
											(CPU_INT16S)			MSG_PEEK,
											(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
											(NET_SOCK_ADDR_LEN *)	&ipsize,
											(void *)				0,
											(CPU_INT08U)			0,
											(CPU_INT08U *)			0,
											(NET_ERR *)				&err );
				if( (retval == 2) && ((Monitoreo[monid].protocol == AP_NTSEC4) || (Monitoreo[monid].protocol == AP_NTSEC5) || (Monitoreo[monid].protocol == AP_NTSEC6) || (Monitoreo[monid].protocol == AP_NTSEC7)))	{
					retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
												(void *)				recvbuffer,
												(CPU_INT16S)			64,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
												(NET_SOCK_ADDR_LEN *)	&ipsize,
												(void *)				0,
												(CPU_INT08U)			0,
												(CPU_INT08U *)			0,
												(NET_ERR *)				&err );
					if(recvbuffer[0] == '@')	{
						Monitoreo[monid].flags |= ACKRXD_FLAG;
						Monitoreo[monid].flags |= ACKWDG_FLAG;
						ScreenLedBlink(8, (1 - monid), 7, 3, 1);
						//ScreenLedBlink(8, 1, 3, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}

						if((Monitoreo[monid].protocol == AP_NTSEC5) || (Monitoreo[monid].protocol == AP_NTSEC6))	{
							if(Monitoreo[monid].sec == 0xFF)	{
								Monitoreo[monid].sec = 0x01;
							} else	{
								Monitoreo[monid].sec++;
							}
						}
					}
				} else
				//-----------------------------------------------------------------------------------
				if( (retval == 5) && ((Monitoreo[monid].protocol == AP_NTSEC6) || (Monitoreo[monid].protocol == AP_NTSEC7)) )	{
					retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
												(void *)				recvbuffer,
												(CPU_INT16S)			64,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
												(NET_SOCK_ADDR_LEN *)	&ipsize,
												(void *)				0,
												(CPU_INT08U)			0,
												(CPU_INT08U *)			0,
												(NET_ERR *)				&err );
					if((recvbuffer[0] == '@') && (recvbuffer[2] == 0xC1) )	{
						Monitoreo[monid].flags |= ACKWDG_FLAG;
						ScreenLedBlink(8, (1 - monid), 7, 3, 1);
						//ScreenLedBlink(8, 1, 3, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}
						if(Monitoreo[monid].sec == recvbuffer[3])	{
							Monitoreo[monid].flags |= ACKRXD_FLAG;
							if(Monitoreo[monid].sec == 0xFF)	{
								Monitoreo[monid].sec = 0x01;
							} else	{
								Monitoreo[monid].sec++;
							}
						}
					}
				} else
				if( (retval == 19) && ((Monitoreo[monid].protocol == AP_NTSEC6) || (Monitoreo[monid].protocol == AP_NTSEC7)) )	{
					retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
												(void *)				recvbuffer,
												(CPU_INT16S)			64,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
												(NET_SOCK_ADDR_LEN *)	&ipsize,
												(void *)				0,
												(CPU_INT08U)			0,
												(CPU_INT08U *)			0,
												(NET_ERR *)				&err );
					if((recvbuffer[0] == '@') && (recvbuffer[2] == 0xCF) )	{
						Monitoreo[monid].flags |= ACKWDG_FLAG;
						ScreenLedBlink(8, (1 - monid), 7, 3, 1);
						//ScreenLedBlink(8, 1, 3, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}
						if(Monitoreo[monid].sec == recvbuffer[3])	{
							Monitoreo[monid].flags |= ACKRXD_FLAG;
							if(Monitoreo[monid].sec == 0xFF)	{
								Monitoreo[monid].sec = 0x01;
							} else	{
								Monitoreo[monid].sec++;
							}
							//ack_syncronization(&(recvbuffer[4]));
						}
					}
				} else
				//-----------------------------------------------------------------------------------
				if( ((retval == 3) || (retval == 5)) && (Monitoreo[monid].protocol == AP_EYSE1) )	{
					retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
												(void *)				recvbuffer,
												(CPU_INT16S)			64,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
												(NET_SOCK_ADDR_LEN *)	&ipsize,
												(void *)				0,
												(CPU_INT08U)			0,
												(CPU_INT08U *)			0,
												(NET_ERR *)				&err );
					if((recvbuffer[0] == 'A') && (recvbuffer[1] == 'C') && (recvbuffer[2] == 'K'))	{
						Monitoreo[monid].flags |= ACKRXD_FLAG;
						Monitoreo[monid].flags |= ACKWDG_FLAG;
						ScreenLedBlink(8, (1 - monid), 7, 3, 1);
						//ScreenLedBlink(8, 1, 3, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}
					}
				} else
				if( retval > 5)	{
					while( retval > 0)	{
						retval = NetSock_RxDataFrom((NET_SOCK_ID)			Monitoreo[monid].monsock,
													(void *)				recvbuffer,
													(CPU_INT16S)			64,
													(CPU_INT16S)			NET_SOCK_FLAG_NONE,
													(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
													(NET_SOCK_ADDR_LEN *)	&ipsize,
													(void *)				0,
													(CPU_INT08U)			0,
													(CPU_INT08U *)			0,
													(NET_ERR *)				&err );
					}
				}

				//veamos si tengo que reintentar
				if(Monitoreo[monid].flags & ACKRXD_FLAG)	{
					Monitoreo[monid].flags &= ~ACKRXD_FLAG;
					if(Monitoreo[monid].flags & EVESND_FLAG)	{
						Monitoreo[monid].flags &= ~EVESND_FLAG;
						ReadOutEvent(monid, &thisEvent);
					} else
					if(Monitoreo[monid].flags & HBTSND_FLAG)	{
						Monitoreo[monid].flags &= ~HBTSND_FLAG;
                        //------------------------------- 20200904 pruebo reenvio de eventos sin ack
                        if(Monitoreo[monid].eventRec_count == 0)    {
                            ReloadUnAckEvents25( monid);
                        }
                        //--------------------------------------------------------------------------
					} else
					if(Monitoreo[monid].flags & EYSEID_FLAG)	{
						Monitoreo[monid].flags &= ~EYSEID_FLAG;
					}
					Monitoreo[monid].state = SM_SOCK_READY;
					Monitoreo[monid].flags &= ~SNDHBT_FLAG;

				} else
				if(SEC_TIMER >= (Monitoreo[monid].timer + SEND_RETRIES_TIME))	{
					Monitoreo[monid].retries++;
					Monitoreo[monid].timer = SEC_TIMER;
					if(Monitoreo[monid].flags & EVESND_FLAG)	{
						preReadEvent(monid, &thisEvent);
						buf_len = ProtocolEncoder(monid, Monitoreo[monid].HBaccount, Monitoreo[monid].protocol, &thisEvent, sendbuffer);
						retval = NetSock_TxDataTo(	(NET_SOCK_ID)			Monitoreo[monid].monsock,
													(void *)				sendbuffer,
													(CPU_INT16S)			buf_len,
													(CPU_INT16S)			NET_SOCK_FLAG_NONE,
													(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
													(NET_SOCK_ADDR_LEN)		sizeof(Monitoreo[monid].server_sock_addr_ip),
													(NET_ERR *)				&err );
						
						if( err != NET_SOCK_ERR_NONE)	{
							Monitoreo[monid].state = SM_WAIT_CLOSE;
							Monitoreo[monid].timer = SEC_TIMER;
							break;
						}

						ScreenLedBlink(8, (7 - monid), 7, 3, 1);
						//ScreenLedBlink(8, 7, 3, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}
					} else
					if(Monitoreo[monid].flags & HBTSND_FLAG)	{
						buf_len = heartbeat(monid, sendbuffer);
						retval = NetSock_TxDataTo(	(NET_SOCK_ID)			Monitoreo[monid].monsock,
													(void *)				sendbuffer,
													(CPU_INT16S)			buf_len,
													(CPU_INT16S)			NET_SOCK_FLAG_NONE,
													(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
													(NET_SOCK_ADDR_LEN)		sizeof(Monitoreo[monid].server_sock_addr_ip),
													(NET_ERR *)				&err );
						
						if( err != NET_SOCK_ERR_NONE)	{
							Monitoreo[monid].state = SM_WAIT_CLOSE;
							Monitoreo[monid].timer = SEC_TIMER;
							break;
						}
						ScreenLedBlink(8, (7 - monid), 7, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}
					} else
					if(Monitoreo[monid].flags & EYSEID_FLAG)	{
						buf_len = sendEyseId(sendbuffer);
						retval = NetSock_TxDataTo(	(NET_SOCK_ID)			Monitoreo[monid].monsock,
													(void *)				sendbuffer,
													(CPU_INT16S)			buf_len,
													(CPU_INT16S)			NET_SOCK_FLAG_NONE,
													(NET_SOCK_ADDR *)		&(Monitoreo[monid].server_sock_addr_ip),
													(NET_SOCK_ADDR_LEN)		sizeof(Monitoreo[monid].server_sock_addr_ip),
													(NET_ERR *)				&err );
						if( err != NET_SOCK_ERR_NONE)	{
							//NetSock_Close(Monitoreo[monid].monsock, &err);
							Monitoreo[monid].state = SM_WAIT_CLOSE;
							Monitoreo[monid].timer = SEC_TIMER;
							break;
						}
						ScreenLedBlink(8, (7 - monid), 7, 3, 1);
						//ScreenLedBlink(8, 7, 3, 3, 1);
						if(OptoInputs == 0x81)	{
							Buzzer_dcb.led_cad = 2*0x100 + 255;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 1;
						}
					} else
					if(Monitoreo[monid].retries > 5)	{
						//NetSock_Close(Monitoreo[monid].monsock, &err);
						Monitoreo[monid].state = SM_WAIT_CLOSE;
						Monitoreo[monid].timer = SEC_TIMER;
						break;
					}
				}
				break;
			case SM_WAIT_CLOSE:
				NetSock_Close(Monitoreo[monid].monsock, &err);
				OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
				switch(err)	{
				case NET_SOCK_ERR_NONE:
					Monitoreo[monid].state = SM_HOME;
					Monitoreo[monid].timer = SEC_TIMER;
					break;
				case NET_SOCK_ERR_NOT_USED:                   //Socket NOT currently used.
				case NET_SOCK_ERR_CLOSED:                     //Socket already closed.
				case NET_SOCK_ERR_INVALID_STATE:              //Invalid socket state.
					Monitoreo[monid].state = SM_HOME;
					Monitoreo[monid].timer = SEC_TIMER;
					break;
				case NET_SOCK_ERR_CONN_CLOSE_IN_PROGRESS:
					break;
				default:
					Monitoreo[monid].state = SM_HOME;
					Monitoreo[monid].timer = SEC_TIMER;
					break;
				}
				break;
			default:
				break;
			}
		}
		}
	}
}

void ack_syncronization( char * ascii_tstamp)
{
	struct tm currtime;
	int tempint, i;
	time_t sectimer;
	char temp[12];

	//convierto a formato timestamp de eventos
	for( i = 0; i < 4; i++)
		temp[i] = ascii_tstamp[i+10];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_year = tempint - YEAR0;

	for( i = 0; i < 2; i++)
		temp[i] = ascii_tstamp[i+6];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mon = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = ascii_tstamp[i+8];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_mday = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = ascii_tstamp[i];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_hour = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = ascii_tstamp[i+2];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_min = tempint;

	for( i = 0; i < 2; i++)
		temp[i] = ascii_tstamp[i+4];
	temp[i] = '\0';
	tempint = atoi(temp);
	currtime.tm_sec = tempint;


	currtime.tm_mon -= 1;
	sectimer = mktime(&currtime);
	fsm_wdog_evo( 99, 0 );

	if( (sectimer < (SEC_TIMER - 10)) || (sectimer > (SEC_TIMER + 10)) )	{

		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, currtime.tm_sec);
		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, currtime.tm_min);
		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, currtime.tm_hour);
		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, currtime.tm_mon + 1);
		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, currtime.tm_year);
		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, currtime.tm_mday);
		RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK, currtime.tm_wday);

		SEC_TIMER = sectimer;
	}
}

int heartbeat( int coid, uint8_t *buffer)
{
	int retval;

    //retval = ReloadUnAckEvents25( coid);
    retval = 0;
    if( retval == 0) {
        switch (Monitoreo[coid].protocol) {
            case AP_EYSE1:
                retval = heartbeat_EYSE1(buffer);
                break;
            case AP_NTSEC4:
                retval = heartbeat_NTSEC4(Monitoreo[coid].HBaccount, buffer);
                break;
            case AP_NTSEC5:
                retval = heartbeat_NTSEC5(Monitoreo[coid].HBaccount, buffer, Monitoreo[coid].sec);
                break;
            case AP_NTSEC6:
                retval = heartbeat_NTSEC6(Monitoreo[coid].HBaccount, buffer, Monitoreo[coid].sec);
                break;
            case AP_NTSEC7:
                retval = heartbeat_NTSEC7(Monitoreo[coid].HBaccount, buffer, Monitoreo[coid].sec);
                break;
        }
    }

	return retval;
}

uint8_t hbreset_retries;

//void fsm_wdog_r3k(int coid)
//{
//	NET_ERR err;
//	struct tm currtime;
//	time_t temp;
//	int error;
//	uint8_t buffer[8];
//
//	switch(Monitoreo[coid].wdogstate)	{
//	case WR3K_IDLE:
//		if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
//			Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//			Monitoreo[coid].wdogstate = WR3K_WDOG;
//			Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//		} else 	{
//			Monitoreo[coid].wdogr3kTimer = SEC_TIMER;
//			Monitoreo[coid].wdogstate = WR3K_WDOG;
//			Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//		}
//		break;
//	case WR3K_WDOG:
//		if( SEC_TIMER > Monitoreo[coid].wdogr3kTimer )	{
//			NetSock_Close(Monitoreo[0].monsock, &err);
//			NetSock_Close(Monitoreo[1].monsock, &err);
//			OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
//
//			ethlink_state = ETHLNK_CONNECTED;
//			NetNIC_Init(&err);
//			OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
//
//			InitMonitoreoStruct();
//			Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (30*60);
//			Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//			Monitoreo[coid].wdogstate = WR3K_WRST;
//#ifdef USAR_IRIDIUM
//			IRIDIUM_flag |= IRI_IPNG_FLAG;
//#endif
//			if(coid == 0)	{
//				Monitoreo[0].flags |= E700_1_FLAG;
//			}
//			else if( coid == 1)	{
//				Monitoreo[1].flags |= E700_2_FLAG;
//			}
//		} else
//		if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
//			Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//			Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//		}
//		break;
//	case WR3K_WRST:
//		if(Monitoreo[coid].wdogr3kTimer > SEC_TIMER)	{
//			temp = Monitoreo[coid].wdogr3kTimer - SEC_TIMER;
//			if(temp > 30*60)
//				Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (30*60);
//		}
//		if( SEC_TIMER > Monitoreo[coid].wdogr3kTimer )	{
//			if((SystemFlag4 & ARSTOK_FLAG) && (hbreset_retries <= HBRESET_RETRIES) )	{
//				NetSock_Close(Monitoreo[0].monsock, &err);
//				NetSock_Close(Monitoreo[1].monsock, &err);
//				LLAVE_TX_OFF();
//				POWER_TX_OFF();
//				hbreset_retries++;
//				buffer[0] = hbreset_retries;
//				error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
//				OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
//				while(1);	//me reseteo por watchdog
//			} else	if(hbreset_retries > HBRESET_RETRIES)  {
//				/**Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//				Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//				Monitoreo[coid].wdogstate = WR3K_WDOG;**/
//                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + 60*60;
//                Monitoreo[coid].wdogstate = WR3K_WAITONEHOUR;
//            }
//
//		} else
//		if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
//			Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//			Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//			Monitoreo[coid].wdogstate = WR3K_WDOG;
//#ifdef USAR_IRIDIUM
//			IRIDIUM_flag &= ~IRI_IPNG_FLAG;
//#endif
//			if(coid == 0)	{
//				Monitoreo[0].flags &= ~E700_1_FLAG;
//			}
//			else if( coid == 1)	{
//				Monitoreo[0].flags &= ~E700_2_FLAG;
//			}
//		}
//		break;
//    case WR3K_WAITONEHOUR:
//        if(Monitoreo[coid].wdogr3kTimer > SEC_TIMER)	{
//            temp = Monitoreo[coid].wdogr3kTimer - SEC_TIMER;
//            if(temp > 60*60)
//                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (60*60);
//        }
//        if( SEC_TIMER > Monitoreo[coid].wdogr3kTimer )  {
//            hbreset_retries = 0;
//            Monitoreo[coid].wdogstate = WR3K_WDOG;
//            Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//
//            NetSock_Close(Monitoreo[0].monsock, &err);
//            NetSock_Close(Monitoreo[1].monsock, &err);
//            LLAVE_TX_OFF();
//            POWER_TX_OFF();
//            buffer[0] = 0;
//            error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
//            OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
//            while(1);	//me reseteo por watchdog
//        } else
//        if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
//            Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//            Monitoreo[coid].flags &= ~ACKWDG_FLAG;
//            Monitoreo[coid].wdogstate = WR3K_WDOG;
//#ifdef USAR_IRIDIUM
//            IRIDIUM_flag &= ~IRI_IPNG_FLAG;
//#endif
//            if(coid == 0)	{
//                Monitoreo[0].flags &= ~E700_1_FLAG;
//            }
//            else if( coid == 1)	{
//                Monitoreo[0].flags &= ~E700_2_FLAG;
//            }
//            hbreset_retries = 0;
//            buffer[0] = hbreset_retries;
//            error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
//            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
//        }
//        break;
//	default:
//		Monitoreo[coid].wdogstate = WR3K_WDOG;
//		Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
//		break;
//	}
//}


void fsm_wdog_r3k(int coid)
{
    NET_ERR err;
    struct tm currtime;
    time_t temp;
    int error;
    uint8_t buffer[8];

    switch(Monitoreo[coid].wdogstate)	{
        case WR3K_IDLE:
            if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
                Monitoreo[coid].wdogstate = WR3K_WDOG;
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
            } else 	{
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER;
                Monitoreo[coid].wdogstate = WR3K_WDOG;
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
            }
            break;
        case WR3K_WDOG:
            if( SEC_TIMER > Monitoreo[coid].wdogr3kTimer )	{
                NetSock_Close(Monitoreo[0].monsock, &err);
                NetSock_Close(Monitoreo[1].monsock, &err);
                OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);

                ethlink_state = ETHLNK_CONNECTED;
                NetNIC_Init(&err);
                OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);

                InitMonitoreoStruct();
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (30*60);
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;

                Monitoreo[coid].wdogstate = WR3K_WRST;

#ifdef USAR_IRIDIUM
                IRIDIUM_flag |= IRI_IPNG_FLAG;
#endif
                if(coid == 0)	{
                    Monitoreo[0].flags |= E700_1_FLAG;
                }
                else if( coid == 1)	{
                    Monitoreo[1].flags |= E700_2_FLAG;
                }
            } else
            if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
            }
            break;
        case WR3K_WRST:
            if(Monitoreo[coid].wdogr3kTimer > SEC_TIMER)	{
                temp = Monitoreo[coid].wdogr3kTimer - SEC_TIMER;
                if(temp > 30*60)
                    Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (30*60);
            }
            if( SEC_TIMER > Monitoreo[coid].wdogr3kTimer )	{
                if((SystemFlag4 & ARSTOK_FLAG) && (hbreset_retries < HBRESET_RETRIES) )	{
                    NetSock_Close(Monitoreo[0].monsock, &err);
                    NetSock_Close(Monitoreo[1].monsock, &err);
                    LLAVE_TX_OFF();
                    POWER_TX_OFF();
                    hbreset_retries++;
                    buffer[0] = hbreset_retries;
                    error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
                    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
                    while(1);	//me reseteo por watchdog
                } else	if(hbreset_retries >= HBRESET_RETRIES)  {
                    Monitoreo[coid].wdogr3kTimer = SEC_TIMER + SEC_TIMER + 60*60;
                    Monitoreo[coid].flags &= ~ACKWDG_FLAG;
                    Monitoreo[coid].wdogstate = WR3K_WAITONEHOUR;
                }

            } else
            if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
                Monitoreo[coid].wdogstate = WR3K_WDOG;
#ifdef USAR_IRIDIUM
                IRIDIUM_flag &= ~IRI_IPNG_FLAG;
#endif
                if(coid == 0)	{
                    Monitoreo[0].flags &= ~E700_1_FLAG;
                }
                else if( coid == 1)	{
                    Monitoreo[0].flags &= ~E700_2_FLAG;
                }
            } else
            if( DebugFlag & NETRSTHAB_flag) {
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
                Monitoreo[coid].wdogstate = WR3K_WDOG;
            }
            break;
        case WR3K_WAITONEHOUR:
            if(Monitoreo[coid].wdogr3kTimer > SEC_TIMER)	{
                temp = Monitoreo[coid].wdogr3kTimer - SEC_TIMER;
                if(temp > 60*60)
                    Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (60*60);
            }
            if( SEC_TIMER > Monitoreo[coid].wdogr3kTimer )  {
                hbreset_retries = 0;
                Monitoreo[coid].wdogstate = WR3K_WDOG;
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);

                NetSock_Close(Monitoreo[0].monsock, &err);
                NetSock_Close(Monitoreo[1].monsock, &err);
                LLAVE_TX_OFF();
                POWER_TX_OFF();
                buffer[0] = 0;
                error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
                OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
                while(1);	//me reseteo por watchdog
            } else
            if( Monitoreo[coid].flags & ACKWDG_FLAG )	{
                Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
                Monitoreo[coid].flags &= ~ACKWDG_FLAG;
                Monitoreo[coid].wdogstate = WR3K_WDOG;
#ifdef USAR_IRIDIUM
                IRIDIUM_flag &= ~IRI_IPNG_FLAG;
#endif
                if(coid == 0)	{
                    Monitoreo[0].flags &= ~E700_1_FLAG;
                }
                else if( coid == 1)	{
                    Monitoreo[0].flags &= ~E700_2_FLAG;
                }
                hbreset_retries = 0;
                buffer[0] = hbreset_retries;
                error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
                OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
            }
            break;
        default:
            Monitoreo[coid].wdogstate = WR3K_WDOG;
            Monitoreo[coid].wdogr3kTimer = SEC_TIMER + (5 * Monitoreo[coid].HeartBeatTime);
            break;
    }
}
