/* 1bgw5c03l
 * console.h
 *
 *  Created on: Oct 19, 2011
 *      Author: IronMan
 */
#ifndef CONSOLE_H_
#define CONSOLE_H_

#define	CONSOLEIN_CTRLCHAR1		0x17			//CTRL + W
#define	CONSOLEIN_CTRLCHAR		0x02			//CTRL + B
#define	CONSOLEOUT_CTRLCHAR		0x03

extern uint16_t inconsole_timer;
extern uint8_t SerialConsoleFlag;
#define	INSERCON_flag		0x01			//entra en modo consola serial
#define	INCONFBOARD_flag	0x02


//#define	NULL	0
#define	NUM_CONSOLES	1
#define	CONSOLE_USERS	4

#define CON_CMD_SIZE 		128	//128
#define CON_BUF_SIZE 		256	//512
#define CON_CMD_DATA_SIZE 	16
#define CON_MAX_NAME 		10
#define CON_MAX_PASSWORD 	10
#define CON_TIMEOUT 		60

extern uint8_t	r3kmode;

#define CON_INIT_MESSAGE 	"\
-----------------------------------------\r\n\
NXP LPC1769 Cortex-M3 System-UP by C.M.I.\r\n\
EYSE (C) 2021 Abonado - EYSE Ingenieria\r\n\
Version: 6.11CB                  20210614\r\n\
-----------------------------------------\r\n"

#define	VERSION_ABO		"Version: 6.11CB            20210614\r\n"

#define VERSION_NUMBER  611

#define VERSION_YEAR	2021

// ConsoleCommand structure
typedef struct {
	char* command;
	int (*cmdfunc)();
	long helptext;
	uint8_t	qualyfier;
} ConsoleCommand;

// ConsoleError structure
typedef struct {
	unsigned int code;
	char* message;
} ConsoleError;

// ConsoleIO structure
typedef struct {
	long param;
	int (*open)();
	void (*close)();
	int (*tick)();
	int (*puts)();
	int (*rdUsed)();
	int (*wrUsed)();
	int (*wrFree)();
	int (*read)();
	int (*write)();
} ConsoleIO;

// pre-defined console_io structs
//#define CONSOLE_IO_COMM0(param)	{ param, COMM0_open, conio_COMM0_close, NULL, conio_COMM0_puts, COMM0rdUsed, COMM0wrUsed, COMM0wrFree, COMM0read, COMM0write }


// ConsoleState structure
typedef struct {
	int console_number;
	ConsoleIO *conio;
	int state;
	int laststate;

	char command[CON_CMD_SIZE];
	char* cmdptr;
	char buffer[CON_BUF_SIZE];
	char* bufferend;

	ConsoleCommand* cmdspec;
	int sawcr;
	int sawesc;
	int echo;
	int substate;
	unsigned int error;
	int numparams;
	int commandparams;
	char cmddata[CON_CMD_DATA_SIZE];
	int spec;
	/* telnet state flags */
	char	suppress_ga;
	char	local_echo;
	unsigned char current_user;
	long timeout;
} ConsoleState;

typedef struct
{
	char 	*name;
	char	*password;
	unsigned char level;
} ConsoleLogin;

#define CON_ERR_TIMEOUT				1
#define CON_ERR_BADCOMMAND			2
#define CON_ERR_BADPARAMETER		3
#define CON_ERR_NAMETOOLONG		4
#define CON_ERR_DUPLICATE			5
#define CON_ERR_BADFILESIZE		6
#define CON_ERR_SAVINGFILE			7
#define CON_ERR_READINGFILE		8
#define CON_ERR_FILENOTFOUND		9
#define CON_ERR_MSGTOOLONG			10
#define CON_ERR_SMTPERROR			11
#define CON_ERR_BADPASSPHRASE		12
#define CON_ERR_CANCELRESET		13
#define CON_ERR_BADVARTYPE			14
#define CON_ERR_BADVARVALUE		15
#define CON_ERR_NOVARSPACE			16
#define CON_ERR_VARNOTFOUND		17
#define CON_ERR_STRINGTOOLONG		18
#define CON_ERR_NOTAFILE			19
#define CON_ERR_NOTAVAR				20
#define CON_ERR_COMMANDTOOLONG	21
#define CON_ERR_BADIPADDRESS		22
#define CON_ERR_INVALIDPASSWORD	23
#define CON_ERR_BADIFACE			24
#define CON_ERR_BADNETWORKPARAM	25

#define CON_STANDARD_ERRORS \
	{ CON_ERR_TIMEOUT, "Timed out." },\
	{ CON_ERR_BADCOMMAND, "Unknown command." },\
	{ CON_ERR_BADPARAMETER, "Bad or missing parameter." },\
	{ CON_ERR_NAMETOOLONG, "Filename too long." },\
	{ CON_ERR_DUPLICATE, "Duplicate object found." },\
	{ CON_ERR_BADFILESIZE, "Bad file size." },\
	{ CON_ERR_SAVINGFILE, "Error saving file." },\
	{ CON_ERR_READINGFILE, "Error reading file." },\
	{ CON_ERR_FILENOTFOUND, "File not found." },\
	{ CON_ERR_MSGTOOLONG, "Mail message too long." },\
	{ CON_ERR_SMTPERROR, "SMTP server error." },\
	{ CON_ERR_BADPASSPHRASE, "Passphrases do not match!" },\
	{ CON_ERR_CANCELRESET, "Reset cancelled." },\
	{ CON_ERR_BADVARTYPE, "Bad variable type." },\
	{ CON_ERR_BADVARVALUE, "Bad variable value." },\
	{ CON_ERR_NOVARSPACE, "Out of variable space." },\
	{ CON_ERR_VARNOTFOUND, "Variable not found." },\
	{ CON_ERR_STRINGTOOLONG, "String too long." },\
	{ CON_ERR_NOTAFILE, "Not a file." },\
	{ CON_ERR_NOTAVAR, "Not a variable." },\
	{ CON_ERR_COMMANDTOOLONG, "Command too long." },\
	{ CON_ERR_BADIPADDRESS, "Bad IP address." },\
	{ CON_ERR_INVALIDPASSWORD, "Invalid Password.", },\
	{ CON_ERR_BADIFACE, "Bad interface name." },\
	{ CON_ERR_BADNETWORKPARAM, "Error setting network parameter." }


#define CON_DISABLED			(-1)
#define CON_INIT				0
#define CON_LOGIN				1
#define CON_READY				2
#define CON_GETCOMMAND			3
#define CON_PARSECOMMAND		4
#define CON_EXECCOMMAND			5

/* states for the telnet console */
#define CONIO_TELNET_INIT		0
#define CONIO_TELNET_LISTEN	1
#define CONIO_TELNET_WAIT		2
#define CONIO_TELNET_RUNNING	3
#define CONIO_TELNET_RESET		4



int conio_COMM0_puts(char *str);
int COMM0_open( long baud );
void conio_COMM0_close(long baud);
int COMM0read( void *data, int length, unsigned long tmout );
int COMM0write( void *data, int lenght);
int COMM0rdUsed(void);
int COMM0wrUsed(void);
int COMM0wrFree(void);

char* con_getparam(char* buffer, int num);
ConsoleCommand* __con_parsecmd(char* command, int numparams, int* commandparams);
int __con_login(ConsoleState* state);
int __con_nullify(char* buffer);
void __con_error(ConsoleState* state, unsigned int errcode);
int __con_inputstring(ConsoleState* state);
void __conexec(ConsoleState* state);
void console_tick(void);
int console_init(void);
unsigned long con_set_timeout(unsigned int seconds);
int con_chk_timeout(unsigned long timeout);

int con_echo(ConsoleState* state);
int con_poll(ConsoleState* state);
int con_E2PROM_Write_numabo(ConsoleState* state);
int con_E2PROM_Write_rfansw(ConsoleState* state);
int con_E2PROM_read1(ConsoleState* state);
int con_E2PROM_Write_fmode(ConsoleState* state);
int con_E2PROM_Write_fmodelay(ConsoleState* state);
int con_DBGRF_mode(ConsoleState* state);
int con_DBGABORF_mode(ConsoleState* state);
int con_set_prevetimer(ConsoleState* state);
int con_rs232(ConsoleState* state);
int con_redbox(ConsoleState* state);
int con_E2PROM_Write_zone(ConsoleState* state);
int con_serialnum(ConsoleState* state);


int con_numabo(ConsoleState* state);
int con_numcen(ConsoleState* state);
int con_nomsuc(ConsoleState* state);
int con_cuenta1(ConsoleState* state);
int con_cuenta2(ConsoleState* state);
int con_ip(ConsoleState* state);
int con_netmask(ConsoleState* state);
int con_gateway(ConsoleState* state);
int con_macaddress(ConsoleState* state);
int con_server1(ConsoleState* state);
int con_port1(ConsoleState* state);
int con_hb1(ConsoleState* state);
int con_protocol1(ConsoleState* state);
int con_inuse1(ConsoleState* state);
int con_server2(ConsoleState* state);
int con_port2(ConsoleState* state);
int con_hb2(ConsoleState* state);
int con_protocol2(ConsoleState* state);
int con_inuse2(ConsoleState* state);
int con_cuentaR3K(ConsoleState* state);
int con_conf_ticket(ConsoleState* state);
int con_set_bisel(ConsoleState* state);
int con_set_TAS(ConsoleState* state);
int setpgm1(ConsoleState* state);
int resetpgm1(ConsoleState* state);
int setTAS(ConsoleState* state);
int resetTAS(ConsoleState* state);
int con_pulse_rele(ConsoleState* state);
int con_state_rele(ConsoleState* state);
int con_set_wdogevo(ConsoleState* state);
int con_wdevo_timer(ConsoleState* state);

int dump_ptmdev(ConsoleState* state);
int cfg3_ptmdev(ConsoleState* state);
int format_lan_cfgfile( ConsoleState* state );
int del_ptmdev(ConsoleState* state);
int helpcmd(ConsoleState* state);

int factorycmd( ConsoleState* state );
void FactoryPgm(void);

int con_paptslot(ConsoleState* state);
int con_set_ptxormask(ConsoleState* state);
int ptxorstate(ConsoleState* state);
int con_paparst_timer(ConsoleState* state);
int dump_vars(ConsoleState* state);
int send_manual_event(ConsoleState* state);
int con_version(ConsoleState* state);
int set_license(ConsoleState* state);

int con_set_radar(ConsoleState* state);

int measure_np(ConsoleState* state);
int measure_bat(ConsoleState* state);
void log_nivel_portadora(void);
int con_set_npmed(ConsoleState* state);

int con_set_bootmode(ConsoleState* state);
int con_calib(ConsoleState* state);
void calibrar( void );
int16_t get_vreal( int zone );
int con_replicar(ConsoleState* state);
int con_show_replicar(ConsoleState* state);
int con_phyeth(ConsoleState* state);

int con_tachof220v(ConsoleState* state);

int con_sirena(ConsoleState* state);
int con_cerradura(ConsoleState* state);
int con_st_cerradura(ConsoleState* state);
int con_st_puerta(ConsoleState* state);
int con_st_llavemecanica(ConsoleState* state);

int con_set_OPENPTM(ConsoleState* state);

int show_dhcp_info(ConsoleState* state);
int con_SetDHCP(ConsoleState* state);
int con_dhcpjumper(ConsoleState* state);

int conio_telnet_rdUsed(void);
int con_dflash_dump(ConsoleState* state);

void pgm_localip(uint8_t *strptr);
void pgm_netmask(uint8_t *strptr);
void pgm_gateway(uint8_t *strptr);
void pgm_macaddress(uint8_t *strptr);
void pgm_server1(uint8_t *strptr);
void pgm_server2(uint8_t *strptr);

int CypherCommands(ConsoleState* state);
int del_ptm( uint8_t ptm_partition);
int add_ptm( uint8_t ptm_partition);

int con_volumetrica(ConsoleState* state);
int con_armar(ConsoleState* state);

int con_prueballave(ConsoleState* state);
int con_txoff_largo(ConsoleState* state);
int con_txoff_corto(ConsoleState* state);

int con_dflash_dump(ConsoleState* state);
int con_ucflash_dump(ConsoleState* state);

int valid_license(void);

int con_showptmpwd(ConsoleState* state);
int con_rvb_dump_dev(ConsoleState* state);
int con_dump_monstruct(ConsoleState* state);
int con_licactivation(ConsoleState* state);
int con_licdeactivation(ConsoleState* state);
int con_radarctivation(ConsoleState* state);
int con_radardeactivation(ConsoleState* state);
int con_ASALTOactivation(ConsoleState* state);
int con_rhb(ConsoleState* state);
int con_rhb_activation(ConsoleState* state);
int con_rhb_deactivation(ConsoleState* state);
int con_timerhb(ConsoleState* state);
int con_timerhbcast(ConsoleState* state);
int con_ptmbuzzon(ConsoleState* state);
int con_ptmbuzzoff(ConsoleState* state);

int con_IP150_activation(ConsoleState* state);
int con_IP150_deactivation(ConsoleState* state);
int con_INPATTERN_activation(ConsoleState* state);
int con_INPATTERN_deactivation(ConsoleState* state);
int con_INPATTERNASK(ConsoleState* state);

int con_IRI_activation(ConsoleState* state);
int con_IRI_mode(ConsoleState* state);

int con_castrotu(ConsoleState* state);

void fsm_rhb_wdog( void );
int cfg3_pbttbl(ConsoleState* state);
int dump_pbttbl(ConsoleState* state);

int con_normalrm(ConsoleState* state);

int con_volredu(ConsoleState* state);
int con_show_redpar(ConsoleState* state);
int con_setNmax(ConsoleState* state);
int con_setAutoreset(ConsoleState* state);

int con_setNmaxASAL(ConsoleState* state);
int con_setNmaxTESO(ConsoleState* state);
int con_setNmaxINCE(ConsoleState* state);
int con_dumprtc2(ConsoleState* state);

int con_setBorDly(ConsoleState* state);
int con_setPtmDly(ConsoleState* state);
int con_E393HAB_activation(ConsoleState* state);
int con_sendvarsdly(ConsoleState* state);

int con_PPONWDOG(ConsoleState* state);

int con_remptmarm(ConsoleState* state);
int con_pakterrors(ConsoleState* state);

int con_rffilter1(ConsoleState* state);
int con_rffilter2(ConsoleState* state);

int con_delaydual(ConsoleState* state);
int con_DeltaT(ConsoleState* state);

extern const ConsoleIO console_io[];

extern unsigned char fsmrhbstate;
#define FSMRHB_IDLE		0x10
#define FSMRHB_TRGD		0x20
#define FSMRHB_PRETRGD	0x30
#define FSMRHB_MANCOP	0x40

#define TMINRHB			6
#define CASTILLETEDVR_PARTITION		66

#endif /* CONSOLE_H_ */





