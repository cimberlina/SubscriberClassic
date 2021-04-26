#include "includes.h"

uint8_t udpcmd_state;

#define UDPCMD_SERVER_PORT 	777

NET_SOCK_ID  udpcmd_sock;
NET_SOCK_ADDR_IP 		udpserver_sock_addr_ip, udpclient_sock_addr_ip;
NET_SOCK_ADDR_LEN 		udpserver_sock_addr_ip_size, udpclient_sock_addr_ip_size;

UDPSERVER_ConioTCB      udpserverConioTcb;
UDPSERVER_ConioTCB *    udptcb_ptr;


//ConsoleIO console1_io[] = {{ 777,
//								   NULL,
//								   NULL,
//								   NULL,
//								   conio_udpserver_puts,
//								   NULL,
//								   NULL,
//								   NULL,
//								   conio_udpserver_read,
//								   conio_udpserver_write }};



void udpserver_init(void)
{
    udpserverConioTcb.conio = (ConsoleIO *)(&(console_io[1]));
	udpserverConioTcb.echo = 1;
    udpserverConioTcb.command[0] = '\0';
    udpserverConioTcb.cmdptr = udpserverConioTcb.command;
    udpserverConioTcb.sawesc = 0;
    
    udptcb_ptr = &udpserverConioTcb;
    
    udpcmd_state = UDPCMD_HOME;
}



void UDP_cmd_server(void)
{
	NET_ERR 	err;
	NET_SOCK_RTN_CODE 	rx_size;
    int retval, i;
    uint32_t error;
    uint8_t temp[16];

	switch(udpcmd_state) 	{
		case UDPCMD_HOME:
			udpcmd_sock = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4, NET_SOCK_TYPE_DATAGRAM, NET_SOCK_PROTOCOL_UDP, &err);
			if (err != NET_SOCK_ERR_NONE) {
				break;
			}
			udpserver_sock_addr_ip_size = sizeof(udpserver_sock_addr_ip);
			Mem_Clr((void *)&(udpserver_sock_addr_ip), (CPU_SIZE_T) udpserver_sock_addr_ip_size);
			udpserver_sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
			udpserver_sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(NET_SOCK_ADDR_IP_WILD_CARD);
			udpserver_sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(UDPCMD_SERVER_PORT);

			NetSock_Bind((NET_SOCK_ID 			) udpcmd_sock,
						 (NET_SOCK_ADDR 	   *)&udpserver_sock_addr_ip,
						 (NET_SOCK_ADDR_LEN 	) NET_SOCK_ADDR_SIZE,
						 (NET_ERR 			   *)&err);
			if (err != NET_SOCK_ERR_NONE) {
				NetSock_Close(udpcmd_sock, &err);
				udpcmd_state = UDPCMD_WAITCLOSE;
				break;
			}
			udpcmd_state = UDPCMD_READY;
			break;
		case UDPCMD_READY:
			udpclient_sock_addr_ip_size = sizeof(udpclient_sock_addr_ip);
			rx_size = NetSock_RxDataFrom((NET_SOCK_ID)			udpcmd_sock,
										(void *)				udpserverConioTcb.command,
										(CPU_INT16S)			TLN_CMD_SIZE,
										(CPU_INT16S)			(MSG_PEEK | MSG_DONTWAIT),
										(NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
										(NET_SOCK_ADDR_LEN *)	&udpclient_sock_addr_ip_size,
										(void *)				0,
										(CPU_INT08U)			0,
										(CPU_INT08U *)			0,
										(NET_ERR *)				&err );


			if(rx_size > 0) 	{
				//Aca llamo al monserver para que procese el paquete recibido
				rx_size = NetSock_RxDataFrom((NET_SOCK_ID)			udpcmd_sock,
											(void *)				udpserverConioTcb.command, 
											(CPU_INT16S)			TLN_CMD_SIZE,
											(CPU_INT16S)			NET_SOCK_FLAG_NONE,
											(NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
											(NET_SOCK_ADDR_LEN *)	&udpclient_sock_addr_ip_size,
											(void *)				0,
											(CPU_INT08U)			0,
											(CPU_INT08U *)			0,
											(NET_ERR *)				&err );
                
                //preparamos  para ejectar la linea de comandos
                udpserverConioTcb.command[rx_size] = 0;
                for( i = 0; i < rx_size; i++)   {
                    if((udpserverConioTcb.command[i] == '\r') || (udpserverConioTcb.command[i] == '\n') )   {
                        udpserverConioTcb.command[i] = 0;
                        break;
                    } 
                }
                udpserverConioTcb.numparams = __con_nullify(udpserverConioTcb.command);
 				if (udpserverConioTcb.numparams == 0) {
					udpserverConioTcb.command[0] = '\0';
                    udpserverConioTcb.cmdptr = udpserverConioTcb.command;
                    udpserverConioTcb.sawesc = 0;
					break;
				}               
                udpserverConioTcb.cmdspec = __con_parsecmd(udpserverConioTcb.command, udpserverConioTcb.numparams, &(udpserverConioTcb.commandparams));
                
                if ((udpserverConioTcb.cmdspec != NULL) && (udpserverConioTcb.cmdspec->cmdfunc != NULL)) {
					udpserverConioTcb.substate = 0;
					//*******************************************************************************************
					//* aca ejecutamos el comando propiamente dicho
					if( (udpserverConioTcb.cmdspec->qualyfier <= 1 ) || (!(SystemFlag6 & USE_LICENSE)) )	{	//comandos de ejcucion libre 0, comando tipo LIC de nivel 1
						retval = udpserverConioTcb.cmdspec->cmdfunc(udptcb_ptr);
					} else {		//aca el comando requiere saber el nivel de seguridad del tecmon
						
						error = flash0_read(temp, LOGIN_ENABLED, 1);
						if(temp[0] == 0x5A )	{
							if( valid_license() )	{
								retval = udpserverConioTcb.cmdspec->cmdfunc(udptcb_ptr);
							} else	{
								retval = -3;
							}
						} else	{
							retval = -4;
						}
					}
					//*
					//*******************************************************************************************
                    switch(retval)  {
                        case -1:
                            rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"ERROR\r\n",
                                                (CPU_INT16S)			7,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                            rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"\0",
                                                (CPU_INT16S)			1,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                            if( err != NET_SOCK_ERR_NONE)	{
                                NetSock_Close(udpcmd_sock, &err);
                                udpcmd_state = SM_WAIT_CLOSE;
                                break;
                            }
                            break;
                        case -2:
                            NetSock_Close(udpcmd_sock, &err);
                            udpcmd_state = SM_WAIT_CLOSE;
                            break;
                        case -3:
							rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
												(void *)				"ERROR : Licencia Invalida\r\n",
												(CPU_INT16S)			28,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
												(NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
												(NET_ERR *)				&err );
							rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
												(void *)				"\0",
												(CPU_INT16S)			1,
												(CPU_INT16S)			NET_SOCK_FLAG_NONE,
												(NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
												(NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
												(NET_ERR *)				&err );
							if( err != NET_SOCK_ERR_NONE)	{
								NetSock_Close(udpcmd_sock, &err);
								udpcmd_state = SM_WAIT_CLOSE;
								break;
							}
							break;
						case -4:
                            rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"ERROR : Login Invalido\r\n",
                                                (CPU_INT16S)			25,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                            rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"\0",
                                                (CPU_INT16S)			1,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                            if( err != NET_SOCK_ERR_NONE)	{
                                NetSock_Close(udpcmd_sock, &err);
                                udpcmd_state = SM_WAIT_CLOSE;
                                break;
                            }
                            break;
                        case 1:
                            rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"OK\r\n",
                                                (CPU_INT16S)			4,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                            rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"\0",
                                                (CPU_INT16S)			1,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                            if( err != NET_SOCK_ERR_NONE)	{
                                NetSock_Close(udpcmd_sock, &err);
                                udpcmd_state = SM_WAIT_CLOSE;
                                break;
                            }
                            break;
                    }
                    
				} else {
                    rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"BAD COMMAND\r\n",
                                                (CPU_INT16S)			13,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
                    rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                                (void *)				"\0",
                                                (CPU_INT16S)			1,
                                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                                (NET_ERR *)				&err );
					if( err != NET_SOCK_ERR_NONE )	{
						NetSock_Close(udpcmd_sock, &err);
                        udpcmd_state = SM_WAIT_CLOSE;
                        break;
					}
				}

			}
            udpserverConioTcb.command[0] = '\0';
            udpserverConioTcb.cmdptr = udpserverConioTcb.command;
            udpserverConioTcb.sawesc = 0;
			break;
		case UDPCMD_WAITCLOSE:
			udpcmd_state = UDPCMD_HOME;
			break;
		default:
			udpcmd_state = UDPCMD_HOME;
			break;
	}
}

int conio_udpserver_puts(unsigned char *s)
{
	int len;
    NET_SOCK_RTN_CODE 	rx_size;
	NET_ERR err;


	len = strlen((char const *)s);
    
    
    rx_size = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                (void *)				s,
                                (CPU_INT16S)			len,
                                (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                (NET_ERR *)				&err );


	if( err != NET_SOCK_ERR_NONE)
		return -2;
	OSTimeDlyHMSM(0, 0, 0, 30, OS_OPT_TIME_HMSM_STRICT, &err);
	return len;
}


int conio_udpserver_write(unsigned char *s, int len)
{
	int offset, retval;
	NET_ERR err;

	offset = 0;

	while(offset < len) {
        retval = NetSock_TxDataTo(	(NET_SOCK_ID)			udpcmd_sock,
                                    (void *)				(s + offset),
                                    (CPU_INT16S)			(len - offset),
                                    (CPU_INT16S)			NET_SOCK_FLAG_NONE,
                                    (NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
                                    (NET_SOCK_ADDR_LEN)		sizeof(udpclient_sock_addr_ip),
                                    (NET_ERR *)				&err );
 
		if( err != NET_SOCK_ERR_NONE)
			return -2;
		offset += retval;
	}
	return len;
}


int conio_udpserver_read(void *d, int length, unsigned long tmout)
{
	long timer;
	int retval, len;
	unsigned char *p;
	OS_ERR err;


	timer = OSTimeGet(&err) + tmout;
	len = length;
	p = d;
	do {
        retval = NetSock_RxDataFrom((NET_SOCK_ID)			udpcmd_sock,
									(void *)				p,
									(CPU_INT16S)			length,
									(CPU_INT16S)			(MSG_PEEK | MSG_DONTWAIT),
									(NET_SOCK_ADDR *)		&udpclient_sock_addr_ip,
									(NET_SOCK_ADDR_LEN *)	&udpclient_sock_addr_ip_size,
									(void *)				0,
									(CPU_INT08U)			0,
									(CPU_INT08U *)			0,
									(NET_ERR *)				&err );
        
		if( err != NET_SOCK_ERR_NONE)
			return -2;
		p += retval;
		len -= retval;
	} while((len > 0) && (timer > OSTimeGet(&err)));

	return (length-len);
}

