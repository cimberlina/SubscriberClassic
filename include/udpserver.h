/* 
 * File:   udpserver.h
 * Author: claudio
 *
 * Created on May 24, 2013, 12:04 PM
 */

#ifndef UDPSERVER_H
#define	UDPSERVER_H

extern uint8_t udpcmd_state;
#define	UDPCMD_HOME			0x10
#define	UDPCMD_READY		0x20
#define	UDPCMD_WAITCLOSE	0x30

void udpserver_init(void);
void UDP_cmd_server(void);
int conio_udpserver_puts(unsigned char *s);
int conio_udpserver_write(unsigned char *s, int len);
int conio_udpserver_read(void *d, int length, unsigned long tmout);

#define TLN_CMD_SIZE	128
#define	TLN_BUF_SIZE	256

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
} UDPSERVER_ConioTCB;

#endif	/* UDPSERVER_H */

