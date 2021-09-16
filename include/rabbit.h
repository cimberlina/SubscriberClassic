/*
 * rabbit.h
 *
 *  Created on: Jul 24, 2012
 *      Author: ironman
 */

#ifndef RABBIT_H_
#define RABBIT_H_
//*****

//#define	ETH_AUTO_NEG	0x01
//#define	ETH_FULLD_100M	0x02
//#define	ETH_FULLD_10M	0x03
//#define	ETH_HALFD_100M	0x04
//#define	ETH_HALFD_10M	0x05

#define	ETH_AUTO_NEG	0x01
#define	ETH_FULLD_100M	0x02
#define	ETH_FULLD_10M	0x04
#define	ETH_HALFD_100M	0x03
#define	ETH_HALFD_10M	0x05
#define	ETH_FD10M_DEF	0x06
#define	ETH_PHY_END		ETH_FD10M_DEF + 1

#define	CENTRALOFFICEMAX	2

extern uint8_t NumAbo;
extern uint8_t NumCen;
//extern char NomSuc[32];
//extern uint16_t hbaccount[CENTRALOFFICEMAX];

//extern uint16_t sockPort[CENTRALOFFICEMAX];
//extern int heartBeatTime[CENTRALOFFICEMAX];
//extern AlarmProtocols coProtocol[CENTRALOFFICEMAX];
//extern int inUse[CENTRALOFFICEMAX];
//extern int sockProtocol[CENTRALOFFICEMAX];
//extern int sockType[CENTRALOFFICEMAX];

//extern char Server1[16];
//extern char Server2[16];

//extern char LocalIP[16];
//extern char Netmask[16];
//extern char Gateway[16];

extern OS_TCB		RabbitTask_TCB;
extern CPU_STK		RabbitTask_Stk[RabbitTask_STK_SIZE];
void  RabbitTask(void  *p_arg);


#define	TXEVENTBUFFERLEN	25

#define	SEND_RETRIES_TIME	58		//15

#define	SS_SOCKET_CLOSED		1
#define	SS_SOCKET_ACTIVE		2
#define	SS_SOCKET_SUSPEND		3
#define	SS_SOCKET_SUSPEND_PEND	4
#define	SS_SOCKET_LISTEN		5
#define	SS_SOCKET_INCOMMING		6

typedef enum	{
	SM_HOME = 0,
	SM_DNS_RESOLUTION,
	SM_CREATE_SOCKET,
	SM_CONNECT,
	SM_EYSEID,
	SM_SOCK_READY,
	SM_SOCK_WAITACK,
	SM_SOCK_TEST,
	SM_WAIT_CLOSE
} SM_STATE;

typedef enum	{
	RXSM_IDLE = 0,
	RXSM_INPKT
} RXSM_STATE;

typedef enum	{
	WR3K_IDLE = 0,
	WR3K_WDOG,
	WR3K_WRST,
    WR3K_WAITONEHOUR
} WDOGR3K_STATE;

#define	EVESND_FLAG		(1 << 0)
#define	HBTSND_FLAG		(1 << 1)
#define	ACKRXD_FLAG		(1 << 2)
#define	EYSEID_FLAG		(1 << 3)
#define	ACKWDG_FLAG		(1 << 4)
#define	SNDHBT_FLAG		(1 << 5)
#define	E700_1_FLAG		(1 << 6)
#define	E700_2_FLAG		(1 << 7)
#define ONEHOURRETRY_FLAG   (1 << 8)

typedef NET_SOCK_ID	SOCKET;

typedef struct 	{
	int				inuse;
	//de la conexion por socket
	char				ServerName[50];
	NET_SOCK_ID 		monsock;
	NET_IP_ADDR			server_ip_addr;
	NET_SOCK_ADDR_IP    server_sock_addr_ip;
	NET_SOCK_ADDR_LEN   server_sock_addr_ip_size;

	NET_PORT_NBR		monport;
	NET_SOCK_TYPE		socktype;
	NET_SOCK_PROTOCOL	sockprotocol;



	int				HeartBeatTime;
	AlarmProtocols	protocol;


	uint16_t		HBaccount;


	//de la maquina de estados
	SM_STATE		state;
	time_t 			timer;
	int				retries;
	uint16_t		flags;
	int 			reconnect;

	RXSM_STATE		rxsm_state;
	time_t			rxsm_timer;
	uint16_t		rxsm_flags;

	time_t			wdogr3kTimer;
	WDOGR3K_STATE	wdogstate;

	//del buffer de eventos a transmitir
	EventRecord eventRecord[TXEVENTBUFFERLEN];
	int eventRec_writeptr;
	int eventRec_readptr;
	int eventRec_count;
	uint8_t sec;

}  MonitorConnectionStruct;

extern int inUse[CENTRALOFFICEMAX];
extern MonitorConnectionStruct Monitoreo[CENTRALOFFICEMAX];

extern uint8_t SysCONN_flag;
#define	GSM_Module_Shutdown		(1 << 0)


extern uint8_t hbreset_retries;
#define HBRESET_RETRIES 5

void GetServerAdd( void );
void GetPortNumber(void);

void InitMonitoreoStruct(void);
void fsm_CentralOffice_Client( int co_id);

int heartbeat( int coid, uint8_t *buffer);
void fsm_sock_rx( int co_id );
int sock_dataready(SOCKET s, int *err);
void fsm_wdog_r3k(int coid);
void ack_syncronization( char * ascii_tstamp);



#endif /* RABBIT_H_ */
