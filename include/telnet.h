/*
 * telnet.h
 *
 *  Created on: Oct 21, 2011
 *      Author: IronMan
 */

#ifndef TELNET_H_
#define TELNET_H_



/*** BeginHeader telnet_init */
/*
 * The telnet-cooker, to cook off the telnet control codes
 * from a telnet client, and negotiate any necessary
 * options.
 */
typedef struct {
	int	state;
#define TELNET_COOKER_NORMAL		0
#define TELNET_COOKER_IAC			1
#define TELNET_COOKER_DO			2
#define TELNET_COOKER_DONT			3
#define TELNET_COOKER_WILL			4
#define TELNET_COOKER_WONT			5
#define TELNET_COOKER_CR			6

	NET_SOCK_ID	s;

	/* telnet state flags */
	char	suppress_ga;
	char	local_echo;

	/* other internal FSM flags */
	char	send_nl;
	int	user_options;
} _TelnetCooker;

/* special telnet characters/commands */
#define TELNET_IAC		255		// "Is A Command"
#define TELNET_DONT		254		// "Don't do this"
#define TELNET_DO			253		// "Do this"
#define TELNET_WONT		252		// "I won't do this"
#define TELNET_WILL		251		// "I will do this"
#define TELNET_SB			250		// "Sub-negotiation follows"
#define TELNET_GA			249		// "Go-Ahead"
#define TELNET_EL			248		// "Erase Line"
#define TELNET_EC			247		// "Erase Character"
#define TELNET_AYT		246		// "Are You There"
#define TELNET_AO			245		// "Abort Output"
#define TELNET_IP			244		// "Suspend/Interrupt"
#define TELNET_BRK		243		// "Break/Attention"
#define TELNET_DM			242		// "Data Mark"
#define TELNET_NOP		241		// "NOOP"
#define TELNET_SE			240		// "End of Sub-negotiation"

/* do/will commands */
#define TELNET_ECHO			1		// "local echo on"
#define TELNET_SUPPRESS_GA	3		// "suppress usage of GA"

#define TELNET_OPTION_GA    0x01  // disable GA messages
#define TELNET_OPTION_ECHO  0x02  // turn off local (client) echo

// prototypes
void telnet_init(_TelnetCooker *state, NET_SOCK_ID s, int options);

int telnet_send_command(_TelnetCooker *state, char command, char option);
int telnet_fastread(_TelnetCooker *state, CPU_INT08U *dp, int len);
int telnet_fastwrite(_TelnetCooker *state, CPU_INT08U *dp, int len);


#endif /* TELNET_H_ */
