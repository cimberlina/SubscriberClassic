/*
*********************************************************************************************************
*											Telnet Console
*									Telnet Command Line Interface
*							(c) Copyright 2011, INDAV Ingenieria de Avanzada
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*									Implementacion del modulo
* Filename		:	telnet_server.c
* Version		:	V1.00
* Programmer(s)	:	CMI
*********************************************************************************************************
*/

/*
*********************************************************************************************************
* 											INCLUDE FILES
*********************************************************************************************************
*/
#include "includes.h"

/*
*********************************************************************************************************
* 											GLOBAL VARIABLES
*********************************************************************************************************
*/

const ConsoleIO TLN_console_io = { 23,
								   NULL,
								   NULL,
								   NULL,
								   conio_telnet_puts,
								   NULL,
								   NULL,
								   NULL,
								   conio_telnet_read,
								   conio_telnet_write };

NET_SOCK_ID tln_sock;

TELNET_ConioTCB tlnConioTcb;

unsigned char tn_receive_buf[TLN_RCV_BUF_SIZE];

/*
*********************************************************************************************************
* 												TASK related
*********************************************************************************************************
*/
OS_TCB		TelnetServerTask_TCB;
CPU_STK		TelnetServerTask_Stk[TelnetServerTask_STK_SIZE];

/*
*********************************************************************************************************
*********************************************************************************************************
* 											LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
void tln_inputchar( unsigned char data)
{
	NET_ERR err;

	if ((data == 0x08) || (data == 0x7f)) {
		if((tlnConioTcb.cmdptr > tlnConioTcb.command))	{
			tlnConioTcb.cmdptr--;
			if( (tlnConioTcb.local_echo == 1)||(tlnConioTcb.echo == 1))
				NetSock_TxData( tln_sock, " \x08", 2, NET_SOCK_FLAG_NONE, &err);
		}
	} else {
		if ((tlnConioTcb.cmdptr - tlnConioTcb.command) < TLN_CMD_SIZE) {
			*(tlnConioTcb.cmdptr) = data;
		}
		tlnConioTcb.cmdptr++;
	}
}



/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
void tln_build_reply( unsigned char command, unsigned char option )
{
	unsigned char buf[3];
	NET_ERR   err;

	buf[0] = 255;
	buf[1] = command;
	buf[2] = option;
	NetSock_TxData( tln_sock, buf, 3, NET_SOCK_FLAG_NONE, &err);
}



/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
void dooption( unsigned char opt )
{
	unsigned char cmd = 0;
	
	switch( opt )	{
		case T_ECHO:
			tlnConioTcb.local_echo = 1;
			cmd = WILL;
			break;
		case T_SGA:
			tlnConioTcb.suppress_ga = 1;
			cmd = WILL;
			break;
		case T_TM:
		default:
			cmd = WONT;
		break;		
	}
	tln_build_reply( cmd, opt );
	
}



/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
void dontoption( unsigned char opt )
{
	//unsigned char cmd = 0;
	
	switch( opt )	{
		case T_ECHO:
			tlnConioTcb.local_echo = 0;
			break;
		case T_SGA:
			tlnConioTcb.suppress_ga = 0;
			break;
	}
	tln_build_reply( WONT, opt );
}



/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
void willoption( unsigned char opt )
{
		unsigned char cmd = 0;
	
	switch( opt )	{
		case T_ECHO:
			tlnConioTcb.local_echo = 1;
			cmd = DO;
			break;
		case T_SGA:
			tlnConioTcb.suppress_ga = 1;
			cmd = DO;
			break;
		case T_TM:
		default:
			cmd = DONT;
		break;		
	}
	tln_build_reply( cmd, opt );

}



/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
void wontoption( unsigned char opt )
{
	//unsigned char cmd = 0;
	
	switch( opt )	{
		case T_ECHO:
			tlnConioTcb.local_echo = 0;
			break;
		case T_SGA:
			tlnConioTcb.suppress_ga = 0;
			break;
	}
	tln_build_reply( DONT, opt );
}



/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
int telnet_receive( void )
{
	int len, retval;
	NET_ERR err;
	unsigned char *bufp;
	unsigned char state, oldstate, charin;
	

	state = TS_DATA;
	oldstate = TS_DATA;
	retval = -2;
	
	while((len = NetSock_RxData (tln_sock, tn_receive_buf, TLN_RCV_BUF_SIZE - 1, NET_SOCK_FLAG_NO_BLOCK, &err)) > 0)	{
		bufp = tn_receive_buf;
		
		for (state = TS_DATA; len > 0L; )	{
			charin = *bufp++ & 0xFF;
			len--;
again:
			switch( state )		{
				case TS_DATA:
do_data:
					if (charin == IAC) {
						oldstate = TS_DATA;
						state = TS_IAC;
						continue;
					}
					if (charin == '\r' )	{
						state = TS_CRSEEN;
						if( (tlnConioTcb.local_echo == 1)||(tlnConioTcb.echo == 1))
							NetSock_TxData( tln_sock, "\n\r", 2, NET_SOCK_FLAG_NONE, &err);
						continue;
					}
					if(charin == '\n')	{
						retval = 1;
						tln_inputchar('\0');
						if( (tlnConioTcb.local_echo == 1)||(tlnConioTcb.echo == 1))
							NetSock_TxData( tln_sock, "\n\r", 2, NET_SOCK_FLAG_NONE, &err);
						continue;
					}
					tln_inputchar(charin);
					if((charin >= 0x20) && (charin <= 0x7F))
						if( (tlnConioTcb.local_echo == 1)||(tlnConioTcb.echo == 1))
							NetSock_TxData( tln_sock, &charin, 1, NET_SOCK_FLAG_NONE, &err);

					continue;
				case TS_CRSEEN:
					state = TS_DATA;
					if (charin == '\n')	{
						retval = 1;
						tln_inputchar('\0');
						continue;	
					}
					tln_inputchar('\r');
					if (charin == '\0')
						continue;	
					goto do_data;
				case TS_URGENT:
					if (charin == IAC) {
						oldstate = TS_URGENT;
						state = TS_IAC;
					}
					continue;
				case TS_IAC:
					switch(charin)	{
						case NOP:
							break;
						case WILL:
						case WONT:
						case DO:
						case DONT:
							state = TS_WILL + (charin - WILL);
							continue;       /* next */
						case DM:                        /* data mark */
							oldstate = state = TS_DATA;
							continue;
						default:
							state = oldstate;
							goto again;
					}
					state = oldstate;
					continue;
				case TS_WILL:
					willoption( charin );
					state = oldstate;
					continue;
				case TS_WONT:
					wontoption( charin );
					state = oldstate;
					continue;
				case TS_DO:
					dooption( charin );
					state = oldstate;
					continue;
				case TS_DONT:
					dontoption( charin );
					state = oldstate;
					continue;
				default:
					break;
			}	
		}
		if (state == TS_CRSEEN && len == 0L) {
			state = TS_DATA;
			tln_inputchar('\r');
		}
	}
	if( len == 0)
		return 0;
	else return retval;
}

/*
*********************************************************************************************************
* Function name	:
*
* Description	:
*
* Argument(s)	:
*
*
* Return(s)		:
*
*
* Caller(s)		:
*
* Note(s)		:
*
**********************************************************************************************************
*/
#define	TLN_SM_HOME				0
#define	TLN_SM_WAITCONN			1
#define	TLN_SM_SNDLOGON			2
#define	TLN_SM_WLOGIN			3
#define	TLN_SM_WPASSW			4
#define	TLN_SM_READY			5
#define	TLN_SM_GETCOMMAND		6
#define	TLN_SM_PARSECOMMAND		7
#define	TLN_SM_EXECCOMMAND		8

void  TelnetServerTask (void  *p_arg)
{
	NET_SOCK_ID         sock_listen;
	//NET_SOCK_ID         sock_req;
	NET_SOCK_ADDR_IP    server_sock_addr_ip;
	NET_SOCK_ADDR_LEN   server_sock_addr_ip_size;
	NET_SOCK_ADDR_IP    client_sock_addr_ip;
	NET_SOCK_ADDR_LEN   client_sock_addr_ip_size;

	unsigned char 		tln_sm_state, buf[3];
	int					retval;
	OS_ERR 				os_err;
	NET_ERR             err;

	uint8_t tlnlnkstate;

	(void)&p_arg;
	tln_sm_state = TLN_SM_HOME;

	tlnConioTcb.conio = (ConsoleIO *)(&(TLN_console_io));
	tlnConioTcb.echo = 1;
	SystemFlag &= ~INTLNT;


	client_sock_addr_ip_size = sizeof(client_sock_addr_ip);

	tlnlnkstate = ETHLNK_DISCONNECTED;
	while(DEF_ON)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

		//-------------------------------------------------------------------
		//Chequeo el estado de la red para ver si es necesario reinicializar
		switch( tlnlnkstate )	{
		case ETHLNK_CONNECTED :
			if( ethlink_state == ETHLNK_DISCONNECTED )	{
				tlnlnkstate = ETHLNK_DISCONNECTED;
				tln_sm_state = TLN_SM_HOME;
				NetSock_Close(tln_sock,    &err);
				NetSock_Close(sock_listen, &err);
			}
			break;
		case ETHLNK_DISCONNECTED :
			if( ethlink_state == ETHLNK_CONNECTED )	{
				tlnlnkstate = ETHLNK_CONNECTED;
			}
			break;
		}
		//-------------------------------------------------------------------

		if( tlnlnkstate == ETHLNK_CONNECTED )	{
		switch( tln_sm_state )	{
			//-------------------------------------------------------------------------------------------
			case TLN_SM_HOME :
				SystemFlag &= ~INTLNT;
				sock_listen = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
											NET_SOCK_TYPE_STREAM,
											NET_SOCK_PROTOCOL_TCP,
											&err);
				if (err != NET_SOCK_ERR_NONE) {
					break;
				}
				server_sock_addr_ip_size = sizeof(server_sock_addr_ip);
				Mem_Clr((void *)&server_sock_addr_ip, (CPU_SIZE_T) server_sock_addr_ip_size);
				server_sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
				server_sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(NET_SOCK_ADDR_IP_WILD_CARD);
				server_sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(TELNET_SERVER_PORT);
				NetSock_Bind((NET_SOCK_ID      ) sock_listen,
							 (NET_SOCK_ADDR   *)&server_sock_addr_ip,
							 (NET_SOCK_ADDR_LEN) NET_SOCK_ADDR_SIZE,
							 (NET_ERR         *)&err);
				if (err != NET_SOCK_ERR_NONE) {
					NetSock_Close(sock_listen, &err);
					break;
				}
				NetSock_Listen( sock_listen,
								TCP_SERVER_CONN_Q_SIZE,
								&err);
				if (err != NET_SOCK_ERR_NONE) {
					NetSock_Close(sock_listen, &err);
					break;
				}
				tln_sm_state = TLN_SM_WAITCONN;
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_WAITCONN :
				SystemFlag &= ~INTLNT;
				tln_sock = NetSock_Accept((NET_SOCK_ID        ) sock_listen,
										  (NET_SOCK_ADDR     *)&client_sock_addr_ip,
										  (NET_SOCK_ADDR_LEN *)&client_sock_addr_ip_size,
										  (NET_ERR           *)&err);
				switch (err) {
					case NET_SOCK_ERR_NONE:
						tln_sm_state = TLN_SM_SNDLOGON;
						break;

					case NET_ERR_INIT_INCOMPLETE:
					case NET_SOCK_ERR_NULL_PTR:
					case NET_SOCK_ERR_NONE_AVAIL:
					case NET_SOCK_ERR_CONN_ACCEPT_Q_NONE_AVAIL:
					case NET_SOCK_ERR_CONN_SIGNAL_TIMEOUT:
					case NET_OS_ERR_LOCK:
						break;

					default:
						tln_sm_state = TLN_SM_HOME;
						NetSock_Close(tln_sock,    &err);
						NetSock_Close(sock_listen, &err);
						break;
				}
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_SNDLOGON :
				//tln_build_reply( WILL, T_ECHO );
				NetSock_TxData( tln_sock,
								TLN_INIT_MESSAGE,
								strlen(TLN_INIT_MESSAGE),
								NET_SOCK_FLAG_NONE,
								&err);
				if (err != NET_SOCK_ERR_NONE) {
					tln_sm_state = TLN_SM_HOME;
					NetSock_Close(tln_sock,    &err);
					NetSock_Close(sock_listen, &err);
				} else
					tln_sm_state = TLN_SM_READY;
					SystemFlag |= INTLNT;
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_WLOGIN :
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_WPASSW :
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_READY :
				SystemFlag |= INTLNT;
				tlnConioTcb.command[0] = '\0';
				tlnConioTcb.cmdptr = tlnConioTcb.command;
				tlnConioTcb.sawesc = 0;
				tln_sm_state = TLN_SM_GETCOMMAND;
				NetSock_TxData( tln_sock,
								"$ ",
								2,
								NET_SOCK_FLAG_NONE,
								&err);
				if( err != NET_SOCK_ERR_NONE)	{
					tln_sm_state = TLN_SM_HOME;
					NetSock_Close(tln_sock,    &err);
					NetSock_Close(sock_listen, &err);
				}
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_GETCOMMAND :
				SystemFlag |= INTLNT;
				retval = NetSock_IsConn(tln_sock, &err);
//				//------------------------------------
//				buf[0] = 0x00;
//				buf[1] = 0x00;
//				buf[2] = 0x00;
//				NetSock_TxData( tln_sock, buf, 1, NET_SOCK_FLAG_NONE, &err);
//				//------------------------------------
				if( retval == DEF_NO )	{
					tln_sm_state = TLN_SM_HOME;
					NetSock_Close(tln_sock,    &err);
					NetSock_Close(sock_listen, &err);
					break;
				}
				switch( telnet_receive())	{
					case -1 :tln
						tln_sm_state = TLN_SM_READY;
						break;
					case 1 :
						tln_sm_state = TLN_SM_PARSECOMMAND;
						break;
					case -2:
						break;
					default:
						tln_sm_state = TLN_SM_HOME;
						NetSock_Close(tln_sock,    &err);
						NetSock_Close(sock_listen, &err);
						break;
				}
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_PARSECOMMAND :
				SystemFlag |= INTLNT;
				tlnConioTcb.numparams = __con_nullify(tlnConioTcb.command);
				if (tlnConioTcb.numparams == 0) {
					tln_sm_state = TLN_SM_READY;
					break;
				}
				tlnConioTcb.cmdspec = __con_parsecmd(tlnConioTcb.command, tlnConioTcb.numparams,
												&(tlnConioTcb.commandparams));
				if ((tlnConioTcb.cmdspec != NULL) && (tlnConioTcb.cmdspec->cmdfunc != NULL)) {
					tlnConioTcb.substate = 0;
					tln_sm_state =  TLN_SM_EXECCOMMAND;
				} else {
					NetSock_TxData( tln_sock,
									"BAD COMMAND\r\n",
									13,
									NET_SOCK_FLAG_NONE,
									&err);
					if( err != NET_SOCK_ERR_NONE)	{
						tln_sm_state = TLN_SM_HOME;
						NetSock_Close(tln_sock,    &err);
						NetSock_Close(sock_listen, &err);
					} else
						tln_sm_state = TLN_SM_READY;
				}
				break;
			//-------------------------------------------------------------------------------------------
			case TLN_SM_EXECCOMMAND :
				SystemFlag |= INTLNT;
				retval = tlnConioTcb.cmdspec->cmdfunc((ConsoleState* )(&tlnConioTcb));
				switch(retval)	{
					case -1 :
						NetSock_TxData( tln_sock,
										"ERROR\r\n",
										7,
										NET_SOCK_FLAG_NONE,
										&err);
						if( err != NET_SOCK_ERR_NONE)	{
							tln_sm_state = TLN_SM_HOME;
							NetSock_Close(tln_sock,    &err);
							NetSock_Close(sock_listen, &err);
						} else
							tln_sm_state = TLN_SM_READY;

						break;
					case -2 :
						tln_sm_state = TLN_SM_HOME;
						NetSock_Close(tln_sock,    &err);
						NetSock_Close(sock_listen, &err);
						break;
					case 1 :
						NetSock_TxData( tln_sock,
										"OK\r\n",
										4,
										NET_SOCK_FLAG_NONE,
										&err);
						if( err != NET_SOCK_ERR_NONE)	{
							tln_sm_state = TLN_SM_HOME;
							NetSock_Close(tln_sock,    &err);
							NetSock_Close(sock_listen, &err);
						} else
							tln_sm_state = TLN_SM_READY;
						break;

				}
				break;
			//-------------------------------------------------------------------------------------------
			default:
				tln_sm_state = TLN_SM_HOME;
				break;
		}
		}
	}
}

int conio_telnet_puts(unsigned char *s)
{
	int len, retval;
	NET_ERR err;


	len = strlen((char const *)s);

	retval = NetSock_TxData( tln_sock,
							 s,
							 len,
							 NET_SOCK_FLAG_NONE,
							 &err);

	if( err != NET_SOCK_ERR_NONE)
		return -2;
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &err);
	return len;
}


int conio_telnet_write(unsigned char *s, int len)
{
	int offset, retval;
	NET_ERR err;

	offset = 0;

	while(offset < len) {
		retval = NetSock_TxData( tln_sock,
								 (s + offset),
								 (len - offset),
								 NET_SOCK_FLAG_NONE,
								 &err);
		if( err != NET_SOCK_ERR_NONE)
			return -2;
		offset += retval;
	}
	return len;
}


int conio_telnet_read(void *d, int length, unsigned long tmout)
{
	long timer;
	int retval, len;
	unsigned char *p;
	OS_ERR err;


	timer = OSTimeGet(&err) + tmout;
	len = length;
	p = d;
	do {
		NetSock_RxData (tln_sock, p , length, NET_SOCK_FLAG_NO_BLOCK, &err);
		if( err != NET_SOCK_ERR_NONE)
			return -2;
		p += retval;
		len -= retval;
	} while((len > 0) && (timer > OSTimeGet(&err)));

	return (length-len);
}

