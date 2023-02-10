
/*
 * console.c
 *  Created on: Oct 19, 2011
 *      Author: IronMan
 */
#include "includes.h"

int conio_telnet_port;
int __code_bytes_needed;

uint8_t	r3kmode;

uint8_t	curr_user_level;
uint8_t curr_user;

uint8_t SerialConsoleFlag;
uint16_t inconsole_timer;

//_TelnetCooker	conio_telnet_cooker;
//NET_SOCK_ID conio_telnet_sock, conio_telnet_sock_listen;
//int conio_telnet_state;

/*
 * This array defines the commands that are available in thDEBUGe console.
 * The first parameter for the ConsoleCommand structure is the
 * command specification--that is, the means by which the console
 * recognizes a command.  The second parameter is the function
 * to call when the command is recognized.  The third parameter is
 * the location of the #ximport'ed help file for the command.
 */

int con_hello(ConsoleState* state);
int con_hung(ConsoleState* state);

#define NP_UMBRAL	1665

const int npscale[26][2] =	{
		{100, 1561},
		{120, 1562},
		{140, 1563},
		{160, 1566},
		{180, 1572},
		{200, 1578},
		{220, 1582},
		{240, 1595},
		{260, 1605},
		{280, 1630},
		{300, 1665},
		{320, 1700},
		{340, 1740},
		{360, 1776},
		{380, 1815},
		{400, 1862},
		{420, 1892},
		{440, 1930},
		{460, 1970},
		{480, 2012},
		{500, 2052},
		{520, 2092},
		{540, 2132},
		{560, 2174},
		{580, 2208},
		{600, 2250}

};

const int batscale[21][2] =	{
		{200,  1578},
		{250,  1621},
		{300,  1665},
		{350,  1763},
		{400,  1862},
		{450,  1957},
		{500,  2052},
		{550,  2152},
		{600,  2250},
		{650,  2337},
		{700,  2426},
		{750,  2530},
		{800,  2622},
		{850,  2720},
		{900,  2820},
		{950,  2920},
		{1000, 3020},
		{1050, 3120},
		{1100, 3220},
		{1150, 3300},
		{1200, 3380}
};
const ConsoleCommand console_commands[] =
{
	{ "dflash", 		con_dflash_dump, 		0,		OPER_LEVEL},
	{ "ucflash", 		con_ucflash_dump, 		0,		OPER_LEVEL},
	{ "version", 		con_version, 			0,		OPER_LEVEL},
	{ "we2prom numabo", con_E2PROM_Write_numabo, 0,		MONI_LEVEL},
	{ "we2prom rfansw", con_E2PROM_Write_rfansw, 0,		MONI_LEVEL},
	{ "we2prom fmode", 	con_E2PROM_Write_fmode,	0,		MONI_LEVEL},
	{ "we2prom fmodly", con_E2PROM_Write_fmodelay, 0,	MONI_LEVEL},
	{ "we2prom zone", 	con_E2PROM_Write_zone, 	0,		MONI_LEVEL},
	{ "debug rf", 		con_DBGRF_mode, 		0,		OPER_LEVEL},
	{ "debug aborf", 	con_DBGABORF_mode,		0,		OPER_LEVEL},
    { "debug lan", 	con_DBGLAN_mode,		0,		OPER_LEVEL},
	{ "RDCONFMEM", 		con_E2PROM_read1,		0,		OPER_LEVEL},
	{ "set prevetimer", con_set_prevetimer, 	0,		MONI_LEVEL},

	//visualizacion de eventos
	{ "eyse_df_format",	con_EvDF_format,	0,		MCMI_LEVEL},
	{ "ev_hex_dump",	con_DumpEvMemory,		0,		MCMI_LEVEL},
	{ "ev_inx_dump",	con_DumpEvIndex,		0,		MCMI_LEVEL},
	{ "ev_time_dump",	con_DumpEventByTime,	0,		OPER_LEVEL},
	{ "set rtc",		con_setrtc,				0,		MONI_LEVEL},
	{ "...@#set rtc",	con_set1rtc,			0,		FREE_LEVEL},
	{ "...@#RHB",		con_rhb,				0,		FREE_LEVEL},
	{ "dump rtc",		con_dumprtc,			0,		OPER_LEVEL},
	{ "rtc?",			con_dumprtc2,			0,		OPER_LEVEL},

	//configuracion de rabbit
	{ "set numabo",		con_numabo,				0,		OPER_LEVEL},
	{ "set numcen",		con_numcen,				0,		OPER_LEVEL},
	{ "set nomsuc",		con_nomsuc,				0,		OPER_LEVEL},
	{ "set cuenta1",	con_cuenta1,			0,		MONI_LEVEL},
	{ "set cuenta2",	con_cuenta2,			0,		MONI_LEVEL},
	{ "set cuentar3k",	con_cuentaR3K,			0,		MONI_LEVEL},

	{ "set localip",	con_ip,					0,		OPER_LEVEL},
	{ "set netmask",	con_netmask,			0,		OPER_LEVEL},
	{ "set gateway",	con_gateway,			0,		OPER_LEVEL},
	{ "set macaddress",	con_macaddress,			0,		MONI_LEVEL},
	{ "set phymode",	con_phyeth,				0,		OPER_LEVEL},


	{ "set server1",	con_server1,			0,		OPER_LEVEL},
	{ "set port1",		con_port1,				0,		OPER_LEVEL},
	{ "set hb1",		con_hb1,				0,		MCMI_LEVEL},
	{ "set protocol1",	con_protocol1,			0,		MCMI_LEVEL},
	{ "set inuse1",		con_inuse1,				0,		OPER_LEVEL},

	{ "set server2",	con_server2,			0,		OPER_LEVEL},
	{ "set port2",		con_port2,				0,		OPER_LEVEL},
	{ "set hb2",		con_hb2,				0,		MCMI_LEVEL},
	{ "set protocol2",	con_protocol2,			0,		MCMI_LEVEL},
	{ "set inuse2",		con_inuse2,				0,		OPER_LEVEL},
	{ "H..P",           con_hung,               0,		OPER_LEVEL},

	{ "dump ptm",		dump_ptmdev,			0,		OPER_LEVEL},
	{ "del ptmdev",		del_ptmdev,				0,		MCMI_LEVEL},
	{ "config ptm",		cfg3_ptmdev,			0,		PROD_LEVEL},
	{ "format ptmtbl",	format_lan_cfgfile,		0,		MCMI_LEVEL},
    { "help",           helpcmd,                0,		OPER_LEVEL},
    { "factory",        factorycmd,             0,		MCMI_LEVEL},
    { "set r3kmode",   	con_rs232,   	        0,		OPER_LEVEL},
    { "redbox",   		con_redbox,   	        0,		MONI_LEVEL},
    { "set serialnum",	con_serialnum, 	        0,		PROD_LEVEL},
    { "set paptslot",	con_paptslot,			0,		MCMI_LEVEL},
    { "set pap_timer",	con_paparst_timer,		0,		MCMI_LEVEL},
//    { "set ptxormask",	con_set_ptxormask,		0,		MONI_LEVEL},
    { "ptxorstate",		ptxorstate,				0,		MONI_LEVEL},
    { "conf_ticket",	con_conf_ticket,		0,		OPER_LEVEL},
    { "set TAS",		con_set_TAS,			0,		OPER_LEVEL},
    { "set WDEVO",		con_set_wdogevo,		0,		MCMI_LEVEL},
    { "set wdevo_timer", con_wdevo_timer,		0,		MCMI_LEVEL},
    { "set RADAR",		con_set_radar,			0,		MONI_LEVEL},
    { "PGM1ON",			setpgm1,				0,		MONI_LEVEL},
    { "PGM1OFF",		resetpgm1,				0,		MONI_LEVEL},
    { "TASON",			setTAS,					0,		MONI_LEVEL},
    { "TASOFF",			resetTAS,				0,		MONI_LEVEL},
    { "RELEPULSE",		con_pulse_rele,			0,		MONI_LEVEL},
    { "RELESTATE",		con_state_rele,			0,		MONI_LEVEL},
    { "vars",			dump_vars,				0,		OPER_LEVEL},
    //{ "manual_event",	send_manual_event,		0,		MCMI_LEVEL},
    //{ "LIC",			set_license,			0,		FREE_LEVEL},
    { "set NPEVENT",	con_set_npmed,			0,		MONI_LEVEL},
    { "medir NP",		measure_np, 			0,		MONI_LEVEL},
    { "medir BAT",		measure_bat, 			0,		MONI_LEVEL},
    { "set BOOT",		con_set_bootmode, 		0,		MONI_LEVEL},
    { "calib",			con_calib,		 		0,		MONI_LEVEL},
    { "replicar",		con_replicar,	 		0,		OPER_LEVEL},
    { "show replicar",	con_show_replicar,	 	0,		OPER_LEVEL},
	{ "show redpar",	con_show_redpar,	 	0,		OPER_LEVEL},
	{ "set tachof220v",	con_tachof220v,		 	0,		MONI_LEVEL},
	{ "SIRENA",			con_sirena,			 	0,		FREE_LEVEL},
	{ "CERRADURA",		con_cerradura,		 	0,		FREE_LEVEL},
	{ "CERRADURA?",		con_st_cerradura,	 	0,		FREE_LEVEL},
	{ "PUERTA?",		con_st_puerta,		 	0,		FREE_LEVEL},
	{ "SLLAVE?",		con_st_llavemecanica, 	0,		FREE_LEVEL},
	{ "DHCP?",			show_dhcp_info,		 	0,		MONI_LEVEL},
	{ "set DHCP",		con_SetDHCP,		 	0,		MONI_LEVEL},
	{ "dhcpjumper",		con_dhcpjumper,		 	0,		MONI_LEVEL},
	{ "dflash",			con_dflash_dump,		0,		OPER_LEVEL},
	{ "toria1735",		con_audit_dump,			0,		MCMI_LEVEL},
	{ "set OPENPTM",	con_set_OPENPTM,		0,		OPER_LEVEL},
	//{ "ECM",			CypherCommands,			0,		OPER_LEVEL},
	{ "volumetricas",	con_volumetrica,		0,		OPER_LEVEL},
	{ "redpar",			con_volredu,			0,		OPER_LEVEL},
	{ "evo_armar",		con_armar,				0,		OPER_LEVEL},
	{ "prueba_llave",	con_prueballave,		0,		OPER_LEVEL},
	{ "TXOFF_largo",	con_txoff_largo,		0,		MONI_LEVEL},
	{ "TXOFF_corto",	con_txoff_corto,		0,		MONI_LEVEL},
	{ "show ptmpwd",	con_showptmpwd,			0,		FREE_LEVEL},
	{ "...@#dumpdev",	con_rvb_dump_dev,		0,		FREE_LEVEL},
	{ "monistruct",		con_dump_monstruct,		0,		FREE_LEVEL},
	{ "ActLicence",		con_licactivation,		0,		FREE_LEVEL},
	{ "PindongAL",		con_licdeactivation,	0,		FREE_LEVEL},
	{ "ActRadar",		con_radarctivation,		0,		FREE_LEVEL},
	{ "PindongAR",		con_radardeactivation,	0,		FREE_LEVEL},
	{ "...@#ActAsalto",	con_ASALTOactivation,	0,		FREE_LEVEL},
	{ "...@#ActRHB",	con_rhb_activation,		0,		FREE_LEVEL},
	{ "...@#DeactRHB",	con_rhb_deactivation,	0,		FREE_LEVEL},
	{ "set RHB",		con_timerhb,			0,		MCMI_LEVEL},
	{ "set RHBCAST",	con_timerhbcast,		0,		MCMI_LEVEL},
	{ "ptmbuzzon",		con_ptmbuzzon,			0,		MCMI_LEVEL},
	{ "ptmbuzzoff",	con_ptmbuzzoff,			0,		MCMI_LEVEL},
	{ "conf_pbt",		cfg3_pbttbl,			0,		MCMI_LEVEL},
	{ "dump pbt",		dump_pbttbl,			0,		MCMI_LEVEL},
    { "IP150_CHECK",	con_IP150_activation,	0,		MCMI_LEVEL},
    { "IP150_UNCHECK",	con_IP150_deactivation,	0,		MCMI_LEVEL},
	{ "INPATT_CHECK",	con_INPATTERN_activation,	0,	MCMI_LEVEL},
	{ "INPATT_UNCHECK",	con_INPATTERN_deactivation,	0,	MCMI_LEVEL},
    { "INPATT?",	    con_INPATTERNASK,	    0,	    MCMI_LEVEL},
	{ "set IRIDIUM",	con_IRI_activation,		0,		MCMI_LEVEL},
	{ "set IRIMODE",	con_IRI_mode,			0,		MCMI_LEVEL},
    { "set PPONWDOG",	con_PPONWDOG,			0,		MCMI_LEVEL},
	{ "castrotu",		con_castrotu,			0,		MCMI_LEVEL},
	{ "set NMAX",		con_setNmax,			0,		MCMI_LEVEL},
	{ "set NMAXASAL",	con_setNmaxASAL,		0,		MCMI_LEVEL},
	{ "set NMAXTESO",	con_setNmaxTESO,		0,		MCMI_LEVEL},
	{ "set NMAXINCE",	con_setNmaxINCE,		0,		MCMI_LEVEL},
	{ "set AUTORESET",	con_setAutoreset,		0,		MCMI_LEVEL},
    { "set BORDLY",	con_setBorDly,		    0,		MCMI_LEVEL},
    { "set PTMDLY",	con_setPtmDly,		    0,		MCMI_LEVEL},
    { "set E393HAB",	con_E393HAB_activation, 0,		MCMI_LEVEL},
    { "...@#SNDVDLY",	con_sendvarsdly,        0,		MCMI_LEVEL},
	{ "!@#NORM",		con_normalrm,			0,		MCMI_LEVEL},
    { "REMPTMARM",		con_remptmarm,			0,		MCMI_LEVEL},
    { "r485stat",      con_pakterrors,			0,		MCMI_LEVEL},
    { "rffilter1",     con_rffilter1,			0,		MCMI_LEVEL},
    { "rffilter2",     con_rffilter2,			0,		MCMI_LEVEL},
    { "delaydual",     con_delaydual,			0,		MCMI_LEVEL},
    { "DeltaT",        con_DeltaT,		    0,		MCMI_LEVEL},
//    { "ActEvLic",      con_evsend_activation, 0,		MCMI_LEVEL},
//    { "DeactEvLic",    con_evsend_deactivation,0,		MCMI_LEVEL},
    { "set NETRSTHAB",	con_netrsthab,		    0,		MCMI_LEVEL},
    { "set HIGRSTHAB",	con_higrsthab,		    0,		MCMI_LEVEL},
    { "gencid",	con_gencid,		    0,		MCMI_LEVEL},
    { "closerst",	con_closerst,		    0,		MCMI_LEVEL},
    { "closesoc",	con_closesoc,		    0,		MCMI_LEVEL},
    { "w",             con_evowdog,               0,		MONI_LEVEL},
    { "actince2",             con_ince2activation,               0,		MONI_LEVEL},
    { "deactince2",             con_ince2deactivation,               0,		MONI_LEVEL},
    { "ince2?",             con_ince2_status,               0,		MONI_LEVEL},
	{ "P",             con_poll,               0,		MONI_LEVEL}
};

const char *help_commands[] =
{
	"we2prom numabo ......... Programacion del nro de abonado con zonificacion",
	"we2prom rfansw ......... Programacion del tipo de respuesta por RF",
	"we2prom fmode .......... Programacion de respuesta rapida o normal",
	"we2prom zone ........... Programacion del numero de zona",
	"debug rf ............... Visualizacion del trafico por RF",
	"debug aborf ............ Visualizacion del trafico por RF, solo del aboando",
	"set prevetimer ......... Configura el tacho de preve en minutos",

	//visualizacion de eventos
	"ev_inx_dump ............ Dump de memoria de eventos por indice",
	"ev_time_dump ........... Dump de memoria de efentos por fecha y hora",
	"set rtc ................ Puesta en fecha y hora",
	"dump rtc ............... Muestra fecha y hora",

	//configuracion de rabbit
	"set numabo ............. Numero de abonado para el monitoreo",
	"set numcen ............. Numero de central para el monitoreo",
	"set nomsuc ............. Nombre de sucursal",
	"set cuenta1 ............ Cuenta CID para bunker 1",
	"set cuenta2 ............ Cuenta CID para bunker 2",
	"set cuentar3k .......... Cuenta CID para GPRS - R3K",

	"set localip ............ Configuracion de ip de la placa",
	"set netmask ............ Configuracion de la mascara de red de la placa",
	"set gateway ............ Configuracion del gateway de sucursal",
	"set macaddress ......... Configuracion de macaddress de la placa",
	"set phymode ............ Configuracion manual de velocidad ethernet",

	"set server1 ............ Direccion ip del host en bunker 1",
	"set port1 .............. Puerto de recepcion del host del bunker 1",
	"set hb1 ................ Tiempo entre heartbeats para el bunker 1",
	"set protocol1 .......... Protocolo de monitoreo para bunker 1",
	"set inuse1 ............. Activacion del monitoreo en bunker 1",

	"set server2 ............ Direccion ip del host en bunker 2",
	"set port2 .............. Puerto de recepcion del host del bunker 2",
	"set hb2 ................ Tiempo entre heartbeats para el bunker 2",
	"set protocol2 .......... Protocolo de monitoreo para bunker 2",
	"set inuse2 ............. Activacion del monitoreo en bunker 2",
	"H..P ................... Reseteo de la placa",

	"dump ptm ............... Muestra la tabla de configuracion de los PTm",
	"del ptmdev ............. Borra una entrada en la tabla de PTm",
	"config ptm ............. Configurar y dar de alta un PTm",
	"format ptmtbl .......... Formateo inicial de la tabla de PTm",
    "help ................... Muestra esta ayuda",
    "factory ................ Configuracion de fabrica",
    "set r3kmode ............ Modo de reporte por rs232 a rabbit o gprs",
    "redbox ................. Habilitacion / Inhabilitacion de caja roja",
    "set serialnum .......... Programa el numero de serie del abonado",
    "set paptslot ........... Configura modo PAP",
    "set pap_timer .......... Programacion del tiempo autoreset para PAP",
    "set ptxormask .......... Prog directa de mascara de inversion de estados PT",
    "ptxorstate ............. Configuracion del estado de inversion de los PT",
    "conf_ticket ............ Imprime la configuracion completa del abonado",
    "set bisel .............. Configura modo BISEL",
    "set TAS ................ Configura modo TAS",
    "set WDEVO .............. Activa/Desactiva el watch dog para paneles de alarma",
    "set wdevo_timer ........ Tiempo en minutos del watchdog para paneles de alarma",
    "RELEPULSE .............. Genera un pulso de 2 seg en un rele de expansion",
    "set RADAR .............. Activa/Desactiva la apertura por radar",
    "RELESTATE .............. Activa o Desactiva el rele indicado en exp de reles",
	"P ...................... Simula una encuesta por RF",
	"version ................ Muestra la version de firmware de la placa",
	"set NPEVENT ............ Activa/Desactiva la generacion de evento por NP bajo",
	"set BOOT ............... Activar bootloader en proximo reset",
	"calib .................. Calibracion para medicion de zonas",
	"replicar ............... Monta particion 485 sobre borneras delanteras",
	"show replicar .......... Muestra tabla de replicacion de particiones",
	"set tachof220v ......... Fija el tacho para F220V en minutos",
	"DHCP? .................. Muestra las ip obtenidas por DHCP",
	"set DHCP ............... Activa / Desactiva el uso de DHCP",
    "~"
};

const ConsoleIO console_io[] =
{
		{ COMM0, COMM0_open, conio_COMM0_close, NULL, conio_COMM0_puts, COMM0rdUsed, COMM0wrUsed, COMM0wrFree, COMM0read, COMM0write },
        { 777, NULL, NULL, NULL, conio_udpserver_puts, NULL, NULL, NULL, conio_udpserver_read, conio_udpserver_write }
		//{ 23, conio_telnet_open, conio_telnet_close, conio_telnet_tick, conio_telnet_puts, conio_telnet_rdUsed, NULL, NULL, conio_telnet_read, conio_telnet_write }
};

const ConsoleError console_errors[] = {
	CON_STANDARD_ERRORS
};

ConsoleState __constate[NUM_CONSOLES];
ConsoleLogin console_login[CONSOLE_USERS];
void (*__con_user_timeout)();
void (*__con_user_idle)();

//--------------------------------------------------------------------------------------------------------




int conio_COMM0_puts(char *str)
{
	CommSendString( COMM0, str);
	return strlen(str);
}


int COMM0_open( long baud )
{
	InitUART_Debug();
	return 1;
}

void conio_COMM0_close(long baud)
{
	return;
}

int COMM0read( void *data, int length, unsigned long tmout )
{
	return ComGetBuff(COMM0, (CPU_INT16U)tmout,(CPU_INT08U *)data, length);
}

int COMM0write( void *data, int lenght)
{
	ComSendBuff(COMM0, (CPU_INT08U *)data, lenght);
	return lenght;
}

int COMM0rdUsed(void)
{
	return COMMXrdUsed(COMM0);
}

int COMM0wrUsed(void)
{
	return COMMXwrUsed(COMM0);
}

int COMM0wrFree(void)
{
	return COMMXwrFree(COMM0);
}


/* START FUNCTION DESCRIPTION ********************************************
console_init                           <ZCONSOLE.LIB>

SYNTAX: int console_init(void);

KEYWORDS:		tcpip

DESCRIPTION:	Initializes console data structures.  This function must
					be called before console_tick() is called for the first
					time.  This function also loads the configuration
					information from the file system.

RETURN VALUE:	0		successfully initialized console
					1		no configuration information found
					< 0	indicates an error loading the configuration
					      data.  -1 indicates an error reading the 1st
					      set of information, -2 the 2nd set, and so on.

SEE ALSO:		console_tick

END DESCRIPTION **********************************************************/
const char *nPRODUCCION = "PRODUCCION";
const char *nINSTALADOR = "INSTALADOR";
const char *nPREVENTIVO = "PREVENTIVO";
const char *nCORRECTIVO = "CORRECTIVO";

int console_init(void)
{
	auto int i;

	__con_user_timeout = NULL;
	__con_user_idle = NULL;

	console_login[0].name = (char *)nPRODUCCION;
	console_login[1].name = (char *)nINSTALADOR;
	console_login[2].name = (char *)nPREVENTIVO;
	console_login[3].name = (char *)nCORRECTIVO;

	console_login[0].password = "1234";
	console_login[1].password = "000000";
	console_login[2].password = "2679";
	console_login[3].password = "5555";

	console_login[0].level = 0x0F;
	console_login[1].level = 0x07;
	console_login[2].level = 0x03;
	console_login[3].level = 0x01;

	curr_user_level = 0;

//	for(i = 0; i < CONSOLE_USERS; i++)	{
//		console_login[i].name[0]= '\0';
//		console_login[i].password[0]='\0';
//	}

	for (i = 0; i < NUM_CONSOLES; i++) {
		__constate[i].console_number = i;
		__constate[i].conio = (ConsoleIO *)(&(console_io[i]));
		__constate[i].state = CON_INIT;
		__constate[i].echo = 1;
		__constate[i].current_user = 100;
	}

	return 0;
}

/* START FUNCTION DESCRIPTION ********************************************
console_tick                           <ZCONSOLE.LIB>

SYNTAX: void console_tick(void);

KEYWORDS:		tcpip

DESCRIPTION:	Allows the console to process requests.  This function
					should be called periodically.

RETURN VALUE:	none

SEE ALSO:		console_init

END DESCRIPTION **********************************************************/

void console_tick(void)
{
	auto int i;

	for (i = 0; i < NUM_CONSOLES; i++) {
		__conexec(&__constate[i]);
	}
}

void __conexec(ConsoleState* state)
{
	auto int retval;
	uint8_t temp[16];
	uint32_t error;

	if(state->state==CON_DISABLED)
		return;

	if (state->state != state->laststate) {
		state->laststate = state->state;
		state->timeout = con_set_timeout(CON_TIMEOUT);
	}

	if (con_chk_timeout(state->timeout)) {
		if (state->state <= CON_GETCOMMAND) {
			// never mind--we're waiting for a command or initializing
			state->timeout = con_set_timeout(CON_TIMEOUT);
		} else {
			if (__con_user_timeout != NULL) {
				__con_user_timeout(state);
			}
			__con_error(state, CON_ERR_TIMEOUT);
			state->state = CON_READY;
		}
	}

	/* run the driver, if necessary */
	if((NULL != state->conio->tick) && (state->state != CON_INIT)) {
		switch(state->conio->tick())	{
			case 0:
				state->state = CON_INIT;
				break;
			case 1:
				break;
			case 2:
				state->state = CON_INIT;
				break;
		}


	}



	switch (state->state) {
	case CON_INIT:
		//state->conio->open(console_io.param[state->console_number]);
		state->conio->puts(CON_INIT_MESSAGE);
		state->substate=0;

		if(console_login[0].name[0])
			state->state = CON_LOGIN;
		else
			state->state = CON_READY;
		break;

	case CON_LOGIN:
		switch(__con_login(state)) {
			case -1:
				state->substate=0;
				break;
			case 1:
				state->state=CON_READY;
				break;
			case 0:
				break;
		}
		break;

	case CON_READY:
		state->command[0] = '\0';
		state->cmdptr = state->command;
		state->sawesc = 0;
		state->state = CON_GETCOMMAND;
		break;

	case CON_GETCOMMAND:
		switch(__con_inputstring(state)) {
			case -1:
				state->state=CON_READY;
				break;
			case 1:
				state->state=CON_PARSECOMMAND;
				break;
			case 0:
				break;
		}
		break;

	case CON_PARSECOMMAND:
		state->numparams = __con_nullify(state->command);
		if (state->numparams == 0) {
			state->state = CON_READY;
			break;
		}
		state->cmdspec = __con_parsecmd(state->command, state->numparams,
		                                &(state->commandparams));
		if ((state->cmdspec != NULL) && (state->cmdspec->cmdfunc != NULL)) {
			state->substate = 0;
			state->state = CON_EXECCOMMAND;

		} else {
			__con_error(state, CON_ERR_BADCOMMAND);
			state->state = CON_READY;
		}
		break;

	case CON_EXECCOMMAND:
		//-----------------------------------------------------------------------------------------
#ifndef  USAR_LICENSIA	
		if(!(SystemFlag6 & USE_LICENSE))	{
			RADAR_flags |= LIC_ENTER;		// !!!! OJO: para que no necesite licensia - Despues sacar.
		}
#endif
		//-----------------------------------------------------------------------------------------
		
		if( (state->cmdspec->qualyfier <= IbuttonTable[lic_ibuttonid].CommandLevel) ||  (!(SystemFlag6 & USE_LICENSE)))	{
			if( (state->cmdspec->qualyfier <= 1) || (!(SystemFlag6 & USE_LICENSE)) )	{	//el nivel 0, son comandos abiertos a todo el mundo
				RADAR_flags |= CONSOLE_CMDIN;			//el nivel 1, es por ej. el comando LIC
				retval = state->cmdspec->cmdfunc(state);
				if (retval == 1) {
					state->conio->puts("OK\r\n");
					state->state = CON_READY;
				} else if (retval == -1) {
					__con_error(state, state->error);
					state->state = CON_READY;
				}
			} else 	{									//para comandos con niveles >= 2, se requiere haber ingresado licensia previamente.
				if( valid_license() )	{
					RADAR_flags |= CONSOLE_CMDIN;
					RADAR_flags |= LIC_ENTER;
					retval = state->cmdspec->cmdfunc(state);
					if (retval == 1) {
						state->conio->puts("OK\r\n");
						state->state = CON_READY;
					} else if (retval == -1) {
						__con_error(state, state->error);
						state->state = CON_READY;
					}
				} else	{
					state->conio->puts("*** Licencia Vencida en tiempo ***\r\n");
					state->conio->puts("*** Solicite una nueva licencia***\r\n");
					state->state = CON_READY;
					temp[0] = 0xA5;         //se acabo la licensia para el login
                    error = flash0_write(1, temp, LOGIN_ENABLED, 1);
				}
			}
		} else	{
			state->conio->puts("* ERROR: Su Nivel de Seguridad no permite ejecutar este comando\r\n");
		}
		state->state = CON_READY;
		break;

	default:
		/* ERROR! -- Should _never_ get here */
		state->state = CON_INIT;
		break;
	}
}

int __con_inputstring(ConsoleState* state)
{
	auto int retval;
	auto char data;

	if (__con_user_timeout != NULL) {
		__con_user_timeout(state);
	}
	if (state->conio->rdUsed() > 0) {
		retval = state->conio->read(&data, 1, 0uL);
		if(0 == retval)
			return 0;
		state->timeout = con_set_timeout(CON_TIMEOUT);
	} else {
		return 0;
	}

	if (state->sawesc > 0) {
		// Skip past escaped characters (starting with 1B, such as cursor
		// movement keys)
		state->sawesc--;
		return 0;
	} else if ((data == '\r') || ((data == '\n') && (state->sawcr == 0))) {
		if (data == '\r') {
			state->sawcr = 1;
		}
		if (state->echo) {
			state->conio->puts("\r\n");
		}
		if ((state->cmdptr - state->command) < CON_CMD_SIZE) {
			*(state->cmdptr) = '\0';
		} else {
			__con_error(state, CON_ERR_COMMANDTOOLONG);
			return -1;
		}

		// find the command
		return 1;
	} else if ((data == '\n') && (state->sawcr == 1)) {
		state->sawcr = 0;
		return 0;
	} else if (state->sawcr == 1) {
		state->sawcr = 0;
	}
	// Handle the character we just got
	if ((data == 0x08) || (data == 0x7f)) {
		// Backspace character
		if (state->cmdptr > state->command) {
			state->cmdptr--;
			if (state->echo) {
				state->conio->puts("\x08 \x08");
			}
		}
	} else if (data == 0x1B) {
		// Escape character--skip past the next two characters
		state->sawesc = 2;
	} else if (data == 0x15) {
		// CTRL-U -- linefeed and clear the command buffer
		if (state->echo) {
			state->conio->puts("\r\n");
		}
		state->cmdptr = state->command;
		state->command[0] = '\0';
	} else if (data >= 0x20) {
		if ((state->cmdptr - state->command) < CON_CMD_SIZE) {
			*(state->cmdptr) = data;
			inconsole_timer = 3*60;
		}
		state->cmdptr++;
		if (state->echo) {
			switch(state->echo)	{
			case '*' :
				state->conio->puts("*");
				break;
			default:
				state->conio->write(&data, 1);
				break;
			}
		}
	} else if( data == CONSOLEOUT_CTRLCHAR)	{
		SerialConsoleFlag &= ~INSERCON_flag;
		RADAR_flags |= CONSOLE_OUT;
		CommSendString(DEBUG_COMM, "\n***  END OF SERIAL CONSOLE MODE  ***\n\r");
	}
	return 0;
}

void __con_error(ConsoleState* state, unsigned int errcode)
{
	auto int i;
	auto char errstr[7];

	for (i = 0; i < (sizeof(console_errors) / sizeof(ConsoleError)); i++) {
		if (console_errors[i].code == errcode) {
			state->conio->puts("ERROR ");
			sprintf(errstr, "%04u  ", errcode);
			state->conio->puts(errstr);
			state->conio->puts(console_errors[i].message);
			state->conio->puts("\r\n");
			break;
		}
	}
}

int __con_nullify(char* buffer)
{
	auto int i;
	auto int looking;
	auto int quoted;
	auto int num;

	looking = 0;
	quoted = 0;
	num = 0;
	for (i = 0; buffer[i] != '\0'; i++) {
		if ((looking == 0) && (buffer[i] == ' ')) {
			buffer[i] = '\0';
		} else if ((looking == 0) && (buffer[i] != ' ')) {
			//
			// If we have an empty string fill it with
			// a 0xff.  This character will be ignored
			// when retrieving the parameter.
			//
			if (buffer[i] == '\"') {
				if (buffer[i+1] == '\"') {
					buffer[i++]=0xff;
					buffer[i]='\0';
				} else {
					quoted = 1;
					buffer[i] = '\0';
				}
			}
			looking = 1;
			num++;
		} else if ((looking == 1) && (quoted == 0) &&
		           ((buffer[i] == ' ') || (buffer[i] == '\"'))) {
		   if (buffer[i] == '\"') {
		   	// Quote in an unquoted param; start new param
		   	quoted = 1;
		   	looking = 1;
		   	num++;
		   } else {
		   	looking = 0;
		   }
			buffer[i] = '\0';
		} else if ((looking == 1) && (quoted == 1) && (buffer[i] == '\"')) {
			// Ending a quoted param
			looking = 0;
			quoted = 0;
			buffer[i] = '\0';
		}
		// Handle escaped characters
		if (buffer[i] == '\\') {
			strcpy(&buffer[i], &buffer[i+1]);
		}
	}
	return num;
}

int __con_login(ConsoleState* state)
{
		OS_ERR	err;


		return 1;


	switch(state->substate) {
		case 0:		/* initialize... write login prompt */
			if(console_login[0].name[0]=='\0') // no password just return
				return 1;

			state->conio->puts("login:  ");
			state->substate=1;

			state->command[0] = '\0';
			state->cmdptr = state->command;
			state->sawesc = 0;
			break;

		case 1: // login
			switch(__con_inputstring(state)) {
				case -1:
					return -1;

				case 1:
					//Usuario Produccion
					if(strcmp(console_login[0].name,state->command)==0) {
						if(console_login[0].password[0] =='\0') {
							return 1;
						} else {
							state->conio->puts("password:  ");
							curr_user = 0;
							state->command[0] = '\0';
							state->cmdptr = state->command;
							state->sawesc = 0;
							state->echo = '*';
							state->substate=2;
						}
					} else
					//Usuario Instalador
					if(strcmp(console_login[1].name,state->command)==0)	{
						if(console_login[1].password[0] =='\0') {
							return 1;
						} else {
							state->conio->puts("password:  ");
							curr_user = 1;
							state->command[0] = '\0';
							state->cmdptr = state->command;
							state->sawesc = 0;
							state->echo = '*';
							state->substate=2;
						}
					} else
					//Usuario Preventivo
					if(strcmp(console_login[2].name,state->command)==0)	{
						if(console_login[2].password[0] =='\0') {
							return 1;
						} else {
							state->conio->puts("password:  ");
							curr_user = 2;
							state->command[0] = '\0';
							state->cmdptr = state->command;
							state->sawesc = 0;
							state->echo = '*';
							state->substate=2;
						}
					} else
					//Usuario Correctivo
					if(strcmp(console_login[3].name,state->command)==0)	{
						if(console_login[3].password[0] =='\0') {
							return 1;
						} else {
							state->conio->puts("password:  ");
							curr_user = 3;
							state->command[0] = '\0';
							state->cmdptr = state->command;
							state->sawesc = 0;
							state->echo = '*';
							state->substate=2;
						}
					}
					else {
						state->conio->puts("password:  ");
						state->command[0] = '\0';
						state->cmdptr = state->command;
						state->sawesc = 0;
						state->echo = '*';
						state->substate=3;
					}
					break;

				case 0:
					break;
			}
			break;

		case 2: // password
			switch(__con_inputstring(state)) {
				case -1:
					state->echo = 1;
					return -1;

				case 1:
					state->echo = 1;

					if(strcmp(console_login[curr_user].password,state->command)==0) {
						state->conio->puts("\r\nPassword Accepted\r\n");
						switch(curr_user)	{
						case 0:
							curr_user_level = 0x01;
							break;
						case 1:
							curr_user_level = 0x02;
							break;
						case 2:
							curr_user_level = 0x04;
							break;
						case 3:
							curr_user_level = 0x08;
							break;
						default:
							curr_user_level = 0x00;
							break;
						}
						state->command[0] = '\0';
						state->cmdptr = state->command;
						state->sawesc = 0;
						return 1;
					} else {
						state->conio->puts("\r\nInvalid Password\r\n");
						*((long*)state->cmddata)= OSTimeGet(&err) + 1000;
						state->substate=4;
					}
					break;

				case 0:
					break;
			}
			break;

		case 3: // invalid login get password
			switch(__con_inputstring(state)) {
				case -1:
					state->echo = 1;
					curr_user_level = 0x00;
					return -1;

				case 1:
					state->echo = 1;
					state->conio->puts("\r\nInvalid Password\r\n");
					*((long*)state->cmddata)= OSTimeGet(&err) + 1000;
					state->substate=4;
					curr_user_level = 0x00;
					break;

				case 0:
					break;
			}
			break;

		case 4: // timeout bad password
			if((long)(*((long*)state->cmddata)-OSTimeGet(&err))<=0) {
				state->substate=0;
			}
			break;
	}
	return 0;
}


ConsoleCommand* __con_parsecmd(char* command, int numparams,
                               int* commandparams)
{
	auto int i;
	auto int part;
	auto char* ptr;
	auto char* endptr;
	auto int n;
	auto int match;
	auto int match_parts;
	auto int still_matches;
	auto char* cmdptr;

	match = -1;
	match_parts = -1;
	for (i = 0; i < (sizeof(console_commands) / sizeof(ConsoleCommand)); i++) {
		part = 0;
		ptr = console_commands[i].command;
		still_matches = 1;
		do {
			endptr = memchr(ptr, ' ', strlen(ptr));
			if (endptr != NULL) {
				n = endptr - ptr;
			} else {
				n = strlen(ptr);
			}
			if (part < numparams) {
				cmdptr = con_getparam(command, part);
				if ((strlen(cmdptr) == n) &&
				    (Str_Cmp_N(ptr, cmdptr, strlen(cmdptr)) == 0)) {
					// It matches
					part++;
					ptr += n + 1;
				} else {
					// It doesn't match
					still_matches = 0;
				}
			} else {
				if (numparams == 0) {
					// Check for "" command (mainly for the help system)
					if ((*console_commands[i].command) != '\0') {
						still_matches = 0;
					}
				} else {
					// Trivially doesn't match, since the number of parameters on the
					// command line is less than the parts of the command we're testing
					still_matches = 0;
				}
			}
		} while ((endptr != NULL) && (still_matches == 1));
		if ((still_matches == 1) && (part > match_parts)) {
			match = i;
			match_parts = part;
		}
	}
	if (match != -1) {
		if (commandparams != NULL) {
			*commandparams = numparams - match_parts;
		}

		return ((ConsoleCommand*)&console_commands[match]);
	} else {
		return NULL;
	}
}

char* con_getparam(char* buffer, int num)
{
	auto int numnulls;
	auto int i;

	numnulls = 0;
	i = 0;
	// Skip past initial nulls
	while (buffer[i] == '\0') {
		i++;
	}
	// Skip to the right parameter
	while (numnulls < num) {
		if ((buffer[i] == '\0') && (buffer[i+1] != '\0')) {
			numnulls++;
		}
		i++;
	}

	//
	// Ignore the 0xff which is added when
	// we hit an empty string.
	//
	if(buffer[i]==0xff)
		i++;

	return (&buffer[i]);
}

unsigned long con_set_timeout(unsigned int seconds)
{
	OS_ERR err;

	return (OSTimeGet(&err) + seconds*1000);
}

int con_chk_timeout(unsigned long timeout)
{
	OS_ERR err;
	return (OSTimeGet(&err) >= timeout);
}

int con_echo(ConsoleState* state)
{
	auto int temp;
	temp = state->numparams - state->commandparams;
	if ((state->commandparams == 1) &&
	    (Str_Cmp(con_getparam(state->command, temp), "on") == 0)) {
		state->echo = 1;
		return 1;
	} else if ((state->commandparams == 1) &&
	           (Str_Cmp(con_getparam(state->command, temp), "off") == 0)) {
		state->echo = 0;
		return 1;
	} else {
		state->error = CON_ERR_BADPARAMETER;
		return -1;
	}
}

//int con_hello(ConsoleState* state)
//{
//	auto int retval;
//	uint8_t buffer[8];
//        int len;
//
//	buffer[0] = 0x00;
//	len = flash0_write(1, buffer, DF_FIDUDT_OFFSET, 1);
//
//	if( (retval = state->conio->puts("Hello, Claudio. I'm fine.\n\rHow can I help you?\n\r")) > 0)
//		return 1;
//	else return -2;
//
//}

int con_version(ConsoleState* state)
{
	auto int retval;

	if( (retval = state->conio->puts(VERSION_ABO)) > 0)
		return 1;
	else return -2;

}

int con_poll(ConsoleState* state)
{
	autoreset_data = 0xEF;
	SysFlag0 |= RF_POLL_flag;
	SysFlag0 |= FSMTX_flag;
	SystemFlag3 |= NAPER_RFPOLL;
	return 1;
}

int con_evowdog(ConsoleState* state)
{
    EVOWD_Flag |= (1 << 0);
    EVOWD_Flag |= (1 << 1);
    EVOWD_Flag |= (1 << 2);
    return 1;
}



int con_hung(ConsoleState* state)
{
	LLAVE_TX_OFF();
	POWER_TX_OFF();

	while(1);

	return 1;
}
int con_set_prevetimer(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set prevetimer xx\n\r");
		retval = EepromReadByte(PREVETIMER_E2P_ADDR, &error);
			state->conio->puts("PREVETIMER (min) = ");
			conio_printHexWord(state, retval);
			state->conio->puts("\n\r");
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	if((numabo >= 0) && (numabo <= 17))
		EepromWriteByte(PREVETIMER_E2P_ADDR, (uint8_t)numabo, &error);
	else	{
		state->conio->puts("*** PREVETIMER debe estar entre 0 y 17 minutos ***\n\r");
		return -1;
	}
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	retval = EepromReadByte(PREVETIMER_E2P_ADDR, &error);
	if(numabo != retval)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(PREVETIMER_E2P_ADDR, (uint8_t)numabo, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		retval = EepromReadByte(PREVETIMER_E2P_ADDR, &error);
	}
	state->conio->puts("PREVETIMER (min) = ");
	conio_printHexWord(state, retval);
	state->conio->puts("\n\r");

	return 1;
}

int con_showptmpwd(ConsoleState* state)
{
	char temp[32];
	int value;


	Mem_Clr(temp, 32);
	Str_Cat(temp, itoa(ptm_pwd)); Str_Cat(temp, "\n\r");
	state->conio->puts(temp);


	return 1;
}

int con_E2PROM_Write_numabo(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval, tzone;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("WE2PROM NUMABO nro_abonado\n\r");
		retval = EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
			state->conio->puts("PGMD NUMABO = ");
			conio_printHexWord(state, retval);
			state->conio->puts("\n\r");
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	if((numabo > 0) && (numabo <= 200))
		EepromWriteByte(RF_NUMABO_E2P_ADDR, (uint8_t)numabo, &error);
	else	{
		state->conio->puts("*** SUBSCRIBER NUMBER ERROR  ***\n\r");
		return -1;
	}
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	retval = EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
	if(numabo != retval)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(RF_NUMABO_E2P_ADDR, (uint8_t)numabo, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		retval = EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
	}
	state->conio->puts("PGMD NUMABO = ");
	conio_printHexWord(state, retval);
	state->conio->puts("\n\r");
#ifdef TABLAS_ABO_ON
	if(retval == numabo)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		tzone = (uint8_t)EepromReadByte(ZONE_E2P_ADDR, &error);
		switch(tzone)	{
		case 1:
			BaseAlarmPkt_numabo = numabo_z1[numabo-1];
			break;
		case 2:
			BaseAlarmPkt_numabo = numabo_z2[numabo-1];
			break;
		case 3:
			BaseAlarmPkt_numabo = numabo_z3[numabo-1];
			break;
		case 4:
			BaseAlarmPkt_numabo = numabo_z4[numabo-1];
			break;
		case 5:
			BaseAlarmPkt_numabo = numabo_z5[numabo-1];
			break;
		case 6:
			BaseAlarmPkt_numabo = numabo_z6[numabo-1];
			break;
		}
	}

	if(BaseAlarmPkt_numabo >= 80)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)4, &error);
		TypeAboAns = 4;
	} else	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)3, &error);
		TypeAboAns = 3;
	}
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(FMODEON_ADDR, (uint8_t)0x01, &error);
	SysFlag3 |= FMODE_flag;

#else
	if(retval == numabo)
		BaseAlarmPkt_numabo = numabo;
#endif

	logCidEvent(account, 1, 670, 0, numabo );
	return 1;
}

int con_E2PROM_Write_fmodelay(ConsoleState* state)
{
	uint16_t fmodelay;
	uint8_t retval;
	int error;

	if( state->numparams < 3 )	{
		state->conio->puts("WE2PROM FMODLY dly (en mSeg)\n\r");
		retval = EepromReadByte(FMODELAY_ADDR, &error);
			state->conio->puts("PGMD DLY = ");
			conio_printHexWord( state, retval);
			state->conio->puts("\n\r");
		return 1;
	}
	fmodelay = atoi(con_getparam(state->command, 2));
	if((fmodelay >= 0) && (fmodelay <= 255))
		EepromWriteByte(FMODELAY_ADDR, (uint8_t)fmodelay, &error);
	else	{
		state->conio->puts("*** FMODE DELAY ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(FMODELAY_ADDR, &error);
	state->conio->puts("PGMD DLY = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\r");
	if(retval == fmodelay)
		FmodeDelay = fmodelay;
	logCidEvent(account, 1, 671, 0, fmodelay );
	return 1;
}

int con_E2PROM_Write_rfansw(ConsoleState* state)
{
	uint16_t rfanswtype;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("WE2PROM RFANSW tipo\n\r");
		retval = EepromReadByte(RF_ANSWER_TYPE, &error);
			state->conio->puts("PGMD RFANSW = ");
			conio_printHexWord( state, retval);
			state->conio->puts("\n\r");
		return 1;
	}
	rfanswtype = atoi(con_getparam(state->command, 2));

	if((rfanswtype > 0) && (rfanswtype < 8))
		EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)rfanswtype, &error);
	else	{
		state->conio->puts("*** RF ANSW TYPE ERROR  ***\n\r");
		return -1;
	}

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	retval = EepromReadByte(RF_ANSWER_TYPE, &error);
	if(retval != rfanswtype)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)rfanswtype, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		retval = EepromReadByte(RF_ANSWER_TYPE, &error);
	}
	state->conio->puts("PGMD RFANSW = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\r");
	if((retval == rfanswtype) && (rfanswtype > 0) && (rfanswtype < 8))
		TypeAboAns = rfanswtype;

	logCidEvent(account, 1, 681, 0, rfanswtype );
	return 1;
}

int con_E2PROM_read1(ConsoleState* state)
{
	uint8_t tempbuffer[32];
	int error;

	//EepromReadBuffer(RF_NUMABO_E2P_ADDR, tempbuffer, 32, &error );
	EepromReadBuffer(LOCAL_IP_E2P_ADDR, tempbuffer, 16, &error );
	dumpMemory(tempbuffer, 16);
	CommSendString(DEBUG_COMM, "\n\r");
	EepromReadBuffer(NETMASK_E2P_ADDR, tempbuffer, 16, &error );
	dumpMemory(tempbuffer, 16);
	CommSendString(DEBUG_COMM, "\n\r");
	EepromReadBuffer(GATEWAY_E2P_ADDR, tempbuffer, 16, &error );
	dumpMemory(tempbuffer, 16);
	CommSendString(DEBUG_COMM, "\n\r"); CommSendString(DEBUG_COMM, "\n\r");

	EepromReadBuffer(IPADDCHK_E2P_ADDR, tempbuffer, 6, &error );

	printByte(tempbuffer[0]);
	CommPutChar(DEBUG_COMM,' ',0);
	printByte(tempbuffer[1]);
	CommPutChar(DEBUG_COMM,' ',0);
	printByte(tempbuffer[2]);
	CommPutChar(DEBUG_COMM,' ',0);
	printByte(tempbuffer[3]);
	CommPutChar(DEBUG_COMM,' ',0);
	printByte(tempbuffer[4]);
	CommPutChar(DEBUG_COMM,' ',0);
	printByte(tempbuffer[5]);
	CommPutChar(DEBUG_COMM,' ',0);
	CommSendString(DEBUG_COMM, "\n\r");

	return 1;
}



int con_E2PROM_Write_fmode(ConsoleState* state)
{
	uint16_t fmodetype;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("WE2PROM FMODE tipo\n\r");
		retval = EepromReadByte(FMODEON_ADDR, &error);
			state->conio->puts("PGMD FMODE = ");
			conio_printHexWord( state, retval);
			state->conio->puts("\n\r");
		return 1;
	}
	fmodetype = atoi(con_getparam(state->command, 2));

	switch(fmodetype)	{
		case 0:
			EepromWriteByte(FMODEON_ADDR, (uint8_t)0x00, &error);
			SysFlag3 &= ~FMODE_flag;
			break;
		case 1:
			EepromWriteByte(FMODEON_ADDR, (uint8_t)0x01, &error);
			SysFlag3 |= FMODE_flag;
			break;
		default:
			state->conio->puts("*** FAST MODE TYPE ERROR  ***\n\r");
			return -1;
			break;
	}


	retval = EepromReadByte(FMODEON_ADDR, &error);

	if(retval != fmodetype)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(FMODEON_ADDR, (uint8_t)fmodetype, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		retval = EepromReadByte(FMODEON_ADDR, &error);
	}

	state->conio->puts("PGMD FMODE = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\r");
	if(retval == 0x01)
		SysFlag3 |= FMODE_flag;
	else if(retval == 0x00)
		SysFlag3 &= ~FMODE_flag;
	return 1;
}

int con_E2PROM_Write_zone(ConsoleState* state)
{
	uint16_t zone;
	uint8_t retval, numabo;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("WE2PROM ZONE zona\n\r");
		retval = EepromReadByte(ZONE_E2P_ADDR, &error);
			state->conio->puts("PGMD ZONE = ");
			conio_printHexWord( state, retval);
			state->conio->puts("\n\r");
		return 1;
	}
	zone = atoi(con_getparam(state->command, 2));
	if(zone > 7)	{
		state->conio->puts("*** ERROR EN NRO DE ZONA  ***\n\r");
		return -1;
	}

	EepromWriteByte(ZONE_E2P_ADDR, (uint8_t)zone, &error);

	retval = EepromReadByte(ZONE_E2P_ADDR, &error);

	if(retval != zone)	{
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(ZONE_E2P_ADDR, (uint8_t)zone, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		retval = EepromReadByte(ZONE_E2P_ADDR, &error);
	}

	state->conio->puts("PGMD ZONE = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\r");

#ifdef TABLAS_ABO_ON
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	numabo = (uint8_t)EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
	switch(retval)	{
	case 1:
		BaseAlarmPkt_numabo = numabo_z1[numabo-1];
		break;
	case 2:
		BaseAlarmPkt_numabo = numabo_z2[numabo-1];
		break;
	case 3:
		BaseAlarmPkt_numabo = numabo_z3[numabo-1];
		break;
	case 4:
		BaseAlarmPkt_numabo = numabo_z4[numabo-1];
		break;
	case 5:
		BaseAlarmPkt_numabo = numabo_z5[numabo-1];
		break;
	case 6:
		BaseAlarmPkt_numabo = numabo_z6[numabo-1];
		break;
	}

	if(BaseAlarmPkt_numabo >= 80)	{
		EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)4, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		TypeAboAns = 4;
	} else	{
		EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)3, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		TypeAboAns = 3;
	}

	EepromWriteByte(FMODEON_ADDR, (uint8_t)0x01, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	SysFlag3 |= FMODE_flag;
#endif

	logCidEvent(account, 1, 672, 0, zone );
	return 1;
}

int con_DBGRF_mode(ConsoleState* state)
{
	uint16_t fmodetype;

	if( state->numparams < 3 )	{
		state->conio->puts("DEBUG RF 0|1\n\r");
		return 1;
	}
	fmodetype = atoi(con_getparam(state->command, 2));

	switch(fmodetype)	{
		case 0:
			DebugFlag &= ~DBGRF_ON_flag;
			break;
		case 1:
			DebugFlag |= DBGRF_ON_flag;
			break;
		default:
			state->conio->puts("*** DEBUG RF ERROR  ***\n\r");
			return -1;
			break;
	}

	return 1;
}

int con_DBGLAN_mode(ConsoleState* state)
{
    uint16_t fmodetype;

    if( state->numparams < 3 )	{
        state->conio->puts("DEBUG LAN 0|1\n\r");
        return 1;
    }
    fmodetype = atoi(con_getparam(state->command, 2));

    switch(fmodetype)	{
        case 0:
            DebugFlag &= ~LAN485DBG_flag;
            break;
        case 1:
            DebugFlag |= LAN485DBG_flag;
            break;
        default:
            state->conio->puts("*** DEBUG LAN ERROR  ***\n\r");
            return -1;
            break;
    }

    return 1;
}

//int con_perifpwr(ConsoleState* state)
//{
//    uint16_t fmodetype;
//
//    if( state->numparams < 2 )	{
//        state->conio->puts("perifpwr 0|1\n\r");
//        return 1;
//    }
//    fmodetype = atoi(con_getparam(state->command, 1));
//
//    switch(fmodetype)	{
//        case 0:
//            GPIO_ClearValue(2, (1 << 8));
//            break;
//        case 1:
//            GPIO_SetValue(2, (1 << 8));
//            break;
//        default:
//            state->conio->puts("*** PERIF PWR ERROR  ***\n\r");
//            return -1;
//            break;
//    }
//
//    return 1;
//}

int con_DBGABORF_mode(ConsoleState* state)
{
	uint16_t fmodetype;

	if( state->numparams < 3 )	{
		state->conio->puts("DEBUG ABORF 0|1\n\r");
		return 1;
	}
	fmodetype = atoi(con_getparam(state->command, 2));

	switch(fmodetype)	{
		case 0:
			DebugFlag &= ~DBGABORF_flag;
			break;
		case 1:
			DebugFlag |= DBGABORF_flag;
			break;
		default:
			state->conio->puts("*** DEBUG RF ERROR  ***\n\r");
			return -1;
			break;
	}

	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////
// Funciones de configuracion de rabbit


int con_numabo(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set numabo nro_abonado\n\r");
		retval = EepromReadByte(MON_NUMABO_E2P_ADDR, &error);
			Str_Cat(buffer, "PGMD MON NUMABO = ");
			//sprintHexWord(tmpbuf, retval);
			tmpbuf = itoa(retval);
			Str_Cat(buffer, tmpbuf);
			Str_Cat(buffer, "\n\r");
			state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo > 0) && (cnumabo < 200))
		EepromWriteByte(MON_NUMABO_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** MON SUBSCRIBER NUMBER ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(MON_NUMABO_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD MON NUMABO = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	if(retval == cnumabo)
		NumAbo = cnumabo;


	return 1;
}

int con_numcen(ConsoleState* state)
{
	uint16_t numcen;
	uint8_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set numcen nro_central\n\r");
		retval = EepromReadByte(RFNUMCEN_E2P_ADDR, &error);
			Str_Cat(buffer, "PGMD MON NUMCEN = ");
			//sprintHexWord(tmpbuf, retval);
			tmpbuf = itoa(retval);
			Str_Cat(buffer, tmpbuf);
			Str_Cat(buffer, "\n\r");
			state->conio->puts(buffer);
		return 1;
	}
	numcen = atoi(con_getparam(state->command, 2));
	if((numcen > 0) && (numcen < 200))
		EepromWriteByte(RFNUMCEN_E2P_ADDR, (uint8_t)numcen, &error);
	else	{
		state->conio->puts("*** MON CENTRAL NUMBER ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(RFNUMCEN_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD MON NUMCEN = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	if(retval == numcen)
		NumCen = numcen;


	return 1;
}

int con_nomsuc(ConsoleState* state)
{
	int len, error, i;
	char buffer[33];


	if( state->numparams < 3 )	{
		state->conio->puts("set nomsuc \"nombre de sucursal\"\n\r");
		EepromReadBuffer(NOMSUC_E2P_ADDR, buffer, 32, &error);
		state->conio->puts("PGMD NOMSUC: ");
		for(i = 0; i < 33; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[32] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) ) + 1;

	if( len > 32 )	{
		state->conio->puts("ERROR *** El nombre de la sucursal no debe ser mayor a 32 caracteres\n\r");
		return 1;
	}


	EepromWriteBuffer(NOMSUC_E2P_ADDR, con_getparam(state->command, 2), len, &error);
	EepromReadBuffer(NOMSUC_E2P_ADDR, buffer, len, &error);
	buffer[32] = 0;

	//state->conio->puts(buffer);

	state->conio->puts("PGMD NOMSUC: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}

	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//Str_Copy_N(NomSuc, buffer, len);

	return 1;
}

int con_cuenta1(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set cuenta1 nro_cuenta1\n\r");
		retval = EepromReadWord(CIDACCOUNT1_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD CUENTA1 = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 0) && (cuenta < 10000))
		EepromWriteWord(CIDACCOUNT1_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** ACCOUT 1 NUMBER ERROR  ***\n\r");
		return -1;
	}

	logCidEvent(account, 1, 680, 0, 1 );

	retval = EepromReadWord(CIDACCOUNT1_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD CUENTA1 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
    account = EepromReadWord(CIDACCOUNT1_E2P_ADDR, &error);

//	if(retval == cuenta)
//		hbaccount[0] = cuenta;



	return 1;
}

int con_cuenta2(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set cuenta2 nro_cuenta2\n\r");
		retval = EepromReadWord(CIDACCOUNT2_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD CUENTA2 = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 0) && (cuenta < 10000))
		EepromWriteWord(CIDACCOUNT2_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** ACCOUT 2 NUMBER ERROR  ***\n\r");
		return -1;
	}

	logCidEvent(account, 1, 680, 0, 2 );

	retval = EepromReadWord(CIDACCOUNT2_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD CUENTA2 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

//	if(retval == cuenta)
//		hbaccount[1] = cuenta;



	return 1;
}


int con_cuentaR3K(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set cuentar3k nro_cuenta\n\r");
		retval = EepromReadWord(R3KACCOUNT_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD CUENTA R3K = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 0) && (cuenta < 10000))
		EepromWriteWord(R3KACCOUNT_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** ACCOUT R3K NUMBER ERROR  ***\n\r");
		return -1;
	}

	logCidEvent(account, 1, 680, 0, 3 );

	retval = EepromReadWord(R3KACCOUNT_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD CUENTA R3K = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
    r3kaccount = cuenta;

	return 1;
}

void pgm_localip(uint8_t *strptr)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	len = strlen(strptr);
	if( len > 15 )
		return;

	Str_Copy_N( buffer, strptr, len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(LOCAL_IP_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(IPADDCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MIPADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_MIPCHK_OFFSET, 1 );	

}

void pgm_netmask(uint8_t *strptr)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	len = strlen(strptr);
	if( len > 15 )
		return;

	Str_Copy_N( buffer, strptr, len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(NETMASK_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(NMASKCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MNMASK_OFFSET, 16 );	
	flash0_write(1, temp, DF_MNMCKH_OFFSET, 1 );

}

void pgm_gateway(uint8_t *strptr)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	len = strlen(strptr);
	if( len > 15 )
		return;

	Str_Copy_N( buffer, strptr, len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(GATEWAY_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(GWADDCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MGWADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_MGWCHK_OFFSET, 1 );

}

void pgm_macaddress(uint8_t *strptr)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	len = strlen(strptr);
	if( len > 7 )
		return;

	Str_Copy_N( buffer, strptr, 6);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 6; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(MACADDCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MACADD_OFFSET, 6 );	
	flash0_write(1, temp, DF_MACCHK_OFFSET, 1 );

}

void pgm_server1(uint8_t *strptr)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	len = strlen(strptr);
	if( len > 15 )
		return;

	Str_Copy_N( buffer, strptr, len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(SERVER1_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(SRV1ACHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_SVR1ADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_SR1CHK_OFFSET, 1 );	

}

void pgm_server2(uint8_t *strptr)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	len = strlen(strptr);
	if( len > 15 )
		return;

	Str_Copy_N( buffer, strptr, len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(SERVER2_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(SRV2ACHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_SVR2ADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_SR2CHK_OFFSET, 1 );	

}


int con_ip(ConsoleState* state)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	if( state->numparams < 3 )	{
		state->conio->puts("set localip \"xxx.xxx.xxx.xxx\"\n\r");
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, buffer, 16, &error);
		state->conio->puts("PGMD LOCALIP: ");
		for(i = 0; i < 16; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[15] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );

	if( len > 15 )	{
		state->conio->puts("ERROR *** WRONG IP ADDRESS FORMAT\n\r");
		return 1;
	}

	Str_Copy_N( buffer, con_getparam(state->command, 2), len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(LOCAL_IP_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(IPADDCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MIPADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_MIPCHK_OFFSET, 1 );

	flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x01;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);


	EepromReadBuffer(LOCAL_IP_E2P_ADDR, buffer, len+1, &error);
	buffer[15] = 0;


	state->conio->puts("PGMD LOCALIP: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] > 127)	{
			if(buffer[i] == 0x00 )
				break;
			state->conio->puts("\n\r");
			return 1;
		}
	}

	state->conio->puts(buffer);
	state->conio->puts("\n\r");

//	Str_Copy_N(LocalIP, buffer, len);

	logCidEvent(account, 1, 676, 0, 0 );
	return 1;
}

int con_netmask(ConsoleState* state)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	if( state->numparams < 3 )	{
		state->conio->puts("set netmask \"xxx.xxx.xxx.xxx\"\n\r");
		EepromReadBuffer(NETMASK_E2P_ADDR, buffer, 16, &error);
		state->conio->puts("PGMD NETMASK: ");
		for(i = 0; i < 16; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[15] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );

	if( len > 15 )	{
		state->conio->puts("ERROR *** WRONG IP ADDRESS FORMAT\n\r");
		return 1;
	}

	Str_Copy_N( buffer, con_getparam(state->command, 2), len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(NETMASK_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(NMASKCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MNMASK_OFFSET, 16 );	
	flash0_write(1, temp, DF_MNMCKH_OFFSET, 1 );

	flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x02;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);


	EepromReadBuffer(NETMASK_E2P_ADDR, buffer, len+1, &error);
	buffer[15] = 0;

	//state->conio->puts(buffer);

	state->conio->puts("PGMD NETMASK: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] > 127)	{
			if(buffer[i] == 0x00 )
				break;
			state->conio->puts("\n\r");
			return 1;
		}
	}

	state->conio->puts(buffer);
	state->conio->puts("\n\r");

//	Str_Copy_N(Netmask, buffer, len);


	return 1;
}

int con_gateway(ConsoleState* state)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	if( state->numparams < 3 )	{
		state->conio->puts("set gateway \"xxx.xxx.xxx.xxx\"\n\r");
		EepromReadBuffer(GATEWAY_E2P_ADDR, buffer, 16, &error);
		state->conio->puts("PGMD GATEWAY: ");
		for(i = 0; i < 16; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[15] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );

	if( len > 15 )	{
		state->conio->puts("ERROR *** WRONG IP ADDRESS FORMAT\n\r");
		return 1;
	}

	Str_Copy_N( buffer, con_getparam(state->command, 2), len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(GATEWAY_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(GWADDCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MGWADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_MGWCHK_OFFSET, 1 );

	flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x04;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);


	EepromReadBuffer(GATEWAY_E2P_ADDR, buffer, len+1, &error);
	buffer[15] = 0;

	//state->conio->puts(buffer);

	state->conio->puts("PGMD GATEWAY: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] > 127)	{
			if(buffer[i] == 0x00 )
				break;
			state->conio->puts("\n\r");
			return 1;
		}
	}

	state->conio->puts(buffer);
	state->conio->puts("\n\r");

//	Str_Copy_N(Gateway, buffer, len);

	return 1;
}

int con_macaddress(ConsoleState* state)
{
	int len, error, i, j;
	uint8_t buffer[6], strbuffer[16], tmpbuffer[3];
	uint8_t temp, temp1[8];
	uint16_t chksum;
	OS_ERR os_err;

	for(i = 0; i < 6; i++)
		buffer[i] = 0x00;

	strbuffer[0] = 0;
	if( state->numparams < 3 )	{
		state->conio->puts("set macaddress \"HHHHHHHHHHHH\"\n\r");
		EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
		state->conio->puts("PGMD MACADDRESS: ");
		for(i = 0; i < 6; i++ )	{
			sprintByte(tmpbuffer, buffer[i]);
			Str_Cat(strbuffer, tmpbuffer);
		}
		strbuffer[12] = 0;
		state->conio->puts(strbuffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );

	if( len != 12 )	{
		state->conio->puts("ERROR *** WRONG  MACADDRESS FORMAT\n\r");
		return 1;
	}

	Str_Copy(strbuffer, con_getparam(state->command, 2));

	for(i = 0, j =0; i < 6; i++, j+=2 )	{
		temp = htoi_nibble(strbuffer[j]) * 0x10;
		temp += htoi_nibble(strbuffer[j+1]);
		buffer[i] = temp;
	}

	chksum = 0x0000;
	for(i = 0; i < 6; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp1[0] = chksum;

	EepromWriteBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(MACADDCHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_MACADD_OFFSET, 6 );	
	flash0_write(1, temp1, DF_MACCHK_OFFSET, 1 );

	flash0_read(temp1, DF_FIDUDT_OFFSET, 1);
	temp1[0] |= 0x08;
	flash0_write(1, temp1, DF_FIDUDT_OFFSET, 1 );
	temp1[0] = 0x5A;
	flash0_write(1, temp1, DF_FIDUMK_OFFSET, 1);


	EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);

	strbuffer[0] = 0;
	state->conio->puts("PGMD MACADDRESS: ");
	for(i = 0; i < 6; i++ )	{
		sprintByte(tmpbuffer, buffer[i]);
		Str_Cat(strbuffer, tmpbuffer);
	}
	strbuffer[12] = 0;
	state->conio->puts(strbuffer);
	state->conio->puts("\n\r");

//	for(i = 0; i < 6; i++ )	{
//		NetIF_MAC_Addr[i] = buffer[i];
//	}

	logCidEvent(account, 1, 677, 0, 0 );
	return 1;
}

int con_server1(ConsoleState* state)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	OS_ERR os_err;
        uint16_t chksum;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	if( state->numparams < 3 )	{
		state->conio->puts("set server1 \"xxx.xxx.xxx.xxx\"\n\r");
		EepromReadBuffer(SERVER1_E2P_ADDR, buffer, 16, &error);
		state->conio->puts("PGMD SERVER1: ");
		for(i = 0; i < 16; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[15] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );

	if( len > 15 )	{
		state->conio->puts("ERROR *** WRONG IP ADDRESS FORMAT\n\r");
		return 1;
	}

	Str_Copy_N( buffer, con_getparam(state->command, 2), len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;


	EepromWriteBuffer(SERVER1_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(SRV1ACHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_SVR1ADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_SR1CHK_OFFSET, 1 );
        
    flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x10;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);

	EepromReadBuffer(SERVER1_E2P_ADDR, buffer, len+1, &error);
	buffer[15] = 0;

	//state->conio->puts(buffer);

	state->conio->puts("PGMD SERVER1: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] > 127)	{
			if(buffer[i] == 0x00 )
				break;
			state->conio->puts("\n\r");
			return 1;
		}
	}

	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//Str_Copy_N(Server1, buffer, len);

	logCidEvent(account, 1, 678, 0, 1 );
	return 1;
}

int con_port1(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];
	uint8_t temp[8];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set port1 nro_port1\n\r");
		retval = EepromReadWord(PORT1_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PORT1 = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 50) && (cuenta < 0xFFFF))
		EepromWriteWord(PORT1_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** PORT 1 NUMBER ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(PORT1_E2P_ADDR, &error);

    temp[0] = (retval >> 8) & 0x00FF;
    temp[1] = retval & 0x00FF;
    flash0_write(1, temp, DF_PORT1_OFFSET, 2 );

    flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x40;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);

	Str_Cat(buffer, "PGMD PORT1 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

//	if(retval == cuenta)
//		sockPort[0] = cuenta;

	logCidEvent(account, 1, 679, 0, 1 );
	return 1;
}

int con_hb1(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set hb1 hearbeat_en_segundos\n\r");
		retval = EepromReadWord(HBTTIME1_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD HB1 = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 5) && (cuenta < 0xFFFF))
		EepromWriteWord(HBTTIME1_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** HB OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(HBTTIME1_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD HB1 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

//	if(retval == cuenta)
//		heartBeatTime[0] = cuenta;

	return 1;
}

int con_protocol1(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set protocol1 1|2|3|4 (1:NETIO_4, 2:NETIO_5, 3:NETIO_6, 4:NETIO_7)\n\r");
		retval = EepromReadByte(PROTCOL1_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PROTOCOL1 = ");
		switch(retval)	{
		case 1:
			Str_Cat(buffer, "NETIO_4\n\r");
			break;
		case 2:
			Str_Cat(buffer, "NETIO_5\n\r");
			break;
		case 3:
			Str_Cat(buffer, "NETIO_6\n\r");
			break;
		case 4:
			Str_Cat(buffer, "NETIO_7\n\r");
			break;
		default:
			Str_Cat(buffer, "PROTOCOL TYPE ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo > 0) && (cnumabo < 5))
		EepromWriteByte(PROTCOL1_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** PROTOCOL TYPE ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(PROTCOL1_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PROTOCOL1 = ");
	switch(retval)	{
	case 1:
		Str_Cat(buffer, "NETIO_4\n\r");
//		coProtocol[0] = AP_NTSEC4;
//		sockProtocol[0] = IPPROTO_UDP;
//		sockType[0] = SOCK_DGRAM;
		break;
	case 2:
		Str_Cat(buffer, "NETIO_5\n\r");
//		coProtocol[0] = AP_NTSEC5;
//		sockProtocol[0] = IPPROTO_UDP;
//		sockType[0] = SOCK_DGRAM;
		break;
	case 3:
		Str_Cat(buffer, "NETIO_6\n\r");
//		coProtocol[0] = AP_EYSE1;
//		sockProtocol[0] = IPPROTO_TCP;
//		sockType[0] = SOCK_STREAM;
		break;
	case 4:
		Str_Cat(buffer, "NETIO_7\n\r");
		break;
	default:
		Str_Cat(buffer, "PROTOCOL TYPE ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	return 1;
}

int con_inuse1(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set inuse1 0|1 (0:OFF, 1:ON)\n\r");
		retval = EepromReadByte(INUSE1_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD INUSE1 = ");
		switch(retval)	{
		case 0:
			Str_Cat(buffer, "OFF\n\r");
			break;
		case 1:
			Str_Cat(buffer, "ON\n\r");
			break;
		default:
			Str_Cat(buffer, "INUSE ACTIVATION ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo >= 0) && (cnumabo < 2))
		EepromWriteByte(INUSE1_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** INUSE ACTIVATION ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(INUSE1_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD INUSE1 = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "OFF\n\r");
		//inUse[0] = FALSE;
		break;
	case 1:
		Str_Cat(buffer, "ON\n\r");
		//inUse[0] = TRUE;
		break;
	default:
		Str_Cat(buffer, "INUSE ACTIVATION ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	return 1;
}

int con_server2(ConsoleState* state)
{
	int len, error, i;
	uint8_t buffer[16], temp[8];
	OS_ERR os_err;
        uint16_t chksum;

	for(i = 0; i < 16; i++)
		buffer[i] = 0x00;

	if( state->numparams < 3 )	{
		state->conio->puts("set server2 \"xxx.xxx.xxx.xxx\"\n\r");
		EepromReadBuffer(SERVER2_E2P_ADDR, buffer, 16, &error);
		state->conio->puts("PGMD SERVER2: ");
		for(i = 0; i < 16; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[15] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );

	if( len > 15 )	{
		state->conio->puts("ERROR *** WRONG IP ADDRESS FORMAT\n\r");
		return 1;
	}

	Str_Copy_N( buffer, con_getparam(state->command, 2), len);
	buffer[len] = 0;

	chksum = 0x0000;
	for(i = 0; i < 16; i++)	{
		chksum += buffer[i];
	}
	chksum &= 0x00FF;
	temp[0] = chksum;

	EepromWriteBuffer(SERVER2_E2P_ADDR, buffer, len+1, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(SRV2ACHK_E2P_ADDR, (uint8_t)chksum, &error);
	OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);

	//ahora grabo la ip de la dflash
	flash0_write(1, buffer, DF_SVR2ADD_OFFSET, 16 );	
	flash0_write(1, temp, DF_SR2CHK_OFFSET, 1 );

        flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x20;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);

	EepromReadBuffer(SERVER2_E2P_ADDR, buffer, len+1, &error);
	buffer[15] = 0;

	//state->conio->puts(buffer);

	state->conio->puts("PGMD SERVER2: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] > 127)	{
			if(buffer[i] == 0x00 )
				break;
			state->conio->puts("\n\r");
			return 1;
		}
	}

	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//Str_Copy_N(Server2, buffer, len);

	logCidEvent(account, 1, 678, 0, 2 );
	return 1;
}

int con_port2(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];
	uint8_t temp[8];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set port2 nro_port2\n\r");
		retval = EepromReadWord(PORT2_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PORT2 = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 50) && (cuenta < 0xFFFF))
		EepromWriteWord(PORT2_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** PORT 2 NUMBER ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(PORT2_E2P_ADDR, &error);

    temp[0] = (retval >> 8) & 0x00FF;
    temp[1] = retval & 0x00FF;
    flash0_write(1, temp, DF_PORT2_OFFSET, 2 );

    flash0_read(temp, DF_FIDUDT_OFFSET, 1);
	temp[0] |= 0x40;
	flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
	temp[0] = 0x5A;
	flash0_write(1, temp, DF_FIDUMK_OFFSET, 1);

	Str_Cat(buffer, "PGMD PORT2 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

//	if(retval == cuenta)
//		sockPort[1] = cuenta;

	logCidEvent(account, 1, 679, 0, 2 );
	return 1;
}

int con_hb2(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set hb2 hearbeat_en_segundos\n\r");
		retval = EepromReadWord(HBTTIME2_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD HB2 = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 5) && (cuenta < 0xFFFF))
		EepromWriteWord(HBTTIME2_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** HB OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(HBTTIME2_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD HB2 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

//	if(retval == cuenta)
//		heartBeatTime[1] = cuenta;

	return 1;
}

int con_protocol2(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set protocol2 1|2|3|4 (1:NETIO_4, 2:NETIO_5, 3:NETIO_6,  4:NETIO_7)\n\r");
		retval = EepromReadByte(PROTCOL2_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PROTOCOL2 = ");
		switch(retval)	{
		case 1:
			Str_Cat(buffer, "NETIO_4\n\r");
			break;
		case 2:
			Str_Cat(buffer, "NETIO_5\n\r");
			break;
		case 3:
			Str_Cat(buffer, "NETIO_6\n\r");
			break;
		case 4:
			Str_Cat(buffer, "NETIO_7\n\r");
			break;
		default:
			Str_Cat(buffer, "PROTOCOL TYPE ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo > 0) && (cnumabo < 5))
		EepromWriteByte(PROTCOL2_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** PROTOCOL TYPE ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(PROTCOL2_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PROTOCOL2 = ");
	switch(retval)	{
	case 1:
		Str_Cat(buffer, "NETIO_4\n\r");
//		coProtocol[1] = AP_NTSEC4;
//		sockProtocol[1] = IPPROTO_UDP;
//		sockType[1] = SOCK_DGRAM;
		break;
	case 2:
		Str_Cat(buffer, "NETIO_5\n\r");
//		coProtocol[1] = AP_NTSEC5;
//		sockProtocol[1] = IPPROTO_UDP;
//		sockType[1] = SOCK_DGRAM;
		break;
	case 3:
		Str_Cat(buffer, "NETIO_6\n\r");
//		coProtocol[1] = AP_EYSE1;
//		sockProtocol[1] = IPPROTO_TCP;
//		sockType[1] = SOCK_STREAM;
		break;
	case 4:
		Str_Cat(buffer, "NETIO_7\n\r");
		break;
	default:
		Str_Cat(buffer, "PROTOCOL TYPE ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	return 1;
}

int con_inuse2(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set inuse2 0|1 (0:OFF, 1:ON\n\r");
		retval = EepromReadByte(INUSE2_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD INUSE2 = ");
		switch(retval)	{
		case 0:
			Str_Cat(buffer, "OFF\n\r");
			break;
		case 1:
			Str_Cat(buffer, "ON\n\r");
			break;
		default:
			Str_Cat(buffer, "INUSE ACTIVATION ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo >= 0) && (cnumabo < 2))
		EepromWriteByte(INUSE2_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** INUSE ACTIVATION ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(INUSE2_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD INUSE2 = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "OFF\n\r");
		//inUse[1] = FALSE;
		break;
	case 1:
		Str_Cat(buffer, "ON\n\r");
		//inUse[1] = TRUE;
		break;
	default:
		Str_Cat(buffer, "INUSE ACTIVATION ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	return 1;
}

//configuracion de los ptm que funcionan como switch remotos
int cfg3_pbttbl(ConsoleState* state)
{
	uint8_t tblindex, particion, prevparticion;
	uint16_t zona, cuenta;
	uint8_t buffer[16];
	int i, chksum;
	uint32_t address;

	tblindex = atoi( con_getparam(state->command, 1) );
	particion = atoi( con_getparam(state->command, 2) );
	zona = atoi( con_getparam(state->command, 3) );
	cuenta = atoi( con_getparam(state->command, 4) );

	if((tblindex < 0) || (tblindex > 7))
		return 1;

	buffer[0] = (uint8_t)particion;
	buffer[1] = (uint8_t)( (zona >> 8) & 0x00FF );
	buffer[2] = (uint8_t)( zona & 0x00FF );
	buffer[3] = (uint8_t)( (cuenta >> 8) & 0x00FF );
	buffer[4] = (uint8_t)( cuenta & 0x00FF );

	chksum = 0;
	for(i = 0; i < 5; i++)
		chksum += buffer[i];
	buffer[5] = (uint8_t)( chksum & 0x00FF);

	address = DF_PBTTBL0 + tblindex*DF_PBTREGLEN;
	flash0_write(1, buffer, address, 6 );

	prevparticion = pbt_dcb[tblindex].particion;
	pbt_dcb[tblindex].particion = particion;
	pbt_dcb[tblindex].zona = zona;
	pbt_dcb[tblindex].cuenta = cuenta;	

	if(particion)
		logCidEvent(account, 1, 890, 0, particion );
	else
		logCidEvent(account, 3, 890, 0, prevparticion );

	return 1;
}

int dump_pbttbl(ConsoleState* state)
{
	uint8_t tblindex, particion;
	uint16_t zona, cuenta;
	int i, chksum;
	uint32_t address;
	char buffer[128], tmpbuffer[6];

	state->conio->puts( "\n\rTabla de Data Control Block de los Pulsadores\n\r" );
	state->conio->puts(     "---------------------------------------------\n\r\n" );

	for( i = 0; i < MAXPBTPTM; i++)	{
		buffer[0] = 0;
		Str_Cat(buffer,"DEV: "); Str_Cat(buffer,itoa(i)); Str_Cat(buffer, ",\t");
		if(i < 10)
			Str_Cat(buffer,"\t");

		sprintByte(tmpbuffer, pbt_dcb[i].particion);
		Str_Cat(buffer,"PARTICION: "); Str_Cat(buffer,itoa(pbt_dcb[i].particion)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, ")\t");

		sprintByte(tmpbuffer, pbt_dcb[i].zona);
		Str_Cat(buffer,"ZONA: "); Str_Cat(buffer,itoa(pbt_dcb[i].zona)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, ")\t");

		sprintHexWord(tmpbuffer, pbt_dcb[i].cuenta);
		Str_Cat(buffer," CUENTA: "); Str_Cat(buffer,itoa(pbt_dcb[i].cuenta)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, "),\t");

		Str_Cat(buffer,"\n\r");
		state->conio->puts(buffer);

	}

	state->conio->puts( "\n\r");
	return 1;
}

//------------------------------------------------------------------------------------------------
//de la configuracion de los ptm

int cfg3_ptmdev(ConsoleState* state)
{
	unsigned char rtuaddr, devnum, particion;
    char disp, *disptr;
	unsigned int cuenta, error;
	uint8_t buffer[5];
	uint16_t address;


	if( state->numparams != 7 )	{
		state->conio->puts("\tUsage : config ptm devnum rtuaddr cuenta particion disp\n\r\tej: config ptm 05 27 1234 15 T\n\r");
		state->conio->puts("\t\t devnum: 	nro de modulo en el rango de 0 a 24\n\r");
		state->conio->puts("\t\t rtuaddr:   dirrecion en red LAN, rango de 1 a 254\n\r");
		state->conio->puts("\t\t cuenta:    Numero de cuenta del PTm en el sistema de monitoreo, rango 0000 a 9999\n\r");
		state->conio->puts("\t\t particion: Numero de particion del PTm, rango 00 a 99\n\r");
		state->conio->puts("\t\t disp:      Tipo de disparo, -: ninguno\n\r");
		state->conio->puts("\t\t                             S: Modo TAS\n\r");
		return 1;
	}

	devnum = atoi( con_getparam(state->command, 2) );
	rtuaddr = atoi( con_getparam(state->command, 3) );
	cuenta = atoi( con_getparam(state->command, 4) );
	particion = atoi( con_getparam(state->command, 5) );
	disptr = con_getparam(state->command, 6);
    disp = disptr[0];

	if( (rtuaddr < 1) || (rtuaddr > 254) )	{
		state->conio->puts("*** Error en la direccion del PTm\n\r");
		return 1;
	}
	if( particion > 99 )	{
		state->conio->puts("*** Error en el numero de particion\n\r");
		return 1;
	}
	if( cuenta > 9999 )	{
		state->conio->puts("*** Error en el numero de cuenta\n\r");
		return 1;
	}
	if( (devnum < 0) || (devnum > 24) )	{
		state->conio->puts("*** Error en el numero de dispositivo\n\r");
		return 1;
	}

	switch(disp)	{
	case '-':
	case 'S':
		break;
	default:
		state->conio->puts("*** Error en el tipo de disparo\n\r");
		return 1;
	}

	ptm_dcb[devnum].rtuaddr = rtuaddr;
	ptm_dcb[devnum].cuenta = cuenta;
	ptm_dcb[devnum].particion = particion;
	ptm_dcb[devnum].disparo = disp;

	buffer[0] = ptm_dcb[devnum].rtuaddr;
	buffer[1] = ptm_dcb[devnum].particion;
	buffer[2] = (ptm_dcb[devnum].cuenta >> 8) & 0x00FF;
	buffer[3] = ptm_dcb[devnum].cuenta & 0x00FF;
	buffer[4] = ptm_dcb[devnum].disparo;

	address = PTM00DCB_E2P_ADDR + devnum*5;
	EepromWriteBuffer(address, buffer, 5, &error);

	return 1;
}

int helpcmd(ConsoleState* state)
{
    int i;
    OS_ERR os_err;
    
    for( i = 0; help_commands[i][0] != '~'; i++)    {
        state->conio->puts( help_commands[i] );
        state->conio->puts( "\n\r" );
        WDT_Feed();
        //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }
    return 1;
    
}

int dump_ptmdev(ConsoleState* state)
{
	int i;
	char buffer[128], tmpbuffer[6];

	state->conio->puts( "\n\rTabla de Data Control Block de los PTm\n\r" );
	state->conio->puts( "--------------------------------------\n\r\n" );


	for( i = 0; i < MAXQTYPTM; i++)	{
		buffer[0] = 0;
		Str_Cat(buffer,"DEV: "); Str_Cat(buffer, itoa(i)); Str_Cat(buffer, ",\t");

		if(i < 10)
			Str_Cat(buffer,"\t");

		sprintByte(tmpbuffer, ptm_dcb[i].rtuaddr);
        
		Str_Cat(buffer,"ADDR: "); Str_Cat(buffer,itoa(ptm_dcb[i].rtuaddr)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, "),");
        if(ptm_dcb[i].rtuaddr < 100)
            Str_Cat(buffer, "\t");

		sprintHexWord(tmpbuffer, ptm_dcb[i].cuenta);
		Str_Cat(buffer," CUENTA: "); Str_Cat(buffer,itoa(ptm_dcb[i].cuenta)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, "),\t");

		sprintByte(tmpbuffer, ptm_dcb[i].particion);
		Str_Cat(buffer,"PARTICION: "); Str_Cat(buffer,itoa(ptm_dcb[i].particion)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, ")\t");

		Str_Cat(buffer,"DISP: ");
		switch(ptm_dcb[i].disparo)	{
		case 'A':
			Str_Cat(buffer,"ASALTO\n\r");
			break;
		case 'I':
			Str_Cat(buffer,"INCENDIO\n\r");
			break;
		case 'T':
			Str_Cat(buffer,"TESORO\n\r");
			break;
		case '-':
			Str_Cat(buffer,"NINGUNO\n\r");
			break;
		case 'S':
			Str_Cat(buffer,"TAS\n\r");
			break;
		default:
			Str_Cat(buffer,"\n\r");
			break;
		}

		state->conio->puts(buffer);
	}
	state->conio->puts( "\n\r");
	return 1;
}

int dump_vars(ConsoleState* state)
{
	int i;
	char buffer[128], tmpbuffer[6];
	

	state->conio->puts( "\n");
	Str_Copy(buffer,"ABODATA:\t");
	sprintByte(tmpbuffer, BaseAlarmPkt_numabo); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer," ");
	sprintByte(tmpbuffer, BaseAlarmPkt_alarm); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer," ");
	sprintByte(tmpbuffer, BaseAlarmPkt_estado_dispositivos); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer," ");
	sprintByte(tmpbuffer, BaseAlarmPkt_memoria_dispositivos); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");
	state->conio->puts(buffer);
	state->conio->puts( "\n");

	Str_Copy(buffer,"Alarmas:\t");
	if(BaseAlarmPkt_alarm & 0x01)
		Str_Cat(buffer,"ASALTO ");
	if(BaseAlarmPkt_alarm & 0x02)
		Str_Cat(buffer,"INCENDIO ");
	if(BaseAlarmPkt_alarm & 0x04)
		Str_Cat(buffer,"TESORO ");
	if(BaseAlarmPkt_alarm & 0x08)
		Str_Cat(buffer,"ROTURA ");
	if(BaseAlarmPkt_alarm & 0x10)
		Str_Cat(buffer,"APERTURA ");
	if(BaseAlarmPkt_alarm & 0x20)
		Str_Cat(buffer,"LLOPER ");
	if(BaseAlarmPkt_alarm & 0x40)
		Str_Cat(buffer,"F220V ");
	if(BaseAlarmPkt_alarm & 0x80)
		Str_Cat(buffer,"NORMAL ");

	state->conio->puts(buffer);
	state->conio->puts( "\n\n");

	Str_Copy(buffer,"SystemFlag:\t"); sprintByte(tmpbuffer, SystemFlag); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SystemFlag1:\t"); sprintByte(tmpbuffer, SystemFlag1); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SystemFlag2:\t"); sprintByte(tmpbuffer, SystemFlag2); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SystemFlag3:\t"); sprintByte(tmpbuffer, SystemFlag3); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SystemFlag4:\t"); sprintByte(tmpbuffer, SystemFlag4); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SystemFlag5:\t"); sprintByte(tmpbuffer, SystemFlag5); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"EVOWD_Flag:\t"); sprintByte(tmpbuffer, EVOWD_Flag); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SysFlag0:\t"); sprintByte(tmpbuffer, SysFlag0); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SysFlag1:\t"); sprintByte(tmpbuffer, SysFlag1); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SysFlag2:\t"); sprintByte(tmpbuffer, SysFlag2); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SysFlag3:\t"); sprintByte(tmpbuffer, SysFlag3); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SysFlag4:\t"); sprintByte(tmpbuffer, SysFlag4); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	Str_Copy(buffer,"SysInputs:\t"); sprintByte(tmpbuffer, SysInputs); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	state->conio->puts( "\n");

	for( i = 0; i < 16; i++ )	{
		Str_Copy(buffer,"Vmed Z["); Str_Cat(buffer,itoa(i)); Str_Cat(buffer,"]:\t"); Str_Cat(buffer, itoa(zvaluemed1[i])); Str_Cat(buffer,"\t");
		Str_Cat(buffer,"Stat Z["); Str_Cat(buffer,itoa(i)); Str_Cat(buffer,"]:\t");
		switch(Status_Zonas[i])	{
		case ALRM_EVENTO :
			Str_Cat(buffer,"EVENTO\n");
			break;
		case ALRM_NORMAL :
			Str_Cat(buffer,"NORMAL\n");
			break;
		case ALRM_INDEF :
			Str_Cat(buffer,"INDEF\n");
			break;
		case ALRM_ROTU :
			Str_Cat(buffer,"ROTU\n");
			break;
		default :
			Str_Cat(buffer,"*ERROR*\n");
			break;
		}
		state->conio->puts(buffer);
	}
	state->conio->puts( "\n");


	return 1;
}


int del_ptmdev(ConsoleState* state)
{
	int error, devnum;
	uint8_t buffer[5];
	uint16_t address;

	devnum = atoi( con_getparam(state->command, 2) );

	ptm_dcb[devnum].rtuaddr = 0x00;
	ptm_dcb[devnum].cuenta = 0x0000;
	ptm_dcb[devnum].particion = 0x00;
	ptm_dcb[devnum].disparo = 0x00;

	buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x00;
	address = PTM00DCB_E2P_ADDR + devnum*5;
	EepromWriteBuffer(address, buffer, 5, &error);

	return 1;
}

int format_lan_cfgfile( ConsoleState* state )
{
	int error,i;
	uint8_t buffer[5];
	uint16_t address;
	OS_ERR os_err;

	buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x00;

	for( i = 0; i < MAXQTYPTM; i++ )	{
		address = PTM00DCB_E2P_ADDR + i*5;
		EepromWriteBuffer(address, buffer, 5, &error);
		OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
	}

	for( i = 0; i < MAXQTYPTM; i++ )	{
		address = PTM00DCB_E2P_ADDR + i*5;
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
		EepromReadBuffer(address, buffer, 5, &error);
		ptm_dcb[i].rtuaddr = buffer[0];
		ptm_dcb[i].particion = buffer[1];
		ptm_dcb[i].cuenta = buffer[2]*0x100 + buffer[3];
		ptm_dcb[i].disparo = buffer[4];
	}
	return 1;
}

void FactoryPgm(void)
{
    int error, i;
    uint32_t len;
    uint8_t buffer[40];
	uint16_t address;
	OS_ERR os_err;

	for(i = 0; i < 10; i++)	{
		buffer[i] = 0x00;
	}
	error = flash0_write(1, buffer, DF_GROUPVOLU_OFFSET, 10);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();


	//error = flash0_write(1, buffer, DF_LICTSVENC_OFFSET, 4);


	buffer[0] = 30;
	len = flash0_write(1, buffer, LIC_INTERVAL, 1);
	OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);

	buffer[0] = 0xA5;
	len = flash0_write(1, buffer, DF_BLOCKCODE_OFFSET, 1);
    WDT_Feed();

	EepromWriteByte(OPENPTM_E2P_ADDR, 0xA5, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteByte(DHCPJUMPER_E2P_ADDR, 0x01, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteByte(DHCPUSED_E2P_ADDR, 0x00, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteWord(TACHOF220_E2P_ADDR, 1, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteWord(RHBTIMER_E2P_ADDR, 15, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteWord(RHBCTIMER_E2P_ADDR, 15, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteByte(ETHPHYMODE_E2P_ADDR, ETH_FULLD_100M, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteByte(RADARMODE_E2P_ADDR, (uint8_t)0xA5, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

    EepromWriteByte(RF_NUMABO_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)0x03, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(FMODEON_ADDR, (uint8_t)0x00, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(RF_NUMABO_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(ZONE_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    
    
    EepromWriteByte(MON_NUMABO_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(RFNUMCEN_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    
    EepromWriteWord(CIDACCOUNT1_E2P_ADDR, 0x0000, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteWord(CIDACCOUNT2_E2P_ADDR, 0x0000, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteWord(R3KACCOUNT_E2P_ADDR, 0x0000, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    
    EepromWriteWord(PAPTSLOT_E2P_ADDR, 0x0000, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    EepromWriteWord(PAPARST_TIMER_E2P_ADDR, 15, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    EepromWriteWord(WDEVOTIMER_E2P_ADDR, 120, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();
	EepromWriteWord(WDEVOEVENT_E2P_ADDR, 0x602, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

    EepromWriteBuffer(NOMSUC_E2P_ADDR, "Config Fabrica", 15, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    
    EepromWriteBuffer(SERIALNUM_E2P_ADDR, "00000000", 8, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    EepromWriteByte(PREVETIMER_E2P_ADDR, (uint8_t)12, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    EepromWriteByte(SYSFLAG1_E2P_ADDR, (uint8_t)0x00, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

    EepromWriteByte(PTXORMASK_E2P_ADDR, (uint8_t)0x00, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    EepromWriteByte(TASMODE_E2P_ADDR, (uint8_t)0xA5, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteByte(WDEVOMODE_E2P_ADDR, (uint8_t)0xA5, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	EepromWriteByte(NPMEDMODE_E2P_ADDR, (uint8_t)0xA5, &error);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

    EepromWriteByte(R3KSENDMODE_E2P_ADDR, (uint8_t)2, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    EepromWriteByte(REDBOXHAB_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    //EepromWriteBuffer(LOCAL_IP_E2P_ADDR, "100.100.100.100", 16, &error);
    //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    pgm_localip("100.100.100.100");
    WDT_Feed();

    //EepromWriteBuffer(NETMASK_E2P_ADDR, "255.255.255.0", 14, &error);
    //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    pgm_netmask("255.255.255.0");
    WDT_Feed();

    //EepromWriteBuffer(GATEWAY_E2P_ADDR, "100.100.100.1", 14, &error);
    //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    pgm_gateway("100.100.100.1");
    WDT_Feed();


    
    //EepromWriteBuffer(SERVER1_E2P_ADDR, "100.100.100.102", 16, &error);
    //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    pgm_server1("100.100.100.102");
    WDT_Feed();
    
    //EepromWriteBuffer(SERVER2_E2P_ADDR, "100.100.100.104", 16, &error);
    //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    pgm_server2("100.100.100.104");
    WDT_Feed();
    
    EepromWriteWord(PORT1_E2P_ADDR, 8023, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteWord(PORT2_E2P_ADDR, 8023, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    //EepromWriteBuffer(MACADDR_E2P_ADDR, "123456", 6, &error);
    //OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    pgm_macaddress("123456");
    WDT_Feed();

    buffer[0] = 0x5A;
	len = flash0_write(1, buffer, DF_FIDUMK_OFFSET, 1);
	buffer[0] = 0xFF;
	len = flash0_write(1, buffer, DF_FIDUDT_OFFSET, 1);
	WDT_Feed();


    EepromWriteWord(HBTTIME1_E2P_ADDR, (uint16_t)120, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteWord(HBTTIME2_E2P_ADDR, (uint16_t)120, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(PROTCOL1_E2P_ADDR, (uint8_t)0x03, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(PROTCOL2_E2P_ADDR, (uint8_t)0x03, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(INUSE1_E2P_ADDR, (uint8_t)0x01, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(INUSE2_E2P_ADDR, (uint8_t)0x00, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    
    buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x00;

	for( i = 0; i < MAXQTYPTM; i++ )	{
		address = PTM00DCB_E2P_ADDR + i*5;
		EepromWriteBuffer(address, buffer, 5, &error);
		OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
	}

	for( i = 0; i < MAXQTYPTM; i++ )	{
		address = PTM00DCB_E2P_ADDR + i*5;
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
        WDT_Feed();
		EepromReadBuffer(address, buffer, 5, &error);
		ptm_dcb[i].rtuaddr = buffer[0];
		ptm_dcb[i].particion = buffer[1];
		ptm_dcb[i].cuenta = buffer[2]*0x100 + buffer[3];
		ptm_dcb[i].disparo = buffer[4];
	}
    
    //---------------------------------------------------------------
    //marca fiduciaria de inicializacion
    EepromWriteByte(FIDUMARK_E2P_ADDR, (uint8_t)0x5A, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();
    EepromWriteByte(FIDUMARK_E2P_ADDR + 1, (uint8_t)0xA5, &error);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
    WDT_Feed();

    for(i = 0; i < 10; i++ )	{
    	buffer[i] = 0x00;
    	relestate[i] = 0x00;
    }
    error = flash0_write(1, buffer, DF_RELESTATES, 10);
    WDT_Feed();
    calibrar();

    buffer[0] = 0x71;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    error = flash0_write(1, buffer, DF_VOLREDUNT_OFFSET, 5);
    WDT_Feed();

	buffer[0] = 0xA5;
	buffer[1] = 0x5A;
	error = flash0_write(1, buffer, DF_CASTROTU_OFFSET, 2);
	WDT_Feed();

	buffer[0] = 0x5A;
	buffer[1] = AUTORST_TIMER_DEF;
	error = flash0_write(1, buffer, DF_AUTORST_OFFSET, 2);
	WDT_Feed();

    buffer[0] = 0x5A;
    buffer[1] = 30;
    error = flash0_write(1, buffer, DF_BORDLY_OFFSET, 2);
    DlyBor_time = 30;
    WDT_Feed();

    buffer[0] = 0x5A;
    buffer[1] = 60;
    error = flash0_write(1, buffer, DF_PTMDLY_OFFSET, 2);
    rfdly_time = 60;

	buffer[0] = 3;
	error = flash0_write(1, buffer, DF_NMAX_OFFSET, 1);
	WDT_Feed();

	buffer[0] = 20;
	error = flash0_write(1, buffer, DF_NMAXASAL_OFFSET, 1);
	WDT_Feed();

	buffer[0] = 60;
	error = flash0_write(1, buffer, DF_NMAXTESO_OFFSET, 1);
	WDT_Feed();

	buffer[0] = 20;
	error = flash0_write(1, buffer, DF_NMAXINCE_OFFSET, 1);
	WDT_Feed();

	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	error = flash0_write(1, buffer, DF_RFFILTER1_OFFSET, 2);
	SysFlag4 |= RFFILTER1;
	WDT_Feed();

	buffer[0] = 0xA5;
	buffer[1] = 0x5A;
	error = flash0_write(1, buffer, DF_RFFILTER2_OFFSET, 2);
	SysFlag4 &= ~RFFILTER2;
	WDT_Feed();

	buffer[0] = (0 >> 8) & 0x00FF;
	buffer[1] = 5 & 0x00FF;
	buffer[2] = 0x5A;
	buffer[3] = 0xA5;
	error = flash0_write(1, (uint8_t *)buffer, DF_DELTAT_OFFSET, 4);
	DeltaT = 0;
	WDT_Feed();

	buffer[0] = (5 >> 8) & 0x00FF;
	buffer[1] = 0 & 0x00FF;
	buffer[2] = 0x5A;
	buffer[3] = 0xA5;
	error = flash0_write(1, (uint8_t *)buffer, DF_DELAYDUAL_OFFSET, 4);
	dualA_delay = 5;
	WDT_Feed();

	buffer[0] = 0xAA;
	buffer[1] = 0xBB;
	error = flash0_write(1, buffer, DF_EVSEND_OFFSET, 2);
	WDT_Feed();
	SystemFlag10 |= UDPLICOK_FLAG;
	SystemFlag10 &= ~UDPUSELIC_FLAG;

    buffer[0] = 0x5A;
    buffer[1] = 0xA5;
    error = flash0_write(1, buffer, DF_NRSTHAB_OFFSET, 2);
    WDT_Feed();
    DebugFlag |= NETRSTHAB_flag;

    buffer[0] = 0x5A;
    buffer[1] = 0xA5;
    error = flash0_write(1, buffer, DF_HRSTHAB_OFFSET, 2);
    WDT_Feed();
    DebugFlag |= HIGRSTHAB_flag;

    buffer[0] = 0xAA;
    buffer[1] = 0xBB;
    error = flash0_write(1, buffer, DF_INPATTERN_OFFSET, 2);
    WDT_Feed();
    SystemFlag7 &= ~INPATT_CHECK;

}

int factorycmd( ConsoleState* state )
{
    auto int temp;
	temp = state->numparams - state->commandparams;
	if ((state->commandparams == 1) &&
	    (Str_Cmp(con_getparam(state->command, temp), "yes") == 0)) {
		state->conio->puts("Wait, please ...\n\r");
		FactoryPgm();
		state->conio->puts("Done!\n\r");
		return 1;
	} else if ((state->commandparams == 1) &&
	           (Str_Cmp(con_getparam(state->command, temp), "no") == 0)) {
		//state->echo = 0;
		return 1;
	} else {
		state->conio->puts("Usage: factory yes\n\r");
		state->error = CON_ERR_BADPARAMETER;
		return -1;
	}
}

int con_rs232(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;
	
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set r3kmode 1|2|3|4 (1:EYSE, 2:CID, 3:AMBOS, 4:CID+timestamp)\n\r");
		retval = EepromReadByte(R3KSENDMODE_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD R3KMODE = ");
		switch(retval)	{
		case 1:
			Str_Cat(buffer, "EYSE\n\r");
			break;
		case 2:
			Str_Cat(buffer, "CID\n\r");
			break;
		case 3:
			Str_Cat(buffer, "AMBOS\n\r");
			break;
		case 4:
			Str_Cat(buffer, "CID con Timestamp\n\r");
			break;
		default:
			Str_Cat(buffer, "R3KMODE TYPE ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo > 0) && (cnumabo < 5))
		EepromWriteByte(R3KSENDMODE_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** R3KMODE TYPE ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(R3KSENDMODE_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD R3KMODE = ");
	switch(retval)	{
	case 1:
		Str_Cat(buffer, "EYSE\n\r");
		break;
	case 2:
		Str_Cat(buffer, "CID\n\r");
		break;
	case 3:
		Str_Cat(buffer, "AMBOS\n\r");
		break;
	case 4:
		Str_Cat(buffer, "CID con Timestamp\n\r");
		break;
	default:
		Str_Cat(buffer, "R3KMODE TYPE ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);
	r3kmode = EepromReadByte(R3KSENDMODE_E2P_ADDR, &error);
	return 1;
}


int con_redbox(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;
	
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 2 )	{
		state->conio->puts("redbox 0|1 (0: Inhabilitar, 1: Habilitar)\n\r");
		retval = EepromReadByte(REDBOXHAB_E2P_ADDR, &error);
		Str_Cat(buffer, "RED BOX = ");
		switch(retval)	{
		case 0:
			Str_Cat(buffer, "Inhabilitada\n\r");
			break;
		case 1:
			Str_Cat(buffer, "Habilitada\n\r");
			break;
		default:
			Str_Cat(buffer, "Habilitada\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 1));
	if((cnumabo >= 0) && (cnumabo < 2))
		EepromWriteByte(REDBOXHAB_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** REDBOX MODE TYPE ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(REDBOXHAB_E2P_ADDR, &error);
	Str_Cat(buffer, "RED BOX = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "Inhabilitada\n\r");
		break;
	case 1:
		Str_Cat(buffer, "Habilitada\n\r");
		break;
	default:
		Str_Cat(buffer, "Habilitada\n\r");
		break;
	}
	state->conio->puts(buffer);
	return 1;
}


int con_serialnum(ConsoleState* state)
{
	int len, error, i;
	char buffer[16], *serialnum;


	if( state->numparams < 3 )	{
		state->conio->puts("set serialnum \"numero de serie\"\n\r");
		EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, 8, &error);
		state->conio->puts("PGMD SERIALNUM: ");
		for(i = 0; i < 8; i++ )	{
			if(buffer[i] == 0x00 )
				break;
			if(buffer[i] > 127)	{
				state->conio->puts("\n\r");
				return 1;
			}
		}
		buffer[8] = 0;
		state->conio->puts(buffer);
		state->conio->puts("\n\r");
		return 1;
	}

	len = strlen( con_getparam(state->command, 2) );
	serialnum = con_getparam(state->command, 2);

	for(i = 0; i < 8; i++ )	{
		if((serialnum[i] < '0') || (serialnum[i] > '9'))	{
			state->conio->puts("ERROR *** El numero de serie debe ser de 8 caracteres numericos\n\r");
			return 1;
		}
	}
	if( len != 8 )	{
		state->conio->puts("ERROR *** El numero de serie debe ser de hasta 8 caracteres numericos\n\r");
		return 1;
	}

	error = 0;
	for(i = 0; i < 8; i++)  {               //detecto que no sea igual a cero
        if( serialnum[i] != '0' )
        	error = 1;
    }

    if(error == 0)	{
    	state->conio->puts("ERROR *** El numero de serie no puede ser 00000000\n\r");
    	return 1;
    }

	logCidEvent(account, 1, 910, 0, 0 );

	buffer[0] = 0x5A;
	error = flash0_write(1, buffer, DF_BLOCKCODE_OFFSET, 1);
    WDT_Feed();

	EepromWriteBuffer(SERIALNUM_E2P_ADDR, con_getparam(state->command, 2), len, &error);
	EepromReadBuffer(SERIALNUM_E2P_ADDR, buffer, len, &error);
	flash0_write(1, buffer, DF_SERIALNUM_OFFSET, len );
	buffer[len] = 0;

	//state->conio->puts(buffer);

	state->conio->puts("PGMD SERIALNUM: ");
	for(i = 0; i < len; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	//logCidEvent(account, 1, 682, 0, 0);
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	return 1;
}

int con_paptslot(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set paptslot tiempo_en_segundos\n\r");
		retval = EepromReadWord(PAPTSLOT_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PAPTSLOT = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta >= 0) && (cuenta < 0xFFFF))
		EepromWriteWord(PAPTSLOT_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** TSLOT OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(PAPTSLOT_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PAPTSLOT = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
	paptslot = retval;

	return 1;
}

int con_paparst_timer(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set pap_timer tiempo_en_minutos\n\r");
		retval = EepromReadWord(PAPARST_TIMER_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PAPARST_TIMER = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta >= 0) && (cuenta < 0xFFFF))
		EepromWriteWord(PAPARST_TIMER_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** PAP ARST TIMER OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(PAPARST_TIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PAPARST_TIMER = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
	paparst_timer = retval;

	return 1;
}

//int con_set_ptxormask(ConsoleState* state)
//{
//	uint16_t numabo;
//	uint8_t retval;
//	int error;
//	OS_ERR os_err;
//
//	if( state->numparams < 3 )	{
//		state->conio->puts("set ptxormask xx\n\r");
//		retval = EepromReadByte(PTXORMASK_E2P_ADDR, &error);
//			state->conio->puts("PTXORMASK = ");
//			conio_printHexWord(state, retval);
//			state->conio->puts("\n\r");
//		return 1;
//	}
//	numabo = atoi(con_getparam(state->command, 2));
//
//	EepromWriteByte(PTXORMASK_E2P_ADDR, (uint8_t)numabo, &error);
//
//	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//
//	retval = EepromReadByte(PTXORMASK_E2P_ADDR, &error);
//	if(numabo != retval)	{
//		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//		EepromWriteByte(PTXORMASK_E2P_ADDR, (uint8_t)numabo, &error);
//		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//		retval = EepromReadByte(PTXORMASK_E2P_ADDR, &error);
//	}
//	state->conio->puts("PTXORMASK = ");
//	conio_printHexWord(state, retval);
//	state->conio->puts("\n\r");
//
//	return 1;
//}

int ptxorstate(ConsoleState* state)
{
	unsigned char particion;
	char *mychar;
	uint8_t xormask, mask;
	OS_ERR os_err;
	int error;


	mask = 0x00;
	if( state->numparams != 3 )	{
		state->conio->puts("\tERROR de sintaxis del comando\n\r");
		return 1;
	}

	particion = atoi( con_getparam(state->command, 1) );
	if( (particion > 9) || (particion < 5) )	{
		state->conio->puts("*** Error en el numero de particion\n\r");
		return 1;
	}
	mask = (1 << (particion - 2));
	xormask = (uint8_t)EepromReadByte(PTXORMASK_E2P_ADDR, &error);

	mychar = con_getparam(state->command, 2);
	switch(*mychar)	{
	case 'N':
	case 'n':
		xormask &= ~mask;
		break;
	case 'I':
	case 'i':
		xormask |= mask;
		break;
	default:
		state->conio->puts("*** Error en el estado\n\r");
		return 1;
		break;
	}

	EepromWriteByte(PTXORMASK_E2P_ADDR, xormask, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	state->conio->puts("PTXORMASK = ");
	conio_printHexWord(state, xormask);
	state->conio->puts("\n\r");

	ptxormask = xormask;

	return 1;
}


int con_conf_ticket(ConsoleState* state)
{
	char buffer[128], buffer1[128];
	struct tm mytime;
	time_t timestamp;
    uint8_t dec_group[10];
    int particion;

	uint16_t retval, value;

	char *tmpbuf;

	int error, i, j;
	uint8_t strbuffer[16], tmpbuffer[16];

	
	OS_ERR os_err;

	state->conio->puts(VERSION_ABO);

	timestamp = SEC_TIMER;
	gmtime((const time_t *) &(timestamp), &mytime);

	state->conio->puts(asctime(&mytime));
	state->conio->puts("\n\r");

	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD NUMABO = ");
	conio_printHexWord(state, retval);
	state->conio->puts("\n\r");

	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(RF_ANSWER_TYPE, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD RFANSW = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\r");

	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
//	retval = EepromReadByte(FMODELAY_ADDR, &error);
//	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//	state->conio->puts("PGMD DLY = ");
//	conio_printHexWord( state, retval);
//	state->conio->puts("\n\r");

	retval = EepromReadByte(ZONE_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD ZONE = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\n\r");

	retval = EepromReadByte(FMODEON_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD FMODE = ");
	conio_printHexWord( state, retval);
	state->conio->puts("\n\n\r");

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	for(i = 0; i < 16; i++ )
		buffer[i] = 0;
	retval = EepromReadByte(PREVETIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PREVETIMER = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//local_ip
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	EepromReadBuffer(LOCAL_IP_E2P_ADDR, buffer, 16, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD LOCALIP: ");
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//netmask
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	EepromReadBuffer(NETMASK_E2P_ADDR, buffer, 16, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD NETMASK: ");
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//gateway
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	EepromReadBuffer(GATEWAY_E2P_ADDR, buffer, 16, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD GATEWAY: ");
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//macaddress
	for(i = 0; i < 16; i++ )	{
		buffer[i] = 0;
		strbuffer[i] = 0;
	}
	EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD MACADDRESS: ");
	for(i = 0; i < 6; i++ )	{
		sprintByte(tmpbuffer, buffer[i]);
		Str_Cat(strbuffer, tmpbuffer);
	}
	strbuffer[12] = 0;
	state->conio->puts(strbuffer);
	state->conio->puts("\n\n\r");

	//server1
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	EepromReadBuffer(SERVER1_E2P_ADDR, buffer, 16, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD SERVER1: ");
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//cuenta1
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(CIDACCOUNT1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD CUENTA1 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//port1
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(PORT1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD PORT1 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//hb1
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(HBTTIME1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD HB1 = ");
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//protocol1
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(PROTCOL1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD PROTOCOL1 = ");
	switch(retval)	{
	case 1:
		Str_Cat(buffer, "NETIO_4\n\r");
		break;
	case 2:
		Str_Cat(buffer, "NETIO_5\n\r");
		break;
	case 3:
		Str_Cat(buffer, "NETIO_6\n\r");
		break;
	case 4:
		Str_Cat(buffer, "NETIO_7\n\r");
		break;
	default:
		Str_Cat(buffer, "PROTOCOL TYPE ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	//inuse1
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(INUSE1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD INUSE1 = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "OFF\n\n\r");
		break;
	case 1:
		Str_Cat(buffer, "ON\n\n\r");
		break;
	default:
		Str_Cat(buffer, "INUSE ACTIVATION ERROR\n\n\r");
		break;
	}
	state->conio->puts(buffer);

	//-------------------------------------------------------
	//server2
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	EepromReadBuffer(SERVER2_E2P_ADDR, buffer, 16, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	state->conio->puts("PGMD SERVER2: ");
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	//cuenta2
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(CIDACCOUNT2_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD CUENTA2 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//port2
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(PORT2_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD PORT2 = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//hb2
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(HBTTIME2_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD HB2 = ");
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	//protocol2
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(PROTCOL2_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD PROTOCOL2 = ");
	switch(retval)	{
	case 1:
		Str_Cat(buffer, "NETIO_4\n\r");
		break;
	case 2:
		Str_Cat(buffer, "NETIO_5\n\r");
		break;
	case 3:
		Str_Cat(buffer, "NETIO_6\n\r");
		break;
	case 4:
		Str_Cat(buffer, "NETIO_7\n\r");
		break;
	default:
		Str_Cat(buffer, "PROTOCOL TYPE ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);
    WDT_Feed();
	//inuse2
	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(INUSE2_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD INUSE2 = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "OFF\n\n\r");
		break;
	case 1:
		Str_Cat(buffer, "ON\n\n\r");
		break;
	default:
		Str_Cat(buffer, "INUSE ACTIVATION ERROR\n\n\r");
		break;
	}
	state->conio->puts(buffer);

	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadWord(R3KACCOUNT_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD CUENTA R3K = ");
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(R3KSENDMODE_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "PGMD R3KMODE = ");
	switch(retval)	{
	case 1:
		Str_Cat(buffer, "EYSE\n\n\r");
		break;
	case 2:
		Str_Cat(buffer, "CID\n\n\r");
		break;
	case 3:
		Str_Cat(buffer, "AMBOS\n\n\r");
		break;
	default:
		Str_Cat(buffer, "R3KMODE TYPE ERROR\n\n\r");
		break;
	}
	state->conio->puts(buffer);

	for(i = 0; i < 16; i++ )
			buffer[i] = 0;
	retval = EepromReadByte(REDBOXHAB_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	Str_Cat(buffer, "RED BOX = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "Inhabilitada\n\r");
		break;
	case 1:
		Str_Cat(buffer, "Habilitada\n\r");
		break;
	default:
		Str_Cat(buffer, "Habilitada\n\r");
		break;
	}
	state->conio->puts(buffer);

	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &os_err);
	for(i = 0; i < 16; i++ )
		buffer[i] = 0;

	flash0_read(tmpbuffer, DF_SERIALNUM_OFFSET, 8);
	buffer[0] = 0; tmpbuffer[8] = 0;
 	Str_Cat(buffer, "FLASH SERIALNUM: ");
 	Str_Cat(buffer, tmpbuffer);
 	state->conio->puts(buffer);
	state->conio->puts("\n\r");

    WDT_Feed();

	EepromReadBuffer(SERIALNUM_E2P_ADDR, tmpbuffer, 8, &error);
	buffer[0] = 0; tmpbuffer[8] = 0;
 	Str_Cat(buffer, "E2PROM SERIALNUM: ");
 	Str_Cat(buffer, tmpbuffer);
 	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	for(i = 0; i < 16; i++ )
		buffer[i] = 0;
	retval = EepromReadWord(PAPTSLOT_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PAPTSLOT = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	for(i = 0; i < 16; i++ )
		buffer[i] = 0;
	retval = EepromReadWord(PAPARST_TIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD PAPARST_TIMER = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

    WDT_Feed();

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	for(i = 0; i < 16; i++ )
		buffer[i] = 0;
	retval = EepromReadByte(PTXORMASK_E2P_ADDR, &error);
	state->conio->puts("PTXORMASK = ");
	conio_printHexWord(state, retval);
	state->conio->puts("\n\r");

	retval = EepromReadByte(SYSFLAG1_E2P_ADDR, &error);
	if(retval & BISEL_FLAG)	{
		state->conio->puts("BISEL MODE = ON\n\r");
	} else	{
		state->conio->puts("BISEL MODE = OFF\n\r");
	}

	retval = EepromReadByte(TASMODE_E2P_ADDR, &error);
	if(retval == 0x5A)	{
		state->conio->puts("TAS MODE = ON\n\r");
	} else	{
		state->conio->puts("TAS MODE = OFF\n\r");
	}


	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	for(i = 0; i < 16; i++ )
		buffer[i] = 0;
	retval = EepromReadWord(WDEVOTIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD WDEVO TIMER = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	retval = EepromReadByte(WDEVOMODE_E2P_ADDR, &error);
	if(retval == 0x5A)	{
		state->conio->puts("WDEVO MODE = ON\n\r");
	} else	{
		state->conio->puts("WDEVO MODE = OFF\n\r");
	}

    WDT_Feed();

	//tacho de 220
	retval = EepromReadWord(TACHOF220_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD TACHO F220V = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);


	//imprimo estado rele OPENPTM
	retval = (uint8_t)EepromReadByte(OPENPTM_E2P_ADDR , &error);
    if(retval != 0xA5)   {
    	state->conio->puts("OPENPTM = ON\n\r");
    } else {
        state->conio->puts("OPENPTM = OFF\n\r");
    }


	//Imprimo la tabla de los PTM
	state->conio->puts( "\n\rTabla de Data Control Block de los PTm\n\r" );
	state->conio->puts( "--------------------------------------\n\r\n" );


	for( i = 0; i < MAXQTYPTM; i++)	{
        WDT_Feed();
		buffer[0] = 0;
		Str_Cat(buffer,"DEV: "); Str_Cat(buffer,itoa(i)); Str_Cat(buffer, ",\t");

		if(i < 10)
			Str_Cat(buffer,"\t");

		sprintByte(tmpbuffer, ptm_dcb[i].rtuaddr);

		Str_Cat(buffer,"ADDR: "); Str_Cat(buffer,itoa(ptm_dcb[i].rtuaddr)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, "),");
        if(ptm_dcb[i].rtuaddr < 100)
            Str_Cat(buffer, "\t");

		sprintHexWord(tmpbuffer, ptm_dcb[i].cuenta);
		Str_Cat(buffer," CUENTA: "); Str_Cat(buffer,itoa(ptm_dcb[i].cuenta)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, "),\t");

		sprintByte(tmpbuffer, ptm_dcb[i].particion);
		Str_Cat(buffer,"PARTICION: "); Str_Cat(buffer,itoa(ptm_dcb[i].particion)); Str_Cat(buffer, "(0x");Str_Cat(buffer, tmpbuffer) ;Str_Cat(buffer, ")\t");

		Str_Cat(buffer,"DISP: ");
		switch(ptm_dcb[i].disparo)	{
		case 'A':
			Str_Cat(buffer,"ASALTO\n\r");
			break;
		case 'I':
			Str_Cat(buffer,"INCENDIO\n\r");
			break;
		case 'T':
			Str_Cat(buffer,"TESORO\n\r");
			break;
		case '-':
			Str_Cat(buffer,"NINGUNO\n\r");
			break;
		case 'S':
			Str_Cat(buffer,"TAS\n\r");
			break;
		default:
			Str_Cat(buffer,"\n\r");
			break;
		}

		state->conio->puts(buffer);
	}
	state->conio->puts( "\n\r");

	//Imprimo la tabla de replicacion
	for(i = 0; i < 80; i++)
		buffer[i] = 0;

	state->conio->puts( "\n\r");
	state->conio->puts( "Particion destino       Particion origen\n\r");
	state->conio->puts( "-----------------       ----------------\n\r");

	for(i = 0; i < 5; i++)	{
		for(j = 0; j < 80; j++)
			buffer[j] = 0;
		Str_Cat(buffer, "\t");Str_Cat(buffer,itoa(i+5)); Str_Cat(buffer, "\t\t\t");Str_Cat(buffer, itoa(BCDToInt(PT_estado_particion[i]))) ;Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
	}
	state->conio->puts( "\n\n\r");
    WDT_Feed();

	//-------------------------------------------------------------------------------
    state->conio->puts( "Configuracion de retardos y cantidad de disparos\n\r");
    state->conio->puts( "------------------------------------------------\n\r");

    //. . . . . . . . . . . . . . . . . . . . . . . . . .
    buffer1[0] = 0;
    flash0_read(buffer, DF_PTMDLY_OFFSET, 2);
    if(buffer[0] != 0x5A)	{
        state->conio->puts("PTMDLY no programado\n\r");
    } else {
        Str_Cat(buffer1, "PGMD PTMDLY = ");
        tmpbuf = itoa(buffer[1]);
        Str_Cat(buffer1, tmpbuf);
        Str_Cat(buffer1, "\n\r");
        state->conio->puts(buffer1);
    }
    WDT_Feed();

    //. . . . . . . . . . . . . . . . . . . . . . . . . .
    buffer1[0] = 0;
    flash0_read(buffer, DF_BORDLY_OFFSET, 2);
    if(buffer[0] != 0x5A)	{
        state->conio->puts("BORDLY no programado\n\r");
    } else {
        Str_Cat(buffer1, "PGMD BORDLY = ");
        tmpbuf = itoa(buffer[1]);
        Str_Cat(buffer1, tmpbuf);
        Str_Cat(buffer1, "\n\r");
        state->conio->puts(buffer1);
    }
    WDT_Feed();

    //. . . . . . . . . . . . . . . . . . . . . . . . . .
    buffer1[0] = 0;
    flash0_read(buffer, DF_AUTORST_OFFSET, 2);
    if(buffer[0] != 0x5A)	{
        state->conio->puts("AUTORESET no programado\n\r");
    } else {
        Str_Cat(buffer1, "PGMD AUTORESET = ");
        //sprintHexWord(tmpbuf, retval);
        tmpbuf = itoa(buffer[1]);
        Str_Cat(buffer1, tmpbuf);
        Str_Cat(buffer1, "\n\r");
        state->conio->puts(buffer1);
    }
    WDT_Feed();

    //. . . . . . . . . . . . . . . . . . . . . . . . . .
    buffer1[0] = 0;
    flash0_read(buffer, DF_NMAXASAL_OFFSET, 1);
    Str_Cat(buffer1, "PGMD NMAXASAL = ");
    tmpbuf = itoa(buffer[0]);
    Str_Cat(buffer1, tmpbuf);
    Str_Cat(buffer1, "\n\r");
    state->conio->puts(buffer1);
    WDT_Feed();

    //. . . . . . . . . . . . . . . . . . . . . . . . . .
    buffer1[0] = 0;
    flash0_read(buffer, DF_NMAXTESO_OFFSET, 1);
    Str_Cat(buffer1, "PGMD NMAXTESO = ");
    tmpbuf = itoa(buffer[0]);
    Str_Cat(buffer1, tmpbuf);
    Str_Cat(buffer1, "\n\r");
    state->conio->puts(buffer1);
    WDT_Feed();

    //. . . . . . . . . . . . . . . . . . . . . . . . . .
    buffer1[0] = 0;
    flash0_read(buffer, DF_NMAXINCE_OFFSET, 1);
    Str_Cat(buffer1, "PGMD NMAXINCE = ");
    tmpbuf = itoa(buffer[0]);
    Str_Cat(buffer1, tmpbuf);
    Str_Cat(buffer1, "\n\r");
    state->conio->puts(buffer1);
    WDT_Feed();

    //Imprimo la tabla de redundancias
    state->conio->puts( "\n\rTabla de Redundancias\n\r" );
    state->conio->puts( "-------------------------\n\r\n" );
    for(i = 0; i < 80; i++)
        buffer[i] = 0;
    WDT_Feed();
    state->conio->puts( "\n\r");
    state->conio->puts( "Particion destino       Particion origen\n\r");
    state->conio->puts( "-----------------       ----------------\n\r");

    for(i = 0; i < 5; i++)	{
        for(j = 0; j < 80; j++)
            buffer[j] = 0;
        Str_Cat(buffer, "\t");Str_Cat(buffer,itoa(i+5)); Str_Cat(buffer, "\t\t\t");Str_Cat(buffer, itoa(BCDToInt(VolumetricRedundance[i]))) ;Str_Cat(buffer, "\n\r");
        state->conio->puts(buffer);
    }
    state->conio->puts( "\n\n\r");

    buffer[0] = 0;
    retval = EepromReadByte(ETHPHYMODE_E2P_ADDR, &error);
    Str_Cat(buffer, "PGMD PHYMODE = ");
    switch(retval)	{
        case 1:
            Str_Cat(buffer, "AUTONEGOTIATE\n\r");
            break;
        case 2:
            Str_Cat(buffer, "100M FULL DUPLEX\n\r");
            break;
        case 3:
            Str_Cat(buffer, "100M HALF DUPLEX\n\r");
            break;
        case 4:
            Str_Cat(buffer, "10M FULL DUPLEX\n\r");
            break;
        case 5:
            Str_Cat(buffer, "10M HALF DUPLEX\n\r");
            break;
        case 6:
            Str_Cat(buffer, "TRY ALL\n\r");
            break;
        default:
            Str_Cat(buffer, "PHYMODE TYPE ERROR\n\r");
            break;
    }
    state->conio->puts(buffer);
    WDT_Feed();

    error = flash0_read(dec_group, DF_GROUPVOLU_OFFSET, 10);
    for(i = 0; i < 80; i++)
        buffer[i] = 0;
    state->conio->puts("\n\r");
    Str_Cat(buffer, "Volumetricas: ");

    for(i = 1; i < 10; i++)	{
        if(dec_group[i] == 0x00)
            continue;
        for(j = 0; j < 8; j++)	{
            if( dec_group[i] & (1 << j))	{
                particion = i*10 + j + 1;
                Str_Cat(buffer,itoa(particion));
                Str_Cat(buffer," ");
            }
        }
        Str_Cat(buffer, "\n\r");
        state->conio->puts(buffer);

        buffer[0] = 0;
        Str_Cat(buffer, "Volumetricas: ");
    }
    state->conio->puts("\n\r");
    WDT_Feed();

    //------------------------------------------------------------------
    buffer[0] = 0;
    flash0_read(buffer1, DF_INPATTERN_OFFSET, 2);
    if ((buffer1[0] == 0x5A) && (buffer1[1] == 0xA5))
        retval = 1;
    else if((buffer1[0] == 0xAA) && (buffer1[1] == 0xBB))
        retval = 0;
    else
        retval = 2;

    Str_Cat(buffer, "INPATTCHECK = ");
    switch (retval) {
        case 0:
            Str_Cat(buffer, "OFF\n\r");
            break;
        case 1:
            Str_Cat(buffer, "ON\n\r");
            break;
        default:
            Str_Cat(buffer, "ACTIVATION ERROR\n\r");
            break;
    }
    state->conio->puts(buffer);
    WDT_Feed();

    //------------------------------------------------------------------

    buffer[0] = 0;
    flash0_read(buffer1, DF_RFFILTER1_OFFSET, 2);
    if((buffer1[0] == 0x5A) && (buffer1[1] == 0xA5))
        retval = 1;
    else
        retval = 0;

    Str_Cat(buffer, "RFFILTER1 = ");
    switch(retval)	{
        case 0:
            Str_Cat(buffer, "OFF\n\r");
            break;
            case 1:
                Str_Cat(buffer, "ON\n\r");
                break;
                default:
                    Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                    break;
    }
    state->conio->puts(buffer);
    WDT_Feed();

    //------------------------------------------------------------------
    buffer[0] = 0;
    flash0_read(buffer1, DF_RFFILTER2_OFFSET, 2);
    if((buffer1[0] == 0x5A) && (buffer1[1] == 0xA5))
        retval = 1;
    else
        retval = 0;

    Str_Cat(buffer, "RFFILTER2 = ");
    switch(retval)	{
        case 0:
            Str_Cat(buffer, "OFF\n\r");
            break;
            case 1:
                Str_Cat(buffer, "ON\n\r");
                break;
                default:
                    Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                    break;
    }
    state->conio->puts(buffer);
    WDT_Feed();

    //------------------------------------------------------------------
    buffer[0] = 0;
    flash0_read(buffer1, DF_DELTAT_OFFSET, 4);
    if((buffer1[2] == 0x5A) && (buffer1[3] == 0xA5)) {
        value = buffer1[0]*0x100 + buffer1[1];
        retval = 1;
    }
    else
        retval = 0;

    Str_Cat(buffer, "DeltaT = ");
    switch(retval)	{
        case 0:
            Str_Cat(buffer,itoa(value));
            Str_Cat(buffer, "  *NOT SET\n\r");
            break;
            case 1:
                Str_Cat(buffer,itoa(value));
                Str_Cat(buffer, "  *SET\n\r");
                break;
                default:
                    Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                    break;
    }
    state->conio->puts(buffer);
    WDT_Feed();

    //-------------------------------------------------------------------
    buffer[0] = 0;
    flash0_read(buffer1, DF_DELAYDUAL_OFFSET, 4);
    if((buffer1[2] == 0x5A) && (buffer1[3] == 0xA5)) {
        value = buffer1[0]*0x100 + buffer1[1];
        retval = 1;
    }
    else
        retval = 0;

    Str_Cat(buffer, "delaydual = ");
    switch(retval)	{
        case 0:
            Str_Cat(buffer,itoa(value));
            Str_Cat(buffer, "  *NOT SET\n\r");
            break;
            case 1:
                Str_Cat(buffer,itoa(value));
                Str_Cat(buffer, "  *SET\n\r");
                break;
                default:
                    Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                    break;
    }
    state->conio->puts(buffer);
    WDT_Feed();

    //-------------------------------------------------------------------------
    flash0_read(buffer, DF_NRSTHAB_OFFSET, 2);
    if((buffer[0] == 0xA5) && (buffer[1] == 0x5A))  {
        state->conio->puts("Reset Interrupcion de Red DESHABILITADO\n\r");
    } else if((buffer[0] == 0x5A) && (buffer[1] == 0xA5))   {
        state->conio->puts("Reset Interrupcion de Red HABILITADO\n\r");
    } else  {
        state->conio->puts("Reset Interrupcion de Red NO CONFIGURADO\n\r");
    }
    WDT_Feed();
    //--------------------------------------------------------------------------
    flash0_read(buffer, DF_HRSTHAB_OFFSET, 2);
    if((buffer[0] == 0xA5) && (buffer[1] == 0x5A))  {
        state->conio->puts("Reset Higienico DESHABILITADO\n\r");
    } else if((buffer[0] == 0x5A) && (buffer[1] == 0xA5))   {
        state->conio->puts("Reset Higienico HABILITADO\n\r");
    } else  {
        state->conio->puts("Reset Higienico NO CONFIGURADO\n\r");
    }
    WDT_Feed();

    if(SystemFlag11 & INCE2MODE_FLAG)   {
        state->conio->puts("Incendio 2 Activado\n\r");
    } else  {
        state->conio->puts("Incendio 2 Desactivado\n\r");
    }

	return 1;
}

//int con_set_bisel(ConsoleState* state)
//{
//	uint16_t numabo;
//	uint8_t retval;
//	int error;
//	OS_ERR os_err;
//
//	if( state->numparams < 3 )	{
//		state->conio->puts("set bisel  0|1\n\r");
//		retval = EepromReadByte(SYSFLAG1_E2P_ADDR, &error);
//		if(retval & BISEL_FLAG)	{
//			state->conio->puts("BISEL MODE = ON\n\r");
//		} else	{
//			state->conio->puts("BISEL MODE = OFF\n\r");
//		}
//		return 1;
//	}
//	numabo = atoi(con_getparam(state->command, 2));
//	retval = EepromReadByte(SYSFLAG1_E2P_ADDR, &error);
//	if(numabo == 1)	{
//		retval |= BISEL_FLAG;
//		SystemFlag1 |= BISEL_FLAG;
//	} else	{
//		retval &= ~BISEL_FLAG;
//		SystemFlag1 &= ~BISEL_FLAG;
//		if(numabo == 255)
//			retval = 0;
//	}
//
//	EepromWriteByte(SYSFLAG1_E2P_ADDR, (uint8_t)retval, &error);
//
//	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//
//	retval = EepromReadByte(SYSFLAG1_E2P_ADDR, &error);
//
//
//	if(retval & BISEL_FLAG)	{
//		state->conio->puts("BISEL MODE = ON\n\r");
//	} else	{
//		state->conio->puts("BISEL MODE = OFF\n\r");
//	}
//
//	return 1;
//}

int setpgm1(ConsoleState* state)
{


	if(!(TasFlags & TASMODE_FLAG) && ( !(SystemFlag5 & OPENPTM) ))	{
		GPIO_SetValue(0, 1<<27);
		SystemFlag |= PGM1_ON_FLAG;
		return 1;
	}
	return -1;
}

int resetpgm1(ConsoleState* state)
{



	if(!(TasFlags & TASMODE_FLAG))	{
		GPIO_ClearValue(0, 1<<27);
		SystemFlag |= PGM1_OFF_FLAG;
		logCidEvent(account, 3, 930, 0, 0);		//genero evento cid de desactivacion pgm1
		return 1;
	}
	return -1;
}

int setTAS(ConsoleState* state)
{





	TasFlags |= TASCMDON_FLAG;

	return 1;
}

int resetTAS(ConsoleState* state)
{



	TasFlags |= TASCMDOFF_FLAG;

	return 1;
}

int con_set_TAS(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set TAS  0|1\n\r");
		retval = EepromReadByte(TASMODE_E2P_ADDR, &error);
		if(retval == 0x5A)	{
			state->conio->puts("TAS MODE = ON\n\r");
		} else	{
			state->conio->puts("TAS MODE = OFF\n\r");
		}
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	retval = EepromReadByte(TASMODE_E2P_ADDR, &error);
	if(numabo == 1)	{
		retval = 0x5A;
		TasFlags |= TASMODE_FLAG;
	} else	{
		retval = 0xA5;
		TasFlags &= ~TASMODE_FLAG;
		if(numabo == 255)
			retval = 0xA5;
	}

	EepromWriteByte(TASMODE_E2P_ADDR, (uint8_t)retval, &error);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	retval = EepromReadByte(TASMODE_E2P_ADDR, &error);


	if(retval == 0x5A)	{
		state->conio->puts("TAS MODE = ON\n\r");
	} else	{
		state->conio->puts("TAS MODE = OFF\n\r");
	}

	return 1;
}



int con_set_OPENPTM(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set OPENPTM  0|1\n\r");
		retval = EepromReadByte(OPENPTM_E2P_ADDR, &error);
		if(retval == 0x5A)	{
			state->conio->puts("OPENPTM MODE = ON\n\r");
		} else	{
			state->conio->puts("OPENPTM MODE = OFF\n\r");
		}
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	retval = EepromReadByte(OPENPTM_E2P_ADDR, &error);
	if(numabo == 1)	{
		retval = 0x5A;
		SystemFlag5 |= OPENPTM;
	} else	{
		//GPIO_SetValue(0, 1<<27);
		retval = 0xA5;
		SystemFlag5 &= ~OPENPTM;
		if(numabo == 255)
			retval = 0xA5;
	}

	EepromWriteByte(OPENPTM_E2P_ADDR, (uint8_t)retval, &error);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	retval = EepromReadByte(OPENPTM_E2P_ADDR, &error);


	if(retval != 0xA5)	{
		state->conio->puts("OPENPTM MODE = ON\n\r");
		logCidEvent(account, 1, 990, 0, 1 );
	} else	{
		state->conio->puts("OPENPTM MODE = OFF\n\r");
		logCidEvent(account, 1, 990, 0, 0 );
	}


	return 1;
}


int con_pulse_rele(ConsoleState* state)
{
	uint8_t rele, rpulse, placa, i;

	placa = atoi( con_getparam(state->command, 1) );
	rele = atoi( con_getparam(state->command, 2) );
	rpulse = atoi( con_getparam(state->command, 3) );

	if((placa >= 10) || (rele >= 8) || (rpulse > 1))
		return  -1;

	if( rpulse == 1)	{
		relepulse[placa] |= bitpat[rele];
		for( i = 0; i < MAXQTYPTM; i++)	{
			if( ptm_dcb[i].rtuaddr == 230 + placa )
				ptm_dcb[i].flags |= RELESIGNAL;
		}
	} else
		relepulse[placa] &= ~bitpat[rele];
	return 1;
}

int con_state_rele(ConsoleState* state)
{
	uint8_t rele, rpulse, placa;
	uint32_t len;

	placa = atoi( con_getparam(state->command, 1) );
	rele = atoi( con_getparam(state->command, 2) );
	rpulse = atoi( con_getparam(state->command, 3) );

	if((placa >= 10) || (rele >= 8) || (rpulse > 1))
		return  -1;

	if( rpulse == 1)	{
		relestate[placa] |= bitpat[rele];
//		for( i = 0; i < MAXQTYPTM; i++)	{
//			if( ptm_dcb[i].rtuaddr == 230 + placa )
//				ptm_dcb[i].flags |= RELESIGNAL;
//		}
	} else
		relestate[placa] &= ~bitpat[rele];

	len = flash0_write(1, relestate, DF_RELESTATES, 10);
	return 1;
}

int con_st_cerradura(ConsoleState* state)
{
	uint8_t cerradura;

	cerradura = atoi( con_getparam(state->command, 1) );

	if( cerradura == 1)	{
		if( SIRENA_Flag & STRIKE1_FLAG)	{
			state->conio->puts("1\n\r");
			//logCidEvent(account, 1, 974, 0, 1);
		} else	{
			state->conio->puts("0\n\r");
			//logCidEvent(account, 3, 974, 0, 1);
		}
	} else
	if( cerradura == 2)	{
		if( SIRENA_Flag & STRIKE2_FLAG)	{
			state->conio->puts("1\n\r");
			//logCidEvent(account, 1, 974, 0, 2);
		} else	{
			state->conio->puts("0\n\r");
			//logCidEvent(account, 3, 974, 0, 2);
		}
	}

	return 1;
}

int con_st_llavemecanica(ConsoleState* state)
{
	uint8_t cerradura;

	cerradura = atoi( con_getparam(state->command, 1) );

	if( cerradura == 1)	{
		if( STRIKE_Flag & STRKLLAVE_STATE)	{
			state->conio->puts("1\n\r");
			//logCidEvent(account, 1, 979, 0, 0);
		} else	{
			state->conio->puts("0\n\r");
			//logCidEvent(account, 3, 979, 0, 0);
		}
	}
	return 1;
}

int con_ince2_status(ConsoleState* state)
{
    if(SystemFlag11 & INCE2MODE_FLAG)   {
        state->conio->puts("1\n\r");
    } else  {
        state->conio->puts("0\n\r");
    }
    return 1;
}

int con_st_puerta(ConsoleState* state)
{
	uint8_t puerta;

	puerta = atoi( con_getparam(state->command, 1) );

	if( puerta == 1)	{
		if( SIRENA_Flag & DOOR1_OC_FLAG)	{
			state->conio->puts("1\n\r");
			//logCidEvent(account, 3, 975, 0, 1);
		} else	{
			state->conio->puts("0\n\r");
			//logCidEvent(account, 1, 975, 0, 1);
		}
	} else
	if( puerta == 2)	{
		if( SIRENA_Flag & DOOR2_OC_FLAG)	{
			state->conio->puts("1\n\r");
			//logCidEvent(account, 3, 975, 0, 2);
		} else	{
			state->conio->puts("0\n\r");
			//logCidEvent(account, 1, 975, 0, 2);
		}
	}
	return 1;
}

int con_cerradura(ConsoleState* state)
{
	uint8_t rele, rpulse, placa;
	uint32_t len;

	rele = atoi( con_getparam(state->command, 1) );
	rpulse = atoi( con_getparam(state->command, 2) );

	if((rele >= 3) || (rpulse > 1))
		return  -1;

	if( rele == 1)	{
		if( (rpulse == 1))	{
			STRIKE_Flag |= STRIKE1_ON_FLAG;
			//relestate[0] |= bitpat[0];
			//relestate[0] |= bitpat[4];
		} else	{
			STRIKE_Flag |= STRIKE1_OFF_FLAG;
			//relestate[0] &= ~bitpat[0];
			//relestate[0] &= ~bitpat[4];
		}
	} else
	if( rele == 2)	{
		if( (rpulse == 1))	{
			STRIKE_Flag |= STRIKE2_ON_FLAG;
			//relestate[0] |= bitpat[1];
			//relestate[0] |= bitpat[5];
		} else	{
			STRIKE_Flag |= STRIKE2_OFF_FLAG;
			//relestate[0] &= ~bitpat[1];
			//relestate[0] &= ~bitpat[5];
		}
	}


	return 1;
}

int con_sirena(ConsoleState* state)
{
	uint8_t rele, rpulse, placa;
	uint32_t len;

	rele = atoi( con_getparam(state->command, 1) );
	rpulse = atoi( con_getparam(state->command, 2) );

	if((rele >= 3) || (rpulse > 1))
		return  -1;

	if( rele == 1)	{
		if( (rpulse == 1))	{
			SIRENA_Flag |= SIR1_ON_FLAG;
		} else	{
			SIRENA_Flag |= SIR1_OFF_FLAG;
		}
	} else
	if( rele == 2)	{
		if( (rpulse == 1))	{
			SIRENA_Flag |= SIR2_ON_FLAG;
		} else	{
			SIRENA_Flag |= SIR2_OFF_FLAG;
		}
	}


	return 1;
}

int con_set_wdogevo(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set WDEVO  0|1\n\r");
		retval = EepromReadByte(WDEVOMODE_E2P_ADDR, &error);
		if(retval == 0x5A)	{
			state->conio->puts("WDEVO MODE = ON\n\r");
		} else	{
			state->conio->puts("WDEVO MODE = OFF\n\r");
		}
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	retval = EepromReadByte(WDEVOMODE_E2P_ADDR, &error);
	if(numabo == 1)	{
		retval = 0x5A;
		SystemFlag3 |= WDOG_EVO_ENABLE;
	} else	{
		retval = 0xA5;
		SystemFlag3 &= ~WDOG_EVO_ENABLE;
		if(numabo == 255)
			retval = 0xA5;
	}

	EepromWriteByte(WDEVOMODE_E2P_ADDR, (uint8_t)retval, &error);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	retval = EepromReadByte(WDEVOMODE_E2P_ADDR, &error);


	if(retval == 0x5A)	{
		state->conio->puts("WDEVO MODE = ON\n\r");
	} else	{
		state->conio->puts("WDEVO MODE = OFF\n\r");
	}

	return 1;
}

int con_set_npmed(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set NPEVENT  0|1\n\r");
		retval = EepromReadByte(WDEVOMODE_E2P_ADDR, &error);
		if(retval == 0x5A)	{
			state->conio->puts("NPEVENT MODE = ON\n\r");
		} else	{
			state->conio->puts("NPEVENT MODE = OFF\n\r");
		}
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	//retval = EepromReadByte(NPMEDMODE_E2P_ADDR, &error);
	if(numabo == 1)	{
		retval = 0x5A;
		SystemFlag4 |= NPMED_FLAG;
	} else	{
		retval = 0xA5;
		SystemFlag4 &= ~NPMED_FLAG;
		if(numabo == 255)
			retval = 0xA5;
	}

	EepromWriteByte(NPMEDMODE_E2P_ADDR, (uint8_t)retval, &error);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	retval = EepromReadByte(NPMEDMODE_E2P_ADDR, &error);


	if(retval == 0x5A)	{
		state->conio->puts("NPEVENT MODE = ON\n\r");
	} else	{
		state->conio->puts("NPEVENT MODE = OFF\n\r");
	}

	return 1;
}




int con_wdevo_timer(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set wdevo_timer tiempo_en_minutos\n\r");
		retval = EepromReadWord(WDEVOTIMER_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD WDEVO TIMER = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta >= 90) && (cuenta <= 240))
		EepromWriteWord(WDEVOTIMER_E2P_ADDR, cuenta, &error);
	else	{
        if(cuenta == 999)
            EepromWriteWord(WDEVOTIMER_E2P_ADDR, 5, &error);
		state->conio->puts("*** WDEVO TIMER OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(WDEVOTIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD WDEVO TIMER = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
	wdtimer = retval;

	return 1;
}

int con_wdevo_event(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set wdevo_event evento\n\r");
		retval = EepromReadWord(WDEVOEVENT_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD WDEVO EVENT = ");
		sprintHexWord(tmpbuf, retval);
		//tmpbuf = itoa(retval);
		//Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoh(con_getparam(state->command, 2));
	if((cuenta >= 0) && (cuenta < 0xFFFF))
		EepromWriteWord(WDEVOEVENT_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** WDEVENT OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(WDEVOEVENT_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD WDEVO EVENT = ");
	sprintHexWord(buffer, retval);
	//tmpbuf = itoa(retval);
	//Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
	wdevo_event = retval;

	return 1;
}


//int send_manual_event(ConsoleState* state)
//{
//	uint8_t qualifier, partition;
//	uint16_t eventcode, zoneuser;
//
//	if( state->numparams != 5 )	{
//		state->conio->puts("\tUsage : manual_event qual eventcode partition zoneuser\n\r\tej: config ptm 05 27 1234 15 T\n\r");
//		state->conio->puts("\t\t qual:      1: evento, 3: normalizacion\n\r");
//		state->conio->puts("\t\t eventcode: Codigo del evento\n\r");
//		state->conio->puts("\t\t partition: Numero de la particion de 0 a 99\n\r");
//		state->conio->puts("\t\t zoneuser:  Numero de zona o usuario, de 0 a 999\n\r");
//		return 1;
//	}
//
//	qualifier = atoi( con_getparam(state->command, 1) );
//	eventcode = atoi( con_getparam(state->command, 2) );
//	partition = atoi( con_getparam(state->command, 3) );
//	zoneuser = atoi( con_getparam(state->command, 4) );
//
//	logCidEvent(account, 1, 946, 0, 0);
//
//	logCidEvent( account, qualifier, eventcode, partition, zoneuser );
//
//	return 1;
//}

uint8_t hex2nibble(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	else if (c >= 'a' && c <= 'f')
		return (c - 'a') + 10;
	else if (c >= 'A' && c <= 'F')
		return (c - 'A') + 10;
	else
		return 0;
}

//int set_license(ConsoleState* state)
//{
//	char *input_code;
//	int len, i, j, lic_account;
//	uint8_t lic_code[16], buffer[12];
//	OS_ERR os_err;
//
//	struct tm time;
//	time_t timestamp;
//
//	uint32_t m_timestamp, old_timestamp, error, licvenc_ts;
//	uint8_t m_tecnico, m_interval, m_rndnumber, m_chksum, old_rndnumber;
//	uint16_t chksum;
//
//
//
//
//	len = strlen( con_getparam(state->command, 1) );
//	input_code = con_getparam(state->command, 1);
//
//	if(len != 32)	{
//		//state->conio->puts("*ERROR : Clave de longitud incorrecta\n\r");
//		return -1;
//	}
//
//	for( i = 0; i < 32; i++ )	{
//		if(!ASCII_IsDigHex(input_code[i]))	{
//			//state->conio->puts("*ERROR : Clave con caracteres invalidos\n\r");
//			return -1;
//		}
//	}
//
//	for( i = 0, j = 0; i < 32; i+=2, j++)	{
//		lic_code[j] = (hex2nibble(input_code[i]) << 4) + hex2nibble(input_code[i+1]);
//	}
//
//	aes_decrypt( lic_code, (unsigned char *)AES_key );
//
//	m_timestamp  = lic_code[3];
//	m_timestamp += ((lic_code[2] << 8)  & 0x0000FF00);
//	m_timestamp += ((lic_code[1] << 16) & 0x00FF0000);
//	m_timestamp += ((lic_code[0] << 24) & 0xFF000000);
//
//
//	m_tecnico = lic_code[4];
//	m_interval = lic_code[5];
//	m_rndnumber = lic_code[6];
//	m_chksum = lic_code[15];
//
//	chksum = 0;
//	for(i = 0; i < 15; i++)	{
//		chksum += lic_code[i];
//	}
//	chksum &= 0x00FF;
//	if(m_chksum != (uint8_t)chksum)	{
//		//state->conio->puts("*ERROR : Checksum de clave, erroneo\n\r");
//		return -1;
//	}
//
//	error = flash0_read(buffer, LIC_TIMESTAMP, 4);
//	old_timestamp  = buffer[3];
//	old_timestamp += ((buffer[2] << 8)  & 0x0000FF00);
//	old_timestamp += ((buffer[1] << 16) & 0x00FF0000);
//	old_timestamp += ((buffer[0] << 24) & 0xFF000000);
//	error = flash0_read(buffer, LIC_RNDNUM, 4);
//	old_rndnumber = buffer[0];
//
//	if((m_timestamp == old_timestamp) && (m_rndnumber == old_rndnumber))	{
//		//state->conio->puts("*ERROR : Clave rechazada por re-utilizacion\n\r");
//		return -1;
//	}
//
//	//chequeo si corresponde a esta placa por numero de serie
//	if((lic_code[7] != 0) || (lic_code[8] != 0) || (lic_code[9] != 0) || (lic_code[10] != 0))	{
//	    lic_account = lic_code[10] - '0';
//	    lic_account += (lic_code[9] - '0')*10;
//	    lic_account += (lic_code[8] - '0')*100;
//	    lic_account += (lic_code[7] - '0')*1000;
//		if(account != lic_account)	{
//			//state->conio->puts("*ERROR : Clave no corresponde a esta placa por numero de cuenta\n\r");
//			return -1;
//		}
//	}
//
//	//chequeo si corresponde a esta placa por numero de abonado
////	if(BaseAlarmPkt_numabo != m_numabo)	{
////		state->conio->puts("ERROR : Clave no corresponde a esta placa por numero de abonado\n\r");
////		return 1;
////	}
//
//
//	//aca se analiza la fecha de vencimiento de la licensia
//	//m_timestamp -= 31*24*60*60;
//        m_timestamp -= 3*60*60;
//	licvenc_ts = m_timestamp + (m_interval * 60*60);
//	buffer[0] = (licvenc_ts >> 24) & 0x000000FF;
//	buffer[1] = (licvenc_ts >> 16) & 0x000000FF;
//	buffer[2] = (licvenc_ts >> 8) & 0x000000FF;
//	buffer[3] = (licvenc_ts) & 0x000000FF;
//	error = flash0_write(1, buffer, DF_LICTSVENC_OFFSET, 4);		//guarde vencimiento de licensia
//
//
//
//
//	if(!valid_license())	{
//		//state->conio->puts("*ERROR : Licencia vencida\n\r");
//		return -1;
//	}
//
//
//	//**************************************************************************
//	//* aca la clave fue aceptada
//	state->conio->puts("OK :Clave aceptada !!!\n\r");
//	RADAR_flags |= LIC_ENTER;
//	RADAR_flags &= ~CONSOLE_CMDIN;
//	buffer[0] = m_interval;
//	error = flash0_write(1, buffer, LIC_INTERVAL, 1);
//	error = flash0_write(1, lic_code, LIC_TIMESTAMP, 4);
//	buffer[0] = m_rndnumber;
//	error = flash0_write(1, buffer, LIC_RNDNUM, 1);
//
//	return 1;
//}


int valid_license(void)
{
	uint8_t buffer[8];
	uint32_t error, ts_licvenc, ts_actual;

	if(!(SystemFlag10 & UDPUSELIC_FLAG))
	    return 1;

	error = flash0_read(buffer, DF_LICTSVENC_OFFSET, 4);

	ts_licvenc  = buffer[3];
	ts_licvenc += ((buffer[2] << 8)  & 0x0000FF00);
	ts_licvenc += ((buffer[1] << 16) & 0x00FF0000);
	ts_licvenc += ((buffer[0] << 24) & 0xFF000000);
	//ts_licvenc += 31*24*60*60;

	ts_actual = SEC_TIMER;

	if( ts_actual <= ts_licvenc)
		return 1;
	else
		return 0;

}

int con_set_radar(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set RADAR  0|1\n\r");
		retval = EepromReadByte(RADARMODE_E2P_ADDR, &error);
		if(retval == 0x5A)	{
			state->conio->puts("RADAR MODE = ON\n\r");
		} else	{
			state->conio->puts("RADAR MODE = OFF\n\r");
		}
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	retval = EepromReadByte(RADARMODE_E2P_ADDR, &error);
	if(numabo == 1)	{
		retval = 0x5A;
		SystemFlag3 |= RADAR_ENABLE;
	} else	{
		retval = 0xA5;
		SystemFlag3 &= ~RADAR_ENABLE;
		if(numabo == 255)
			retval = 0xA5;
	}

	EepromWriteByte(RADARMODE_E2P_ADDR, (uint8_t)retval, &error);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	retval = EepromReadByte(RADARMODE_E2P_ADDR, &error);


	if(retval == 0x5A)	{
		state->conio->puts("RADAR MODE = ON\n\r");
	} else	{
		state->conio->puts("RADAR MODE = OFF\n\r");
	}

	return 1;
}


int measure_np(ConsoleState* state)
{
	int i;
	float m, v, v1, v2, x1, x2, x;
	char *bufferptr, buffer[128];

	for(i = 0; i < 128; i++)
		buffer[i] = 0;

	for(i = 0; i < 26; i++)	{
		if(np_med1 < npscale[i][1])
			break;
	}

	x1 = npscale[i-1][1];
	x2 = npscale[i][1];
	v1 = npscale[i-1][0];
	v2 = npscale[i][0];
	x = np_med1;

	m = (v2 - v1)/(x2-x1);

	v = v1 + m*(x-x1);

	bufferptr = itoa((int)v);

	state->conio->puts("\n\r");
	Str_Cat(buffer,"Nivel de Portadora = ");
	buffer[21] = bufferptr[0];
	buffer[22] = '.';

	for(i = 0; bufferptr[i+1] != 0; i++)	{
		buffer[i+23] = bufferptr[i+1];
	}
	buffer[i+23] = 0;
	Str_Cat(buffer, " Volts");

	state->conio->puts(buffer);
	return 1;
}

void log_nivel_portadora(void)
{
	int i;
	float m, v, v1, v2, x1, x2, x;


	for(i = 0; i < 26; i++)	{
		if(np_med1 < npscale[i][1])
			break;
	}

	x1 = npscale[i-1][1];
	x2 = npscale[i][1];
	v1 = npscale[i-1][0];
	v2 = npscale[i][0];
	x = np_med1;

	m = (v2 - v1)/(x2-x1);

	v = v1 + m*(x-x1);

	//bufferptr = itoa((int)v);

	if( (v > 500) || (v < 150) )	{
		logCidEvent(account, 1, 920, 0, 0 );
	} else	{
		logCidEvent(account, 1, 920, 0, (uint16_t)v );
	}

}

int measure_bat(ConsoleState* state)
{
	int i;
	float m, v, v1, v2, x1, x2, x;
	char *bufferptr, buffer[128];

	for(i = 0; i < 128; i++)
		buffer[i] = 0;

	for(i = 0; i < 21; i++)	{
		if(bat_med1 < batscale[i][1])
			break;
	}



	x1 = batscale[i-1][1];
	x2 = batscale[i][1];
	v1 = batscale[i-1][0];
	v2 = batscale[i][0];
	x = bat_med1;

	m = (v2 - v1)/(x2-x1);

	v = v1 + m*(x-x1);

	bufferptr = itoa((int)v);

	state->conio->puts("\n\r");
	Str_Cat(buffer,bufferptr);
	state->conio->puts(buffer);

	if(bat_med1 >= 3360)	{
		state->conio->puts("Tension de bateria mayor a 12V\n\r ");
		return 1;
	}

	for(i = 0; i < 128; i++)
		buffer[i] = 0;
	state->conio->puts("\n\r");
	Str_Cat(buffer,"Tension de bateria = ");
	if(v >= 1000)	{
		buffer[21] = bufferptr[0];
		buffer[22] = bufferptr[1];
		buffer[23] = '.';
		for(i = 0; bufferptr[i+2] != 0; i++)	{
			buffer[i+24] = bufferptr[i+2];
		}
		buffer[i+24] = 0;
	} else	{
		buffer[21] = bufferptr[0];
		buffer[22] = '.';
		for(i = 0; bufferptr[i+1] != 0; i++)	{
			buffer[i+23] = bufferptr[i+1];
		}
		buffer[i+23] = 0;
	}



	Str_Cat(buffer, " Volts");

	state->conio->puts(buffer);
	return 1;
}


int con_set_bootmode(ConsoleState* state)
{
	uint16_t numabo;
	uint8_t retval1, retval2;
	int error;
	OS_ERR os_err;

	if( state->numparams < 3 )	{
		state->conio->puts("set BOOT  0|1\n\r");
		retval1 = EepromReadByte(BOOTMARK1_E2P_ADDR, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		retval2 = EepromReadByte(BOOTMARK2_E2P_ADDR, &error);
		if((retval1 == 0x5A) && (retval2 == 0xA5))	{
			state->conio->puts("BOOT MODE = ON\n\r");
		} else	{
			state->conio->puts("BOOT MODE = OFF\n\r");
		}
		return 1;
	}
	numabo = atoi(con_getparam(state->command, 2));
	retval1 = EepromReadByte(BOOTMARK1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	retval2 = EepromReadByte(BOOTMARK2_E2P_ADDR, &error);
	if(numabo == 1)	{
		retval1 = 0x5A;
		retval2 = 0xA5;
	} else	{
		retval1 = 0xA5;
		retval2 = 0x5A;
		if(numabo == 255)	{
			retval1 = 0xA5;
			retval2 = 0x5A;
		}
	}

	EepromWriteByte(BOOTMARK1_E2P_ADDR, (uint8_t)retval1, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	EepromWriteByte(BOOTMARK2_E2P_ADDR, (uint8_t)retval2, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	retval1 = EepromReadByte(BOOTMARK1_E2P_ADDR, &error);
	retval2 = EepromReadByte(BOOTMARK2_E2P_ADDR, &error);


	if((retval1 == 0x5A) && (retval2 == 0xA5))	{
		state->conio->puts("BOOT MODE = ON\n\r");
	} else	{
		state->conio->puts("BOOT MODE = OFF\n\r");
	}

	return 1;
}


int con_calib(ConsoleState* state)
{
	int i, len;
	int16_t	calib_offset[8], calibrated_value[8], vreal[8];
	char buffer[128];

	for( i = 0; i < 8; i++ )	{
		if(Status_Zonas[i] != ALRM_NORMAL)	{
			state->conio->puts( "\n\r");
			state->conio->puts( "********************* ERROR **********************\n\r");
			state->conio->puts( "* Todas las zonas deben estar en NORMAL con 3.4V *\n\r");
			state->conio->puts( "*  ! Verificarlo midiendolo con un voltimetro !  *\n\r");
			state->conio->puts( "**************************************************\n\r");
			return -1;
		}
	}

	for( i = 0; i < 8; i++ )	{
		calib_offset[i] = zvaluemed1[i] - 2812;
	}

	len = flash0_write(1, (uint8_t*)calib_offset, DF_Z0CALIB_OFFSET, 16);

	len = flash0_read((uint8_t*)calib_offset, DF_Z0CALIB_OFFSET, 16);
	for( i = 0; i < 8; i++ )	{
		calibrated_value[i] = zvaluemed1[i] - calib_offset[i];
		vreal[i] = (int16_t)(((float)(calibrated_value[i] - 700)/(float)616)*((float)100));
	}

	for( i = 0; i < 8; i++ )	{
		Str_Copy(buffer,"Vreal Z["); Str_Cat(buffer,itoa(i)); Str_Cat(buffer,"]:\t"); Str_Cat(buffer, itoa(vreal[i])); Str_Cat(buffer,"\t");
		state->conio->puts(buffer);
		state->conio->puts( "\n\r");
	}
	state->conio->puts( "\n\r");
	return 1;

}

int16_t get_vreal( int zone )
{
	int16_t vreal, calibrated_value;
	int16_t calib_offset[8];
	int len;

	len = flash0_read((uint8_t*)calib_offset, DF_Z0CALIB_OFFSET, 16);

	calibrated_value = zvaluemed1[zone] - calib_offset[zone];
	vreal = (int16_t)(((float)(calibrated_value - 700)/(float)616)*((float)100));

	return vreal;
}

void calibrar( void )
{
	int i, len;
	int16_t	calib_offset[8];


	for( i = 0; i < 8; i++ )	{
		if(Status_Zonas[i] != ALRM_NORMAL)	{
			return;
		}
	}

	for( i = 0; i < 8; i++ )	{
		calib_offset[i] = zvaluemed1[i] - 2812;
	}

	len = flash0_write(1, (uint8_t*)calib_offset, DF_Z0CALIB_OFFSET, 16);

	return;
}

int con_replicar(ConsoleState* state)
{
	uint8_t partition1, partition2;
	uint32_t error;
	uint8_t temp[10], i;

	if( state->numparams < 3 )	{
		state->conio->puts( "Error en la cantidad de parametros\n\r");
		state->conio->puts( "replicar particion_destino particion_origen\n\r");
		return -1;
	}

	partition1 = atoi(con_getparam(state->command, 1));
	partition2 = atoi(con_getparam(state->command, 2));

	if((partition1 < 5) || (partition1 > 9))	{
		state->conio->puts( "La particion destino debe ser entre 5 y 9\n\r");
		return -1;
	}
	if(partition2 > 99)	{
		state->conio->puts( "La particion origen debe ser entre 1 y 99\n\r");
		return -1;
	}
	partition2 = IntToBCD(atoi(con_getparam(state->command, 2)));

	error = flash0_read(temp, DF_PARTITION_OFFSET, 5);
	temp[partition1-5] = partition2;

	error = flash0_write(1, temp, DF_PARTITION_OFFSET, 5);

	for(i = 0; i < 5; i++)	{
		PT_estado_particion[i] = 0x00;
	}
	error = flash0_read(temp, DF_PARTITION_OFFSET, 5);
	for(i = 0; i < 5; i++)	{
		if(temp[i] <= 0x99)	{
			PT_estado_particion[i] = temp[i];
		}
	}

	return 1;
}



int con_volredu(ConsoleState* state)
{
    uint8_t partition1, partition2;
    uint32_t error;
    uint8_t temp[10], i;

    if( state->numparams < 3 )	{
        state->conio->puts( "Error en la cantidad de parametros\n\r");
        state->conio->puts( "redpar particion_destino particion_origen\n\r");
        return -1;
    }

    partition1 = atoi(con_getparam(state->command, 1));
    partition2 = atoi(con_getparam(state->command, 2));

    if((partition1 < 5) || (partition1 > 9))	{
        state->conio->puts( "La particion destino debe ser entre 5 y 9\n\r");
        return -1;
    }
    if(partition2 > 99)	{
        state->conio->puts( "La particion origen debe ser entre 1 y 99\n\r");
        return -1;
    }
    partition2 = IntToBCD(atoi(con_getparam(state->command, 2)));

    error = flash0_read(temp, DF_VOLREDUNT_OFFSET, 5);
    temp[partition1-5] = partition2;

    error = flash0_write(1, temp, DF_VOLREDUNT_OFFSET, 5);

    for(i = 0; i < 5; i++)	{
        VolumetricRedundance[i] = 0x00;
    }
    error = flash0_read(temp, DF_VOLREDUNT_OFFSET, 5);
    for(i = 0; i < 5; i++)	{
        if(temp[i] <= 0x99)	{
            VolumetricRedundance[i] = temp[i];
        }
    }

    return 1;
}


int con_armar(ConsoleState* state)
{
	int accion, partition2, pgroup, puni;
	uint32_t error;
	uint8_t temp[10], i;

	if( state->numparams < 3 )	{
		state->conio->puts( "Error en la cantidad de parametros\n\r");
		return -1;
	}

	accion = atoi(con_getparam(state->command, 1));
	partition2 = atoi(con_getparam(state->command, 2));


//	if((partition2 >= 99) || (partition2 < 71) )	{
//		state->conio->puts( "La particion origen debe ser entre 71 y 98\n\r");
//		return -1;
//	}

	pgroup = partition2 / 10;
	puni = partition2 % 10;

	if((puni >= 9) || (puni < 1) )	{
		state->conio->puts( "La subparticion origen debe ser entre 1 y 8\n\r");
		return -1;
	}

	if( (pgroup >= 1) && (pgroup <= 6))	{
		for( i = 0; i < MAXQTYPTM; i++ )	{
			if( ptm_dcb[i].rtuaddr == 0x00)
				continue;

			if(ptm_dcb[i].particion == partition2)	{
				if( accion == 1)
					PTM_dev_status[i] |= 0x81;
				else
				if( accion == 3)
					PTM_dev_status[i] &= ~0x81;
			}

		}
	}

	if( (pgroup >= 7) && (pgroup <= 9))	{
		if( accion == 1)	{
			PDX_dev_status[pgroup - 7] |= (1 << (puni - 1));
		} else
		if( accion == 3)	{
			PDX_dev_status[pgroup - 7] &= ~(1 << (puni - 1));
		}
		guardar_PDX_status();
	}

	return 1;
}


int con_volumetrica(ConsoleState* state)
{
	uint8_t dec_group[10];
	int particion, parti_group, parti_uni;
	uint32_t error;
	uint8_t temp[10], i, j, len_param;
	OS_ERR      err;
	char buffer[80];

	if( state->numparams < 2 )	{
		
		error = flash0_read(dec_group, DF_GROUPVOLU_OFFSET, 10);
		for(i = 0; i < 80; i++)
			buffer[i] = 0;
		state->conio->puts("\n\r");
		Str_Cat(buffer, "Volumetricas: ");

		for(i = 1; i < 10; i++)	{
			if(dec_group[i] == 0x00)
				continue;
			for(j = 0; j < 8; j++)	{
				if( dec_group[i] & (1 << j))	{
					particion = i*10 + j + 1;
					Str_Cat(buffer,itoa(particion));
					Str_Cat(buffer," ");
				}
			}
			Str_Cat(buffer, "\n\r");
			state->conio->puts(buffer);

			buffer[0] = 0;
			Str_Cat(buffer, "Volumetricas: ");
		}
		state->conio->puts("\n\r");

		return 1;
	}


	len_param = state->numparams;

	for(i = 0; i < 10; i++)	{
		dec_group[i] = 0x00;
	}

	for(i = 1; i < len_param; i++)		{
		particion = atoi(con_getparam(state->command, i));

		if(particion == 0)	{
				for(j = 0; j < 10; j++)	{
					dec_group[j] = 0x00;
				}
				error = flash0_write(1, dec_group, DF_GROUPVOLU_OFFSET, 10);
				state->conio->puts("\n\rBorrando Volumetricas ... \n\r");
		}

		parti_group = particion / 10;
		parti_uni = particion % 10;
		if((parti_group < 1) || (parti_group > 9))	{
			continue;
		}
		if((parti_uni < 1) || (parti_uni > 8))		{
			continue;
		}
		dec_group[parti_group] |= (1 << (parti_uni - 1));
	}

	error = flash0_write(1, dec_group, DF_GROUPVOLU_OFFSET, 10);

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
	error = flash0_read(dec_group, DF_GROUPVOLU_OFFSET, 10);

	for(i = 0; i < 80; i++)
		buffer[i] = 0;
	Str_Cat(buffer, "Volumetricas: ");
	state->conio->puts("\n\r");

	for(i = 1; i < 10; i++)	{
		if(dec_group[i] == 0x00)
			continue;
		for(j = 0; j < 8; j++)	{
			if( dec_group[i] & (1 << j))	{
				particion = i*10 + j + 1;
				Str_Cat(buffer,itoa(particion));
				Str_Cat(buffer," ");
			}
		}
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);

		buffer[0] = 0;
		Str_Cat(buffer, "Volumetricas: ");
	}
	state->conio->puts("\n\r");

	return 1;
}

int con_show_replicar(ConsoleState* state)
{
	int i, j;
	char buffer[80];

	for(i = 0; i < 80; i++)
		buffer[i] = 0;

	state->conio->puts( "\n\r");
	state->conio->puts( "Particion destino       Particion origen\n\r");
	state->conio->puts( "-----------------       ----------------\n\r");

	for(i = 0; i < 5; i++)	{
		for(j = 0; j < 80; j++)
			buffer[j] = 0;
		Str_Cat(buffer, "\t");Str_Cat(buffer,itoa(i+5)); Str_Cat(buffer, "\t\t\t");Str_Cat(buffer, itoa(BCDToInt(PT_estado_particion[i]))) ;Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
	}
	state->conio->puts( "\n\r");
	return 1;
}

int con_show_redpar(ConsoleState* state)
{
	int i, j;
	char buffer[80];

	for(i = 0; i < 80; i++)
		buffer[i] = 0;

	state->conio->puts( "\n\r");
	state->conio->puts( "Particion destino       Particion origen\n\r");
	state->conio->puts( "-----------------       ----------------\n\r");

	for(i = 0; i < 5; i++)	{
		for(j = 0; j < 80; j++)
			buffer[j] = 0;
		Str_Cat(buffer, "\t");Str_Cat(buffer,itoa(i+5)); Str_Cat(buffer, "\t\t\t");Str_Cat(buffer, itoa(BCDToInt(VolumetricRedundance[i]))) ;Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
	}
	state->conio->puts( "\n\r");
	return 1;
}

int con_phyeth(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set phymode 1|2|3|4 (1:AUTO, 2:100M FD, 3:100M HD, 4:10M FD, 5:10M HD, 6:ALL)\n\r");
		retval = EepromReadByte(ETHPHYMODE_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD PHYMODE = ");
		switch(retval)	{
		case 1:
			Str_Cat(buffer, "AUTONEGOTIATE\n\r");
			break;
		case 2:
			Str_Cat(buffer, "100M FULL DUPLEX\n\r");
			break;
		case 3:
			Str_Cat(buffer, "100M HALF DUPLEX\n\r");
			break;
		case 4:
			Str_Cat(buffer, "10M FULL DUPLEX\n\r");
			break;
		case 5:
			Str_Cat(buffer, "10M HALF DUPLEX\n\r");
			break;
		case 6:
			Str_Cat(buffer, "TRY ALL\n\r");
			break;
		default:
			Str_Cat(buffer, "PHYMODE TYPE ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);

		buffer[0] = 0;
		Str_Cat(buffer, "ETH  PHYMODE = ");
		switch(phymodewarm)	{
		case 1:
			Str_Cat(buffer, "AUTONEGOTIATE\n\r");
			break;
		case 2:
			Str_Cat(buffer, "100M FULL DUPLEX\n\r");
			break;
		case 3:
			Str_Cat(buffer, "100M HALF DUPLEX\n\r");
			break;
		case 4:
			Str_Cat(buffer, "10M FULL DUPLEX\n\r");
			break;
		case 5:
			Str_Cat(buffer, "10M HALF DUPLEX\n\r");
			break;
		case 6:
			Str_Cat(buffer, "TRY ALL\n\r");
			break;
		default:
			Str_Cat(buffer, "PHYMODE TYPE ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo > 0) && (cnumabo < 7))
		switch(cnumabo)	{
		case 1:
			EepromWriteByte(ETHPHYMODE_E2P_ADDR, ETH_AUTO_NEG, &error);
			break;
		case 2:
			EepromWriteByte(ETHPHYMODE_E2P_ADDR, ETH_FULLD_100M, &error);
			break;
		case 3:
			EepromWriteByte(ETHPHYMODE_E2P_ADDR, ETH_HALFD_100M, &error);
			break;
		case 4:
			EepromWriteByte(ETHPHYMODE_E2P_ADDR, ETH_FULLD_10M, &error);
			break;
		case 5:
			EepromWriteByte(ETHPHYMODE_E2P_ADDR, ETH_HALFD_10M, &error);
			break;
		case 6:
			EepromWriteByte(ETHPHYMODE_E2P_ADDR, 0x06, &error);
			break;
		}

	else	{
		state->conio->puts("*** PHYMODE TYPE ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(ETHPHYMODE_E2P_ADDR, &error);
	buffer[0] = 0;
	Str_Cat(buffer, "PGMD PHYMODE = ");

	switch(retval)	{
	case 1:
		Str_Cat(buffer, "AUTONEGOTIATE\n\r");
		break;
	case 2:
		Str_Cat(buffer, "100M FULL DUPLEX\n\r");
		break;
	case 3:
		Str_Cat(buffer, "100M HALF DUPLEX\n\r");
		break;
	case 4:
		Str_Cat(buffer, "10M FULL DUPLEX\n\r");
		break;
	case 5:
		Str_Cat(buffer, "10M HALF DUPLEX\n\r");
		break;
	case 6:
		Str_Cat(buffer, "TRY ALL\n\r");
		break;
	default:
		Str_Cat(buffer, "PHYMODE TYPE ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);
	//phymode = EepromReadByte(ETHPHYMODE_E2P_ADDR, &error);
	return 1;
}

int con_tachof220v(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set tachof220v tiempo_en_minutos\n\r");
		retval = EepromReadWord(TACHOF220_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD TACHO F220V = ");
		//sprintHexWord(tmpbuf, retval);
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta >= 0) && (cuenta < 6))
		EepromWriteWord(TACHOF220_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** TACHO DE 220V FUERA DE RANGO (0 a 6 min)***\n\r");
		return -1;
	}
	retval = EepromReadWord(TACHOF220_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD TACHO F220V = ");
	//sprintHexWord(tmpbuf, retval);
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);
	//paparst_timer = retval;

	return 1;
}

int show_dhcp_info(ConsoleState* state)
{
	int len, error, i;
	char buffer[16];



	state->conio->puts("DHCP Local IP : ");
	EepromReadBuffer(DHCPIPADD_E2P_ADDR, buffer, 16, &error);
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	state->conio->puts("DHCP Gateway :  ");
	EepromReadBuffer(DHCPGWADD_E2P_ADDR, buffer, 16, &error);
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	state->conio->puts("DHCP Netmask :  ");
	EepromReadBuffer(DHCPNMASK_E2P_ADDR, buffer, 16, &error);
	for(i = 0; i < 16; i++ )	{
		if(buffer[i] == 0x00 )
			break;
		if(buffer[i] > 127)	{
			state->conio->puts("\n\r");
			return 1;
		}
	}
	buffer[15] = 0;
	state->conio->puts(buffer);
	state->conio->puts("\n\r");

	return 1;
}

int con_SetDHCP(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set DHCP 0|1 (0:OFF, 1:ON)\n\r");
		retval = EepromReadByte(DHCPUSED_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD DHCP = ");
		switch(retval)	{
		case 0:
			Str_Cat(buffer, "OFF\n\r");
			break;
		case 1:
			Str_Cat(buffer, "ON\n\r");
			break;
		default:
			Str_Cat(buffer, "DHCP ACTIVATION ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 2));
	if((cnumabo >= 0) && (cnumabo < 2))
		EepromWriteByte(DHCPUSED_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** DHCP ACTIVATION ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(DHCPUSED_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD DHCP = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "OFF\n\r");
		//inUse[0] = FALSE;
		break;
	case 1:
		Str_Cat(buffer, "ON\n\r");
		//inUse[0] = TRUE;
		break;
	default:
		Str_Cat(buffer, "DHCP ACTIVATION ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	return 1;
}


int con_dhcpjumper(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval;
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 2 )	{
		state->conio->puts("dhcpjumper 0|1 (0:OFF, 1:ON)\n\r");
		retval = EepromReadByte(DHCPJUMPER_E2P_ADDR, &error);
		Str_Cat(buffer, "DHCP JUMPER = ");
		switch(retval)	{
		case 0:
			Str_Cat(buffer, "OFF\n\r");
			break;
		case 1:
			Str_Cat(buffer, "ON\n\r");
			break;
		default:
			Str_Cat(buffer, "DHCP JUMPER ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}
	cnumabo = atoi(con_getparam(state->command, 1));
	if((cnumabo >= 0) && (cnumabo < 2))
		EepromWriteByte(DHCPJUMPER_E2P_ADDR, (uint8_t)cnumabo, &error);
	else	{
		state->conio->puts("*** DHCP JUMPER ERROR  ***\n\r");
		return -1;
	}
	retval = EepromReadByte(DHCPJUMPER_E2P_ADDR, &error);
	Str_Cat(buffer, "DHCP JUMPER = ");
	switch(retval)	{
	case 0:
		Str_Cat(buffer, "OFF\n\r");
		//inUse[0] = FALSE;
		break;
	case 1:
		Str_Cat(buffer, "ON\n\r");
		//inUse[0] = TRUE;
		break;
	default:
		Str_Cat(buffer, "DHCP JUMPER ERROR\n\r");
		break;
	}
	state->conio->puts(buffer);

	return 1;
}

//int CypherCommands(ConsoleState* state)
//{
//	char *input_code;
//	int len, i, j;
//	uint8_t lic_code[16], buffer[12];
//	OS_ERR os_err;
//	uint16_t cuenta;
//
//	uint32_t m_timestamp, old_timestamp, error;
//	uint8_t m_numabo, m_interval, m_rndnumber, m_chksum, old_rndnumber;
//	uint16_t chksum;
//
//
//
//
//	len = strlen( con_getparam(state->command, 1) );
//	input_code = con_getparam(state->command, 1);
//
//	if(len != 32)	{
//		state->conio->puts("*ERROR : Clave de longitud incorrecta\n\r");
//		return 1;
//	}
//
//	for( i = 0; i < 32; i++ )	{
//		if(!ASCII_IsDigHex(input_code[i]))	{
//			state->conio->puts("*ERROR : Clave con caracteres invalidos\n\r");
//			return 1;
//		}
//	}
//
//	for( i = 0, j = 0; i < 32; i+=2, j++)	{
//		lic_code[j] = (hex2nibble(input_code[i]) << 4) + hex2nibble(input_code[i+1]);
//	}
//
//	aes_decrypt( lic_code, (unsigned char *)AES_key );
//
//	m_chksum = lic_code[15];
//	chksum = 0;
//	for(i = 0; i < 15; i++)	{
//		chksum += lic_code[i];
//	}
//	chksum &= 0x00FF;
//	if(m_chksum != (uint8_t)chksum)	{
//		state->conio->puts("*ERROR : Checksum de comando, erroneo\n\r");
//		return 1;
//	}
//
//	error = flash0_read(buffer, LIC_INTERVAL, 1);
//	if((buffer[0] > 0) && (buffer[0] <= 48))	{
//		//el checksum dio bien, procesamos los comandos.
//		switch( lic_code[4])	{
//			case 0x31:				//dar de alta un PTM
//			case 0x32:
//				for( i = 5; i < 15; i++)	{
//					if( lic_code[i] != 0)	{
//						error = add_ptm(lic_code[i]);
//						OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
//						if(error == -1)	{
//							state->conio->puts("*ERROR : Comando NO ejecutado, la tabla esta llena\n\r");
//							return 1;
//						}
//					}
//				}
//				break;
//			case 0x51:				//dar de baja un PTM
//			case 0x52:
//				for( i = 5; i < 15; i++)	{
//					if( lic_code[i] != 0)	{
//						error = del_ptm(lic_code[i]);
//						OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
//						if(error == -1)	{
//							state->conio->puts("*ERROR : Comando NO ejecutado, PTM inexistente\n\r");
//							return 1;
//						}
//					}
//				}
//				break;
//			case 0x53:			//programar numero de cuanta bykom
//				lic_code[9] = 0;
//				cuenta = atoi( &(lic_code[5]));
//				if((cuenta > 0) && (cuenta < 10000))	{
//					EepromWriteWord(CIDACCOUNT1_E2P_ADDR, cuenta, &error);
//					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//					EepromWriteWord(CIDACCOUNT2_E2P_ADDR, cuenta, &error);
//					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//					EepromWriteWord(R3KACCOUNT_E2P_ADDR, cuenta, &error);
//					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
//				}
//				break;
//			default:
//				break;
//		}
//		state->conio->puts("*OK : Comando ejecutado\n\r");
//	} else {
//		state->conio->puts("*ERROR : Primero debe ingresar Licencia con LIC\n\r");
//	}
//
//	return 1;
//}

int add_ptm( uint8_t ptm_partition)
{
	int i, error;
	uint8_t buffer[5];
	uint16_t address;

	//chequeo si el ptm ya existe
	for( i = 0; i < MAXQTYPTM; i++ )	{
		if( ptm_dcb[i].particion ==  ptm_partition )	{	//existe, la sobre-escribo con la normalizada
			if(ptm_partition == 58)	{
				ptm_dcb[i].rtuaddr = 230;
			} else
			if(ptm_partition == 70)	{
				ptm_dcb[i].rtuaddr = 240;
			} else
			if(ptm_partition == 80)	{
				ptm_dcb[i].rtuaddr = 241;
			} else
			if(ptm_partition == 90)	{
				ptm_dcb[i].rtuaddr = 242;
			} else	{
				ptm_dcb[i].rtuaddr = ptm_partition;
			}

			ptm_dcb[i].cuenta = 0x0000;
			ptm_dcb[i].particion = ptm_partition;
			ptm_dcb[i].disparo = '-';

			buffer[0] = ptm_dcb[i].rtuaddr;
			buffer[1] = ptm_dcb[i].particion;
			buffer[2] = (ptm_dcb[i].cuenta >> 8) & 0x00FF;
			buffer[3] = ptm_dcb[i].cuenta & 0x00FF;
			buffer[4] = ptm_dcb[i].disparo;

			address = PTM00DCB_E2P_ADDR + i*5;
			EepromWriteBuffer(address, buffer, 5, &error);
			logCidEvent(account, 1, 918, ptm_partition, 0);
			return 1;
		}
	}

	//no existia, entonces busco un lugar libre en la tabla
	for( i = 0; i < MAXQTYPTM; i++ )	{
		if( ptm_dcb[i].rtuaddr ==  0x00 )	{	//encontre el primer lugar libre
			if(ptm_partition == 58)	{
				ptm_dcb[i].rtuaddr = 230;
			} else
			if(ptm_partition == 70)	{
				ptm_dcb[i].rtuaddr = 240;
			} else
			if(ptm_partition == 80)	{
				ptm_dcb[i].rtuaddr = 241;
			} else
			if(ptm_partition == 90)	{
				ptm_dcb[i].rtuaddr = 242;
			} else	{
				ptm_dcb[i].rtuaddr = ptm_partition;
			}

			ptm_dcb[i].cuenta = 0x0000;
			ptm_dcb[i].particion = ptm_partition;
			ptm_dcb[i].disparo = '-';

			buffer[0] = ptm_dcb[i].rtuaddr;
			buffer[1] = ptm_dcb[i].particion;
			buffer[2] = (ptm_dcb[i].cuenta >> 8) & 0x00FF;
			buffer[3] = ptm_dcb[i].cuenta & 0x00FF;
			buffer[4] = ptm_dcb[i].disparo;

			address = PTM00DCB_E2P_ADDR + i*5;
			EepromWriteBuffer(address, buffer, 5, &error);
			logCidEvent(account, 1, 918, ptm_partition, 0);
			return 1;
		}
	}

	//Error, no hay lugar en la tabla para un nuevo PTM
	//state->conio->puts("ERROR : La tabla de PTM esta llena\n\r");
	return -1;
}

int del_ptm( uint8_t ptm_partition)
{
	int i, error;
	uint8_t buffer[5];
	uint16_t address;

	//chequeo si el ptm ya existe
	for( i = 0; i < MAXQTYPTM; i++ )	{
		if( ptm_dcb[i].particion ==  ptm_partition )	{	//existe, lo borro poniendolo a cero.
			
			ptm_dcb[i].rtuaddr = 0x00;
			ptm_dcb[i].cuenta = 0x0000;
			ptm_dcb[i].particion = 0x00;
			ptm_dcb[i].disparo = '-';

			buffer[0] = ptm_dcb[i].rtuaddr;
			buffer[1] = ptm_dcb[i].particion;
			buffer[2] = (ptm_dcb[i].cuenta >> 8) & 0x00FF;
			buffer[3] = ptm_dcb[i].cuenta & 0x00FF;
			buffer[4] = ptm_dcb[i].disparo;

			address = PTM00DCB_E2P_ADDR + i*5;
			EepromWriteBuffer(address, buffer, 5, &error);
			logCidEvent(account, 1, 919, ptm_partition, 0);
			return 1;
		}
	}

	return -1;
}


int con_prueballave(ConsoleState* state)
{

	uint32_t TMprueba, TMllaveon, TMllaveoff, TMgap, retries;

	if( state->numparams < 6 )	{
		return -1;
	}




	TMprueba = atoi(con_getparam(state->command, 1));
	TMllaveon = atoi(con_getparam(state->command, 2));
	TMllaveoff = atoi(con_getparam(state->command, 3));
	TMgap = atoi(con_getparam(state->command, 4));
	retries = atoi(con_getparam(state->command, 5));

	Timer_prueba = TMprueba;
	Timpr_llaveon = TMllaveon;
	Timpr_llaveoff =TMllaveoff;
	Timpr_gap = TMgap;

	if(retries == 0)
		retries = 1;
	Prretries = retries;

	PruebasFlags |= StartCPOLL_flag;

	return 1;
}

int con_txoff_largo(ConsoleState* state)
{
	int factivacion;

	if( state->numparams < 2 )	{
		state->conio->puts("*ERROR : Indicar activacion con 0 o 1\n\r");
		return -1;
	}

	factivacion = atoi(con_getparam(state->command, 1));
	if(factivacion == 1)	{
		PruebasFlags |= TXOFFCMD_flag;
	}
	else	{
		PruebasFlags &= ~TXOFFCMD_flag;
		POWER_TX_ON();
		FTXOFF_state = FTXOFF_WAIT;
		ftxoff_timer = 0;
	}

	return 1;
}

int con_txoff_corto(ConsoleState* state)
{
	int factivacion;

	if( state->numparams < 2 )	{
		state->conio->puts("*ERROR : Indicar activacion con 0 o 1\n\r");
		return -1;
	}

	factivacion = atoi(con_getparam(state->command, 1));
	if(factivacion == 1)	{
		PruebasFlags |= TXOFF2CMD_flag;
	}
	else	{
		PruebasFlags &= ~TXOFF2CMD_flag;
		POWER_TX_ON();
		FTXOFF2_state = FTXOFF2_WAIT;
		ftxoff2_timer = 0;
	}

	return 1;
}

uint8_t gralbuffer[528];

int con_dflash_dump(ConsoleState* state)
{
    uint8_t j;
    uint32_t i, len, offset;
    char buffer[128], temp[8];
    int page;
    OS_ERR os_err;

    for(i = 0; i < 528; i++)	{
    	gralbuffer[i] ='U';
    }

    page = atoi( con_getparam(state->command, 1));

    state->conio->puts("\n\r");

    len = 528;
    offset = page*528;

    flash0_read(gralbuffer, offset, len);

    for( i = 0; i < len; i += 16 )	{
        sprintHexDWord(buffer, offset + i);Str_Cat(buffer,"\t");
        for( j = 0; j < 16; j++ )	{
            sprintByte(temp, gralbuffer[i+j]); Str_Cat(buffer, temp);
            Str_Cat(buffer," ");
            if( j == 7 )
                Str_Cat(buffer,"- ");
        }
        Str_Cat(buffer,"\t");
        for( j = 0; j < 16; j++ )	{
            if((gralbuffer[i+j] >= 0x20) && (gralbuffer[i+j] <= 0x7F)) {
            	temp[0] = gralbuffer[i + j]; temp[1] = 0;
                //sprintByte(temp, gralbuffer[i + j]);
                Str_Cat(buffer, temp);
            }
            else Str_Cat(buffer,".");
        }
        Str_Cat(buffer,"\n\r");
        state->conio->puts(buffer);
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }

    return 1;
}

int con_ucflash_dump(ConsoleState* state)
{
    uint8_t j;
    uint32_t i, len, offset;
    char buffer[128], temp[16];
    int page;
    uint8_t  *memptr;
    uint32_t uC_flash_start, uC_flash_end;
    OS_ERR os_err;


    page = atoi( con_getparam(state->command, 1));

    state->conio->puts("\n\r");

    len = 528;

    uC_flash_start = 0x00008000 + page*len;
    uC_flash_end = uC_flash_start + len;

    for(i = 0; i < 528; i++)	{
        memptr = (uint8_t *)(uC_flash_start + i);
        gralbuffer[i] = *memptr;
    }

    for( i = 0; i < len; i += 16 )	{
        sprintHexDWord(buffer, uC_flash_start + i); Str_Cat(buffer,"\t");
        for( j = 0; j < 16; j++ )	{
            sprintByte(temp, gralbuffer[i+j]); Str_Cat(buffer, temp);
            Str_Cat(buffer," ");
            if( j == 7 )
                Str_Cat(buffer,"- ");
        }
        Str_Cat(buffer,"\t");
        for( j = 0; j < 16; j++ )	{
            if((gralbuffer[i+j] >= 0x20) && (gralbuffer[i+j] <= 0x7F)) {
                temp[0] = gralbuffer[i + j]; temp[1] = 0;
                //sprintByte(temp, gralbuffer[i + j]);
                Str_Cat(buffer, temp);
            }
            else Str_Cat(buffer,".");
        }
        Str_Cat(buffer,"\n\r");
        state->conio->puts(buffer);
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }

    return 1;
}


int con_rvb_dump_dev(ConsoleState* state)
{
	char buffer[128];
	int i;

	Mem_Clr(buffer, 128);

	for(i = 0; i < MAXQTYPTM; i++)	{
		if(ptm_dcb[i].rtuaddr != 0x00)	{
			Str_Cat(buffer, itoa(i)); Str_Cat(buffer, ":");
			Str_Cat(buffer, itoa(ptm_dcb[i].particion)); Str_Cat(buffer, ";");
		}
	}
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	return 1;
}

int con_pakterrors(ConsoleState* state) {
    int i, qtydeclared;
    char buffer[8], tmpbuffer[10];

    qtydeclared = 0;
    for( i = 0; i <= MAXQTYPTM; i++ ) {
        if( ptm_dcb[i].rtuaddr != 0x00 )    {
            qtydeclared++;
        }
    }

    state->conio->puts("\n");
    Mem_Clr(buffer, sizeof(buffer));

    state->conio->puts("Dispositivos declarados :\t");
    state->conio->puts( itoa(qtydeclared));
    state->conio->puts( "\n\r");

    state->conio->puts("Errores por vuelta :\t\t");
    state->conio->puts( itoa(lan485errorpkt));
    state->conio->puts( "\n\r");

    if(qtydeclared != 0) {
        state->conio->puts("Eficacia :\t\t\t");
        state->conio->puts(itoa((100 - (lan485errorpkt * 100) / qtydeclared)));
        state->conio->puts(" %\n\r\n\r");
    }

    state->conio->puts("Errores acumulados :\t\t");
    state->conio->puts( itoa(accumulated_errors));
    state->conio->puts( "\n\r");

    if( totalpakets != 0) {
        state->conio->puts("Eficacia acumulada :\t\t");
        state->conio->puts(itoa((100 - (accumulated_errors*100) / totalpakets)));
        state->conio->puts("\n\r");
    }

    return 1;
}


int con_dump_monstruct(ConsoleState* state)
{
	int i;
	char buffer[128], tmpbuffer[10];
	

	state->conio->puts( "\n");


	Str_Copy(buffer,"inuse:\t"); state->conio->puts(buffer);
	conio_printHexWord(state, Monitoreo[0].inuse);
	state->conio->puts( "\n");

	Str_Copy(buffer,"wdogstate:\t");
	switch(Monitoreo[0].wdogstate)	{
		case WR3K_IDLE:
			Str_Cat(buffer,"WR3K_IDLE\n");
			break;
		case WR3K_WDOG:
			Str_Cat(buffer,"WR3K_WDOG\n");
			break;
		case WR3K_WRST:
			Str_Cat(buffer,"WR3K_WRST\n");
			break;
		default:
			Str_Cat(buffer,"WRONG STATE\n");
			break;
	}
	state->conio->puts(buffer);

	Str_Copy(buffer,"flags:\t"); state->conio->puts(buffer);
	conio_printHexWord(state, Monitoreo[0].flags);
	state->conio->puts( "\n");

	Str_Copy(buffer,"SEC_TIMER:\t"); Str_Cat(buffer,itoa(SEC_TIMER)); state->conio->puts(buffer);
	state->conio->puts( "\n");

	Str_Copy(buffer,"wdogr3kTimer:\t"); Str_Cat(buffer,itoa(Monitoreo[0].wdogr3kTimer)); state->conio->puts(buffer);
	state->conio->puts( "\n");
	Str_Copy(buffer,"SystemFlag4:\t"); sprintByte(tmpbuffer, SystemFlag4); Str_Cat(buffer, tmpbuffer); Str_Cat(buffer,"\n");state->conio->puts(buffer);
	
	state->conio->puts( "\n");


	return 1;
}



int con_castrotu(ConsoleState* state)
{
	uint16_t cnumabo;
	uint8_t retval, temp[8];
	int error;

	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 2 )	{
		state->conio->puts("castrotu 0|1 (0:OFF, 1:ON)\n\r");
		flash0_read(temp, DF_CASTROTU_OFFSET, 2);
		if((temp[0] == 0x5A) && (temp[1] == 0xA5))	
			retval = 1;
		else
			retval = 0;

		Str_Cat(buffer, "CASTROTU = ");
		switch(retval)	{
		case 0:
			Str_Cat(buffer, "OFF\n\r");
			break;
		case 1:
			Str_Cat(buffer, "ON\n\r");
			break;
		default:
			Str_Cat(buffer, "ACTIVATION ERROR\n\r");
			break;
		}
		state->conio->puts(buffer);
		return 1;
	}

	cnumabo = atoi(con_getparam(state->command, 1));
	if( cnumabo == 1) 	{
		buffer[0] = 0x5A;
		buffer[1] = 0xA5;
		error = flash0_write(1, buffer, DF_CASTROTU_OFFSET, 2);
		SystemFlag10 |= CASTROTUSND_FLAG;
	}
	else	{
		buffer[0] = 0xA5;
		buffer[1] = 0x5A;
		error = flash0_write(1, buffer, DF_CASTROTU_OFFSET, 2);
		SystemFlag10 &= ~CASTROTUSND_FLAG;
	}


	return 1;
}

int con_rffilter1(ConsoleState* state)
{
    uint16_t cnumabo;
    uint8_t retval, temp[8];
    int error;

    char buffer[32];

    buffer[0] = 0;

    if( state->numparams < 2 )	{
        state->conio->puts("rffilter1 0|1 (0:OFF, 1:ON)\n\r");
        flash0_read(temp, DF_RFFILTER1_OFFSET, 2);
        if((temp[0] == 0x5A) && (temp[1] == 0xA5))
            retval = 1;
        else
            retval = 0;

        Str_Cat(buffer, "RFFILTER1 = ");
        switch(retval)	{
            case 0:
                Str_Cat(buffer, "OFF\n\r");
                break;
            case 1:
                Str_Cat(buffer, "ON\n\r");
                break;
            default:
                Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                break;
        }
        state->conio->puts(buffer);
        return 1;
    }

    cnumabo = atoi(con_getparam(state->command, 1));
    if( cnumabo == 1) 	{
        buffer[0] = 0x5A;
        buffer[1] = 0xA5;
        error = flash0_write(1, buffer, DF_RFFILTER1_OFFSET, 2);
        SysFlag4 |= RFFILTER1;
    }
    else	{
        buffer[0] = 0xA5;
        buffer[1] = 0x5A;
        error = flash0_write(1, buffer, DF_RFFILTER1_OFFSET, 2);
        SysFlag4 &= ~RFFILTER1;
    }


    return 1;
}

int con_rffilter2(ConsoleState* state)
{
    uint16_t cnumabo;
    uint8_t retval, temp[8];
    int error;

    char buffer[32];

    buffer[0] = 0;

    if( state->numparams < 2 )	{
        state->conio->puts("rffilter2 0|1 (0:OFF, 1:ON)\n\r");
        flash0_read(temp, DF_RFFILTER2_OFFSET, 2);
        if((temp[0] == 0x5A) && (temp[1] == 0xA5))
            retval = 1;
        else
            retval = 0;

        Str_Cat(buffer, "RFFILTER2 = ");
        switch(retval)	{
            case 0:
                Str_Cat(buffer, "OFF\n\r");
                break;
            case 1:
                Str_Cat(buffer, "ON\n\r");
                break;
            default:
                Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                break;
        }
        state->conio->puts(buffer);
        return 1;
    }

    cnumabo = atoi(con_getparam(state->command, 1));
    if( cnumabo == 1) 	{
        buffer[0] = 0x5A;
        buffer[1] = 0xA5;
        error = flash0_write(1, buffer, DF_RFFILTER2_OFFSET, 2);
        SysFlag4 |= RFFILTER2;
    }
    else	{
        buffer[0] = 0xA5;
        buffer[1] = 0x5A;
        error = flash0_write(1, buffer, DF_RFFILTER2_OFFSET, 2);
        SysFlag4 &= ~RFFILTER2;
    }


    return 1;
}

int con_licactivation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	error = flash0_write(1, buffer, DF_LICFLAG_OFFSET, 2);
    WDT_Feed();
    SystemFlag6 |= USE_LICENSE;

	return 1;
}

int con_licdeactivation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0xAA;
	buffer[1] = 0xBB;
	error = flash0_write(1, buffer, DF_LICFLAG_OFFSET, 2);
    WDT_Feed();
    SystemFlag6 &= ~USE_LICENSE;

	return 1;
}

int con_radarctivation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	error = flash0_write(1, buffer, DF_ARADAR_OFFSET, 2);
    WDT_Feed();
    SystemFlag6 |= USE_ARADAR;

	return 1;
}

int con_radardeactivation(ConsoleState* state)
{
    uint8_t buffer[4];
    int error;


    buffer[0] = 0xAA;
    buffer[1] = 0xBB;
    error = flash0_write(1, buffer, DF_ARADAR_OFFSET, 2);
    WDT_Feed();
    SystemFlag6 &= ~USE_ARADAR;

    return 1;
}

int con_ince2activation(ConsoleState* state)
{
    uint8_t buffer[4];
    int error;


    buffer[0] = 0x5A;
    buffer[1] = 0xA5;
    error = flash0_write(1, buffer, DF_ACTINCE2_OFFSET, 2);
    WDT_Feed();
    SystemFlag11 |= INCE2MODE_FLAG;

    return 1;
}

int con_ince2deactivation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0xAA;
	buffer[1] = 0xBB;
	error = flash0_write(1, buffer, DF_ACTINCE2_OFFSET, 2);
    WDT_Feed();
    SystemFlag11 &= ~INCE2MODE_FLAG;

	return 1;
}

int con_ASALTOactivation(ConsoleState* state)
{

	SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
	state->conio->puts("OK\n\r");
	logCidEvent(account, 1, 889, 0, 0);

	return 1;
}

int con_rhb(ConsoleState* state)
{

	SystemFlag6 |= RHB_FLAG;
	state->conio->puts("OK\n\r");
	return 1;
}

int con_rhb_activation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	error = flash0_write(1, buffer, DF_ENARHB_OFFSET, 2);
    WDT_Feed();
    SystemFlag6 |= ENARHB_FLAG;
    timer_rhb = TMINRHB*60;

	return 1;
}

int con_rhb_deactivation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0xAA;
	buffer[1] = 0xBB;
	error = flash0_write(1, buffer, DF_ENARHB_OFFSET, 2);
    WDT_Feed();
    SystemFlag6 &= ~ENARHB_FLAG;

	return 1;
}

//
//int con_evsend_activation(ConsoleState* state)
//{
//    uint8_t buffer[4];
//    int error;
//
//
//    buffer[0] = 0x5A;
//    buffer[1] = 0xA5;
//    error = flash0_write(1, buffer, DF_EVSEND_OFFSET, 2);
//    WDT_Feed();
//    SystemFlag10 |= UDPLICOK_FLAG;
//    SystemFlag10 |= UDPUSELIC_FLAG;
//
//    return 1;
//}
//
//int con_evsend_deactivation(ConsoleState* state)
//{
//    uint8_t buffer[4];
//    int error;
//
//
//    buffer[0] = 0xAA;
//    buffer[1] = 0xBB;
//    error = flash0_write(1, buffer, DF_EVSEND_OFFSET, 2);
//    WDT_Feed();
//    SystemFlag10 |= UDPLICOK_FLAG;
//    SystemFlag10 &= ~UDPUSELIC_FLAG;
//
//    return 1;
//}
//

int con_IP150_activation(ConsoleState* state)
{
    uint8_t buffer[4];
    int error;


    buffer[0] = 0x5A;
    buffer[1] = 0xA5;
    error = flash0_write(1, buffer, DF_IP150CHK_OFFSET, 2);
    WDT_Feed();
    SystemFlag7 |= IP150_CHECK;
    fsmwdip150_state = FSMWDIP150_ENTRY;

    return 1;
}

int con_INPATTERN_activation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	error = flash0_write(1, buffer, DF_INPATTERN_OFFSET, 2);
	WDT_Feed();
	SystemFlag7 |= INPATT_CHECK;


	return 1;
}

int con_INPATTERN_deactivation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error;


	buffer[0] = 0xAA;
	buffer[1] = 0xBB;
	error = flash0_write(1, buffer, DF_INPATTERN_OFFSET, 2);
	WDT_Feed();
	SystemFlag7 &= ~INPATT_CHECK;


	return 1;
}

int con_INPATTERNASK(ConsoleState* state) {
    uint8_t retval, temp[8];


    char buffer[32];

    flash0_read(temp, DF_INPATTERN_OFFSET, 2);
    if ((temp[0] == 0x5A) && (temp[1] == 0xA5))
        retval = 1;
    else if((temp[0] == 0xAA) && (temp[1] == 0xBB))
        retval = 0;
    else
        retval = 2;

    Str_Cat(buffer, "INPATTCHECK = ");
    switch (retval) {
        case 0:
            Str_Cat(buffer, "OFF\n\r");
            break;
        case 1:
            Str_Cat(buffer, "ON\n\r");
            break;
        default:
            Str_Cat(buffer, "ACTIVATION ERROR\n\r");
            break;
    }
    state->conio->puts(buffer);
    return 1;
}

int con_IP150_deactivation(ConsoleState* state)
{
    uint8_t buffer[4];
    int error;


    buffer[0] = 0xAA;
    buffer[1] = 0xBB;
    error = flash0_write(1, buffer, IP150CHECK_E2P_ADDR, 2);
    WDT_Feed();
    SystemFlag7 &=  ~IP150_CHECK;

    return 1;
}

int con_normalrm(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, num;



	num = atoi(con_getparam(state->command, 2));

	switch(num)	{
		case 1:
			SysFlag2 |= NORM_ASAL;
			BaseAlarmPkt_alarm &= ~bitpat[ASAL_bit];
			asal_autr_counter = 0;
			asal_autr_timer = 0;
			break;
		case 2:
			SysFlag2 |= NORM_INCE;
			BaseAlarmPkt_alarm &= ~bitpat[INCE_bit];
			ince_autr_counter = 0;
			ince_autr_timer = 0;
			break;
		case 3:
			SysFlag2 |= NORM_TESO;
			BaseAlarmPkt_alarm &= ~bitpat[TESO_bit];
			teso_autr_counter = 0;
			teso_autr_timer = 0;
			break;
	}
    return 1;
}

unsigned char fsmrhbstate;
//#define FSMRHB_IDLE		0x10
//#define FSMRHB_TRGD		0x20

uint32_t timer_rhb;

void fsm_rhb_wdog( void )
{
	if(!(SystemFlag7 & RHBVOLU_FLAG))	{
		fsmrhbstate = FSMRHB_IDLE;
		timer_rhb = RHB_Time*60;
		return;
	}

	switch(fsmrhbstate)	{
		case FSMRHB_IDLE:
			if(!timer_rhb)	{
				fsmrhbstate = FSMRHB_PRETRGD;
				timer_rhb = RHBCAST_Time*60;
				SystemFlag7 |= RHBBUZZON_FLAG;
				SystemFlag7 &= ~RHBNETOK_FLAG;
                SystemFlag7 &= ~CASTDVR_ACK;
				logCidEvent(account, 1, 891, 0, 0);
			} else
            if(SystemFlag6 & RHB_FLAG)	{
			//if((SystemFlag6 & RHB_FLAG) || (SystemFlag7 & IP150_ALIVE))	{
				SystemFlag6 &= ~RHB_FLAG;
				timer_rhb = RHB_Time*60;
			}
			break;
		case FSMRHB_PRETRGD:
			if(!timer_rhb)	{
				if(SystemFlag10 & CASTROTUSND_FLAG)	{
					SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
				}
				logCidEvent(account, 1, 892, 0, 0);
				fsmrhbstate = FSMRHB_TRGD;
			} else
			if(SystemFlag7 & CASTDVR_ACK)	{
				fsmrhbstate = FSMRHB_MANCOP;
				SystemFlag7 &= ~RHBBUZZON_FLAG;
			} else
            if(SystemFlag6 & RHB_FLAG)	{
			//if((SystemFlag6 & RHB_FLAG) || (SystemFlag7 & IP150_ALIVE))	{
				fsmrhbstate = FSMRHB_TRGD;
			}
			break;
		case FSMRHB_TRGD:
            if(SystemFlag6 & RHB_FLAG)	{
			//if((SystemFlag6 & RHB_FLAG) || (SystemFlag7 & IP150_ALIVE))	{
				fsmrhbstate = FSMRHB_IDLE;
				SystemFlag6 &= ~RHB_FLAG;
				timer_rhb = RHB_Time*60;
				SystemFlag7 &= ~RHBBUZZON_FLAG;
				SystemFlag7 |= RHBNETOK_FLAG;
				logCidEvent(account, 3, 891, 0, 0);
			}
			break;
		case FSMRHB_MANCOP:
			if(!(SystemFlag7 & CASTDVR_ACK) )	{
				fsmrhbstate = FSMRHB_PRETRGD;
				timer_rhb = RHBCAST_Time*60;
				SystemFlag7 |= RHBBUZZON_FLAG;
				SystemFlag7 &= ~RHBNETOK_FLAG;
				logCidEvent(account, 1, 891, 0, 0);
			} else
            if(SystemFlag6 & RHB_FLAG)	{
			//if((SystemFlag6 & RHB_FLAG) || (SystemFlag7 & IP150_ALIVE))	{
				fsmrhbstate = FSMRHB_IDLE;
				SystemFlag6 &= ~RHB_FLAG;
				timer_rhb = RHB_Time*60;
				SystemFlag7 &= ~RHBBUZZON_FLAG;
				SystemFlag7 |= RHBNETOK_FLAG;
				logCidEvent(account, 3, 891, 0, 0);
			}
			break;
		default:
			fsmrhbstate = FSMRHB_IDLE;
			timer_rhb = RHB_Time*60;
			break;
	}
}

int con_ptmbuzzon(ConsoleState* state)
{
	SystemFlag7 |= RHBBUZZON_FLAG;
	return 1;
}

int con_ptmbuzzoff(ConsoleState* state)
{
	SystemFlag7 &= ~RHBBUZZON_FLAG;
	return 1;
}

int con_timerhb(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set RHB hearbeat_en_minutos\n\r");
		retval = EepromReadWord(RHBTIMER_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD RHB = ");
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 1) && (cuenta < 0xFFFF))
		EepromWriteWord(RHBTIMER_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** HB OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(RHBTIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD RHB = ");
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	return 1;
}

int con_timerhbcast(ConsoleState* state)
{
	uint16_t cuenta;
	uint16_t retval;
	int error;
	char *tmpbuf;
	char buffer[32];

	buffer[0] = 0;

	if( state->numparams < 3 )	{
		state->conio->puts("set RHBCAST hearbeat_en_minutos\n\r");
		retval = EepromReadWord(RHBCTIMER_E2P_ADDR, &error);
		Str_Cat(buffer, "PGMD RHB CAST = ");
		tmpbuf = itoa(retval);
		Str_Cat(buffer, tmpbuf);
		Str_Cat(buffer, "\n\r");
		state->conio->puts(buffer);
		return 1;
	}
	cuenta = atoi(con_getparam(state->command, 2));
	if((cuenta > 1) && (cuenta < 0xFFFF))
		EepromWriteWord(RHBCTIMER_E2P_ADDR, cuenta, &error);
	else	{
		state->conio->puts("*** HB OUT OF RANGE  ***\n\r");
		return -1;
	}
	retval = EepromReadWord(RHBCTIMER_E2P_ADDR, &error);
	Str_Cat(buffer, "PGMD RHB CAST = ");
	tmpbuf = itoa(retval);
	Str_Cat(buffer, tmpbuf);
	Str_Cat(buffer, "\n\r");
	state->conio->puts(buffer);

	return 1;
}

int con_IRI_activation(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;

#ifdef USAR_IRIDIUM
	if( state->numparams < 3 )	{
		state->conio->puts("set IRIDIUM 0|1 (0:OFF, 1:ON)\n\r");
		//state->conio->puts(buffer);
		return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	if( value == 1)	{
		buffer[0] = 0x5A;
		buffer[1] = 0xA5;
		error = flash0_write(1, buffer, DF_ENAIRI_OFFSET, 2);
		WDT_Feed();
		IRIDIUM_flag |= IRI_USEIRIDIUM_FLAG;

		//-------------------------------------
		// Activando modem Iridium
		state->conio->puts("Activando modem Iridium ...     Espere por favor\n\r");
		//state->conio->puts(buffer);
		IridiumSBD_begin();
		FSMIRI_state = FSMIRI_IDLE;
		hbiri_time = 47;
		HBIRI_timer = SEC_TIMER + hbiri_time;
		iphbiri_timer = SEC_TIMER + 15*60;

		if(IRIDIUM_flag & IRI_IRIDIUMOK_FLAG)	{
			state->conio->puts("Modem Iridium Detectado Correctamente\n\r");
			//state->conio->puts(buffer);
			logCidEvent(account, 1, 997, 0, 8);
		} else	{
			state->conio->puts("ERROR:  No se detecto modem Iridium conectado\n\r");
			//state->conio->puts(buffer);
			//Iridium activado pero no ok
			if((IRIDIUM_flag & IRI_USEIRIDIUM_FLAG) && (!(IRIDIUM_flag & IRI_IRIDIUMOK_FLAG)))	{
				logCidEvent(account, 1, 997, 0, 6); 
			}
		}
		//-------------------------------------
	} else {
		buffer[0] = 0xAA;
		buffer[1] = 0xBB;
		error = flash0_write(1, buffer, DF_ENAIRI_OFFSET, 2);
		WDT_Feed();
		IRIDIUM_flag &= ~IRI_USEIRIDIUM_FLAG;
		state->conio->puts("Modem Iridium desactivado\n\r");
		//state->conio->puts(buffer);
	}
#endif
	return 1;
}

int con_E393HAB_activation(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, value;


    if( state->numparams < 3 )	{
        state->conio->puts("set E393HAB 0|1 (0:OFF, 1:ON)\n\r");
        //state->conio->puts(buffer);
        return 1;
    }

    value = atoi(con_getparam(state->command, 2));

    if( value == 1)	{
        buffer[0] = 0x5A;
        buffer[1] = 0xA5;
        error = flash0_write(1, buffer, DF_E393HAB_OFFSET, 2);
        WDT_Feed();
        RFDLYBOR_flag |= RFDLYBOR_E393HAB_FLAG;

    } else {
        buffer[0] = 0xAA;
        buffer[1] = 0xBB;
        error = flash0_write(1, buffer, DF_E393HAB_OFFSET, 2);
        WDT_Feed();
        RFDLYBOR_flag &= ~RFDLYBOR_E393HAB_FLAG;

    }

    return 1;
}

int con_PPONWDOG(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, value;


    if( state->numparams < 3 )	{
        state->conio->puts("set PPONWDOG 0|1 (0:OFF, 1:ON)\n\r");
        //state->conio->puts(buffer);
        return 1;
    }

    value = atoi(con_getparam(state->command, 2));

    if( value == 1)	{
        buffer[0] = 0x5A;
        buffer[1] = 0xA5;
        error = flash0_write(1, buffer, DF_PPONWDOG_OFFSET, 2);
        WDT_Feed();
        SysFlag4 |= USEPPONWDOG_flag;

    } else {
        buffer[0] = 0xAA;
        buffer[1] = 0xBB;
        error = flash0_write(1, buffer, DF_PPONWDOG_OFFSET, 2);
        WDT_Feed();
        SysFlag4 &= ~USEPPONWDOG_flag;

    }

    return 1;
}

int con_netrsthab(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, value;

    if( state->numparams < 3 )	{
        state->conio->puts("set NETRSTHAB 0|1 (0:Deshabilitado, 1:Habilitado)\n\r");
        flash0_read(buffer, DF_NRSTHAB_OFFSET, 2);
        if((buffer[0] == 0xA5) && (buffer[1] == 0x5A))  {
            state->conio->puts("Reset Interrupcion de Red DESHABILITADO\n\r");
        } else if((buffer[0] == 0x5A) && (buffer[1] == 0xA5))   {
            state->conio->puts("Reset Interrupcion de Red HABILITADO\n\r");
        } else  {
            state->conio->puts("Reset Interrupcion de Red NO CONFIGURADO\n\r");
        }
        return 1;
    }

    value = atoi(con_getparam(state->command, 2));

    switch(value)	{
        case 0:
            buffer[0] = 0xA5;
            buffer[1] = 0x5A;
            error = flash0_write(1, buffer, DF_NRSTHAB_OFFSET, 2);
            WDT_Feed();
            DebugFlag &= ~NETRSTHAB_flag;
            break;
        case 1:
            buffer[0] = 0x5A;
            buffer[1] = 0xA5;
            error = flash0_write(1, buffer, DF_NRSTHAB_OFFSET, 2);
            WDT_Feed();
            DebugFlag |= NETRSTHAB_flag;
            break;
    }

    return 1;
}

int con_higrsthab(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, value;

    if( state->numparams < 3 )	{
        state->conio->puts("set HIGRSTHAB 0|1 (0:Deshabilitado, 1:Habilitado)\n\r");
        flash0_read(buffer, DF_HRSTHAB_OFFSET, 2);
        if((buffer[0] == 0xA5) && (buffer[1] == 0x5A))  {
            state->conio->puts("Reset Higienico DESHABILITADO\n\r");
        } else if((buffer[0] == 0x5A) && (buffer[1] == 0xA5))   {
            state->conio->puts("Reset Higienico HABILITADO\n\r");
        } else  {
            state->conio->puts("Reset Higienico NO CONFIGURADO\n\r");
        }
        return 1;
    }

    value = atoi(con_getparam(state->command, 2));

    switch(value)	{
        case 0:
            buffer[0] = 0xA5;
            buffer[1] = 0x5A;
            error = flash0_write(1, buffer, DF_HRSTHAB_OFFSET, 2);
            WDT_Feed();
            DebugFlag &= ~HIGRSTHAB_flag;
            break;
        case 1:
            buffer[0] = 0x5A;
            buffer[1] = 0xA5;
            error = flash0_write(1, buffer, DF_HRSTHAB_OFFSET, 2);
            WDT_Feed();
            DebugFlag |= HIGRSTHAB_flag;
            break;
    }

    return 1;
}

int con_IRI_mode(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;

#ifdef USAR_IRIDIUM
	if( state->numparams < 3 )	{
		state->conio->puts("set IRIMODE 0|1|2|3|4 (0:OFF, 1:IP DOWN, 2:RF DOWN, 3:IP+RF DOWN, 4:ALWAYS, 5:IP+RF+GPRS DOWN)\n\r");
		//state->conio->puts(buffer);
		return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	switch(value)	{
		case 0:
			buffer[0] = 0x5A;
			buffer[1] = 0x0;
			error = flash0_write(1, buffer, DF_IRIMODE_OFFSET, 2);
			WDT_Feed();
			IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
			IRI_mode = 0;
			break;
		case 1:
			buffer[0] = 0x5A;
			buffer[1] = 0x1;
			error = flash0_write(1, buffer, DF_IRIMODE_OFFSET, 2);
			WDT_Feed();
			IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
			IRIDIUM_flag |= IRI_IPDWNHAB_FLAG;
			IRI_mode = 1;
			break;
		case 2:
			buffer[0] = 0x5A;
			buffer[1] = 0x2;
			error = flash0_write(1, buffer, DF_IRIMODE_OFFSET, 2);
			WDT_Feed();
			IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
			IRIDIUM_flag |= IRI_RFDWNHAB_FLAG;
			IRI_mode = 2;
			break;
		case 3:
			buffer[0] = 0x5A;
			buffer[1] = 0x3;
			error = flash0_write(1, buffer, DF_IRIMODE_OFFSET, 2);
			WDT_Feed();
			IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
			IRIDIUM_flag |= IRI_BOTHDWNHAB_FLAG;
			IRI_mode = 3;
			break;
		case 4:
			buffer[0] = 0x5A;
			buffer[1] = 0x04;
			error = flash0_write(1, buffer, DF_IRIMODE_OFFSET, 2);
			WDT_Feed();
			IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
			IRIDIUM_flag |= IRI_ALWAYSHAB_FLAG;
			IRI_mode = 4;
			break;
		case 5:
			buffer[0] = 0x5A;
			buffer[1] = 0x5;
			error = flash0_write(1, buffer, DF_IRIMODE_OFFSET, 2);
			WDT_Feed();
			IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
			IRIDIUM_flag |= IRI_BOTHDWNHAB_FLAG;
			IRIDIUM_flag |= IRI_GPRSDWN_FLAG;
			IRI_mode = 5;
			break;
		default:
			break;
	}
#endif
	return 1;
}

int con_setPtmDly(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, value;

    char *tmpbuf;
    char buffer1[32];


    if( state->numparams < 3 )	{
        state->conio->puts("set PTMDLY ddd, con ddd >= 0 y ddd <= 255, ddd en segundos\n\r\n");

        flash0_read(buffer, DF_PTMDLY_OFFSET, 2);

        if(buffer[0] != 0x5A)	{
            state->conio->puts("PTMDLY no programado\n\r");
            return -1;
        }
        Str_Cat(buffer1, "PGMD PTMDLY = ");
        //sprintHexWord(tmpbuf, retval);
        tmpbuf = itoa(buffer[1]);
        Str_Cat(buffer1, tmpbuf);
        Str_Cat(buffer1, "\n\r");
        state->conio->puts(buffer1);
        return 1;
    }

    value = atoi(con_getparam(state->command, 2));

    if((value < 0) || (value > 255))	{
        state->conio->puts("ERROR el tiempo de ptmdly debe ser menor a 255 segundos\n\r");
        return -1;
    }

    buffer[0] = 0x5A;
    buffer[1] = value;
    error = flash0_write(1, buffer, DF_PTMDLY_OFFSET, 2);
    WDT_Feed();

    rfdly_time = value;

    return 1;
}

int con_setBorDly(ConsoleState* state)
{
    uint8_t buffer[4];
    int error, value;
    char *tmpbuf;
    char buffer1[32];



    if( state->numparams < 3 )	{
        state->conio->puts("set BORDLY ddd, con ddd >= 0 y ddd <= 255, ddd en segundos\n\r");


        flash0_read(buffer, DF_BORDLY_OFFSET, 2);

        if(buffer[0] != 0x5A)	{
            state->conio->puts("BORDLY no programado\n\r");
            return -1;
        }
        Str_Cat(buffer1, "PGMD BORDLY = ");
        //sprintHexWord(tmpbuf, retval);
        tmpbuf = itoa(buffer[1]);
        Str_Cat(buffer1, tmpbuf);
        Str_Cat(buffer1, "\n\r");
        state->conio->puts(buffer1);
        return 1;
    }

    value = atoi(con_getparam(state->command, 2));

    if((value < 0) || (value > 255))	{
        state->conio->puts("ERROR el tiempo de bordly debe ser menor a 255 segundos\n\r");
        return -1;
    }

    buffer[0] = 0x5A;
    buffer[1] = value;
    error = flash0_write(1, buffer, DF_BORDLY_OFFSET, 2);
    WDT_Feed();

    DlyBor_time = value;

    return 1;
}

int con_setAutoreset(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;
    char *tmpbuf;
    char buffer1[32];


	if( state->numparams < 3 )	{
		state->conio->puts("set AUTORESET ddd, con ddd > 0 y ddd <= 190, ddd en minutos\n\r");

        flash0_read(buffer, DF_AUTORST_OFFSET, 2);

        if(buffer[0] != 0x5A)	{
            state->conio->puts("AUTORESET no programado\n\r");
            return -1;
        }
        Str_Cat(buffer1, "PGMD AUTORESET = ");
        //sprintHexWord(tmpbuf, retval);
        tmpbuf = itoa(buffer[1]);
        Str_Cat(buffer1, tmpbuf);
        Str_Cat(buffer1, "\n\r");
        state->conio->puts(buffer1);
        return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	if((value > AUTORST_TIMER_MAX) || (value < AUTORST_TIMER_MIN))	{
		state->conio->puts("ERROR el tiempo de autoreset debe ser menor a 240 minutos\n\r");
		return -1;
	}

	buffer[0] = 0x5A;
	buffer[1] = value;
	error = flash0_write(1, buffer, DF_AUTORST_OFFSET, 2);
	WDT_Feed();

	autorst_timer = value;
	
	return 1;
}

int con_setNmax(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;


	if( state->numparams < 3 )	{
		state->conio->puts("set NMAX d, con d >= 3 y d <= 15\n\r");
		return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	if((value > 15) || (value < 3))	{
		state->conio->puts("ERROR el valor de nmax debe estar entre 3 y 15\n\r");
		return -1;
	}

	buffer[0] = value;

	error = flash0_write(1, buffer, DF_NMAX_OFFSET, 1);
	WDT_Feed();

	nmax = value;
	
	return 1;
}

int con_setNmaxASAL(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;


	if( state->numparams < 3 )	{
		state->conio->puts("set NMAXASAL d, con d >= 3 y d <= 200\n\r");
		return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	if((value > 200) || (value < 3))	{
		state->conio->puts("ERROR el valor de nmax debe estar entre 3 y 200\n\r");
		return -1;
	}

	buffer[0] = value;

	error = flash0_write(1, buffer, DF_NMAXASAL_OFFSET, 1);
	WDT_Feed();

	nmax_asal = value;
	
	return 1;
}

int con_setNmaxTESO(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;


	if( state->numparams < 3 )	{
		state->conio->puts("set NMAXTESO d, con d >= 3 y d <= 200\n\r");
		return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	if((value > 200) || (value < 3))	{
		state->conio->puts("ERROR el valor de nmax debe estar entre 3 y 200\n\r");
		return -1;
	}

	buffer[0] = value;

	error = flash0_write(1, buffer, DF_NMAXTESO_OFFSET, 1);
	WDT_Feed();

	nmax_teso = value;
	
	return 1;
}

int con_setNmaxINCE(ConsoleState* state)
{
	uint8_t buffer[4];
	int error, value;


	if( state->numparams < 3 )	{
		state->conio->puts("set NMAXINCE d, con d >= 3 y d <= 200\n\r");
		return 1;
	}

	value = atoi(con_getparam(state->command, 2));

	if((value > 200) || (value < 3))	{
		state->conio->puts("ERROR el valor de nmax debe estar entre 3 y 200\n\r");
		return -1;
	}

	buffer[0] = value;

	error = flash0_write(1, buffer, DF_NMAXINCE_OFFSET, 1);
	WDT_Feed();

	nmax_ince = value;
	
	return 1;
}

int con_sendvarsdly(ConsoleState* state)
{
    uint8_t buffer[4];
    int ptmdly, bordly, autoreset;

    flash0_read(buffer, DF_PTMDLY_OFFSET, 2);
    WDT_Feed();
    ptmdly = buffer[1];
    if(buffer[0] == 0x5A)
        logCidEvent(account, 1, 879, 0, ptmdly);
    else
        logCidEvent(account, 1, 879, 0, 0);

    flash0_read(buffer, DF_BORDLY_OFFSET, 2);
    WDT_Feed();
    bordly = buffer[1];
    if(buffer[0] == 0x5A)
        logCidEvent(account, 1, 880, 0, bordly);
    else
        logCidEvent(account, 1, 880, 0, 0);

    flash0_read(buffer, DF_AUTORST_OFFSET, 2);
    WDT_Feed();
    autoreset = buffer[1];
    if(buffer[0] == 0x5A)
        logCidEvent(account, 1, 881, 0, autoreset);
    else
        logCidEvent(account, 1, 881, 0, 0);

    flash0_read(buffer, DF_NMAXASAL_OFFSET, 1);
    logCidEvent(account, 1, 882, 0, buffer[0]);
    WDT_Feed();
    flash0_read(buffer, DF_NMAXTESO_OFFSET, 1);
    logCidEvent(account, 1, 883, 0, buffer[0]);
    WDT_Feed();
    flash0_read(buffer, DF_NMAXINCE_OFFSET, 1);
    logCidEvent(account, 1, 884, 0, buffer[0]);
    WDT_Feed();

    return 1;
}

int con_remptmarm(ConsoleState* state)
{
    uint8_t buffer[4];
    int i, value;


    if( state->numparams < 2 )	{
        state->conio->puts("REMPTMARM particion, con particion >= 0 y particion <= 99\n\r");
        return 1;
    }

    value = atoi(con_getparam(state->command, 1));

    if((value > 99) || (value < 0))	{
        state->conio->puts("ERROR el valor de particion debe estar entre 0 y 99\n\r");
        return -1;
    }


    for( i = 0; i < MAXQTYPTM; i++ )    {
        if(ptm_dcb[i].rtuaddr == value) {
            ptm_dcb[i].flags |= PTMCMD_ARM;
            if(!(PTM_dev_status[i] & 0x01))
                GenerateCIDEventPTm(i, 'R', 407, 0);
            break;
        }
    }
    if(i >= MAXQTYPTM)  {
        state->conio->puts("ERROR\n\r");
        return -1;
    }

    return 1;
}

int con_delaydual(ConsoleState* state)
{
    uint16_t value;
    uint8_t retval, temp[8];
    int error;

    char buffer[32];

    buffer[0] = 0;
    value = 0;

    if( state->numparams < 2 )	{
        state->conio->puts("delaydual mseg (entre 0 y 500)\n\r");
        flash0_read(temp, DF_DELAYDUAL_OFFSET, 4);
        if((temp[2] == 0x5A) && (temp[3] == 0xA5)) {
            value = temp[0]*0x100 + temp[1];
            retval = 1;
        }
        else
            retval = 0;

        Str_Cat(buffer, "delaydual = ");
        switch(retval)	{
            case 0:
                Str_Cat(buffer,itoa(value));
                Str_Cat(buffer, "  *NOT SET\n\r");
                break;
            case 1:
                Str_Cat(buffer,itoa(value));
                Str_Cat(buffer, "  *SET\n\r");
                break;
            default:
                Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                break;
        }
        state->conio->puts(buffer);
        return 1;
    }

    value = atoi(con_getparam(state->command, 1));

    buffer[0] = (value >> 8) & 0x00FF;
    buffer[1] = value & 0x00FF;
    buffer[2] = 0x5A;
    buffer[3] = 0xA5;
    error = flash0_write(1, (uint8_t *)buffer, DF_DELAYDUAL_OFFSET, 4);

    dualA_delay = value;

    return 1;
}

int con_DeltaT(ConsoleState* state)
{
    uint16_t value;
    uint8_t retval, temp[8];
    int error;

    char buffer[32];

    buffer[0] = 0;
    value = 0;

    if( state->numparams < 2 )	{
        state->conio->puts("DeltaT mseg (entre 0 y 500)\n\r");
        flash0_read(temp, DF_DELTAT_OFFSET, 4);
        if((temp[2] == 0x5A) && (temp[3] == 0xA5)) {
            value = temp[0]*0x100 + temp[1];
            retval = 1;
        }
        else
            retval = 0;

        Str_Cat(buffer, "DeltaT = ");
        switch(retval)	{
            case 0:
                Str_Cat(buffer,itoa(value));
                Str_Cat(buffer, "  *NOT SET\n\r");
                break;
            case 1:
                Str_Cat(buffer,itoa(value));
                Str_Cat(buffer, "  *SET\n\r");
                break;
            default:
                Str_Cat(buffer, "ACTIVATION ERROR\n\r");
                break;
        }
        state->conio->puts(buffer);
        return 1;
    }

    value = atoi(con_getparam(state->command, 1));

    buffer[0] = (value >> 8) & 0x00FF;
    buffer[1] = value & 0x00FF;
    buffer[2] = 0x5A;
    buffer[3] = 0xA5;
    error = flash0_write(1, (uint8_t *)buffer, DF_DELTAT_OFFSET, 4);

    DeltaT = value;

    return 1;
}

int con_gencid(ConsoleState* state)
{
    uint16_t devnum, evqual, event, particion, zone;

    devnum = atoi( con_getparam(state->command, 1) );
    evqual = atoi( con_getparam(state->command, 2) );
    event = atoi( con_getparam(state->command, 3) );
    particion = atoi( con_getparam(state->command, 4) );
    zone = atoi( con_getparam(state->command, 5));

    logCidEvent(account, evqual, event, particion, zone);

    return 1;
}

int con_closerst(ConsoleState* state)
{
    NET_ERR err;
    int error;
    uint8_t buffer[8];

    NetSock_Close(Monitoreo[0].monsock, &err);
    NetSock_Close(Monitoreo[1].monsock, &err);
    LLAVE_TX_OFF();
    POWER_TX_OFF();
    buffer[0] = 0;
    error = flash0_write(1, buffer, DF_HBRSTRTRY_OFFSET, 1);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    while(1);	//me reseteo por watchdog
    return 1;
}

int con_closesoc(ConsoleState* state)
{
    NET_ERR err;

    //NetSock_Close(Monitoreo[0].monsock, &err);
    //NetSock_Close(Monitoreo[1].monsock, &err);
//    state->conio->puts("Wait ... \n\r");
//    OSTimeDlyHMSM(0, 0, 30, 0, OS_OPT_TIME_HMSM_STRICT, &err);

    ethlink_state = ETHLNK_CONNECTED;
    NetNIC_Init(&err);
    OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_HMSM_STRICT, &err);

//    InitMonitoreoStruct();
//    Monitoreo[0].wdogr3kTimer = SEC_TIMER + (5*60);
//    Monitoreo[0].flags &= ~ACKWDG_FLAG;
//    Monitoreo[1].wdogr3kTimer = SEC_TIMER + (5*60);
//    Monitoreo[1].flags &= ~ACKWDG_FLAG;
    return 1;

}