/*
 * telnet_console.h
 *
 *  Created on: Oct 28, 2011
 *      Author: IronMan
 */

#ifndef TELNET_CONSOLE_H_
#define TELNET_CONSOLE_H_

#define TLN_CMD_SIZE	128
#define	TLN_BUF_SIZE	256			//512

#define IAC			255             /* interpret as command: */
#define DONT			254             /* you are not to use option */
#define DO			253             /* please, you use option */
#define WONT		252             /* I won't use option */
#define WILL			251             /* I will use option */
#define SB			250             /* interpret as subnegotiation */
#define GA 			249             /* you may reverse the line */
#define EL			248             /* erase the current line */
#define EC			247             /* erase the current character */
#define AYT			246             /* are you there */
#define AO			245             /* abort output--but let program finish */
#define IP			244             /* interrupt process--permanently */
#define BREAK		243             /* break */
#define DM			242             /* data mark--for connection cleaning */
#define NOP			241             /* nop */
#define SE			240             /* end sub negotiation */
#define EOR			239             /* end of record */

#define SYNCH		242             /* for telfunc calls */

/* telnet options */

#define T_BINARY        0       /* 8-bit data path */
#define T_ECHO          1       /* echo */
#define T_RCP           2       /* prepare to reconnect */
#define T_SGA           3       /* suppress go ahead */
#define T_NAMS          4       /* approximate message size */
#define T_STATUS        5       /* give status */
#define T_TM            6       /* timing mark */
#define T_RCTE          7       /* remote controlled transmission and echo */
#define T_NAOL          8       /* negotiate about output line width */
#define T_NAOP          9       /* negotiate about output page size */
#define T_NAOCRD        10      /* negotiate about CR disposition */
#define T_NAOHTS        11      /* negotiate about horizontal tabstops */
#define T_NAOHTD        12      /* negotiate about horizontal tab disposition */
#define T_NAOFFD        13      /* negotiate about formfeed disposition */
#define T_NAOVTS        14      /* negotiate about vertical tab stops */
#define T_NAOVTD        15      /* negotiate about vertical tab disposition */
#define T_NAOLFD        16      /* negotiate about output LF disposition */
#define T_XASCII        17      /* extended ascic character set */
#define T_LOGOUT        18      /* force logout */
#define T_BM            19      /* byte macro */
#define T_DET           20      /* data entry terminal */
#define T_SUPDUP        21      /* supdup protocol */
#define T_OSUPDUP       22      /* supdup output */
#define T_SENDLOC       23      /* send location */
#define T_TTYTYPE       24      /* terminal type */
#define T_EOR           25      /* end of record */
#define T_TUID          26      /* TACACS user identification */
#define T_OUTMRK        27      /* output marking */
#define T_TTYLOC        28      /* terminal location number */
#define T_3270          29      /* 3270-REGIME */
#define T_X3PAD         30      /* X.3-PAD */
#define T_EXOPL         255     /* extended-options-list */


/* Telnet receiver states for 'telrcv' automaton */
#define TS_DATA		0       /* expecting data */
#define TS_IAC		1       /* have seen an IAC byte */
#define TS_WILL		2       /* have seen a WILL command */
#define TS_WONT		3       /* have seen a WON'T command */
#define TS_DO		4       /* have seen a DO command */
#define TS_DONT		5       /* have seen a DON'T command */
#define TS_CRSEEN	6       /* have seen a CR */
#define TS_SB		7       /* have seen an SB (subnegotiation) command */
#define TS_PARAM	8       /* subnegotiation paramater list */
#define TS_PIAC		9       /* have seen an IAC byte in subnegotiation parameter list */
#define TS_URGENT	10      /* reading urgent data */

#define TLN_RCV_BUF_SIZE	256		//512


typedef struct {
	int console_number;
	ConsoleIO *conio;
	int state;
	int laststate;

	char command[TLN_CMD_SIZE];
	char* cmdptr;
	char buffer[TLN_BUF_SIZE];
	char* bufferend;

	ConsoleCommand* cmdspec;
	int sawcr;
	int sawesc;
	int echo;
	int substate;
	unsigned int error;
	int numparams;
	int commandparams;
	char cmddata[16];
	int spec;
	/* telnet state flags */
	char	suppress_ga;
	char	local_echo;
	long timeout;
} TELNET_ConioTCB;


void tln_inputchar( unsigned char data);
void tln_build_reply( unsigned char command, unsigned char option );
void dooption( unsigned char opt );
void dontoption( unsigned char opt );
void willoption( unsigned char opt );
void wontoption( unsigned char opt );
int telnet_receive( void );
int conio_telnet_read(void *d, int length, unsigned long tmout);
int conio_telnet_write(unsigned char *s, int len);
int conio_telnet_puts(unsigned char *s);


extern OS_TCB		TelnetServerTask_TCB;
extern CPU_STK		TelnetServerTask_Stk[];



void  TelnetServerTask (void  *p_arg);


#define TLN_INIT_MESSAGE 	"\
-----------------------------------------\r\n\
NXP LPC1769 Cortex-M3 System-UP by C.M.I.\r\n\
EYSE (C) 2012 Abonado - EYSE Ingenieria\r\n\
-----------------------------------------\r\n\n"

#define TELNET_SERVER_PORT		23
#define TCP_SERVER_CONN_Q_SIZE	1

#endif /* TELNET_CONSOLE_H_ */
